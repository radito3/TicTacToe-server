#ifndef TICTACTOE_SERVER_CONNECTION_H
#define TICTACTOE_SERVER_CONNECTION_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <regex>
#include <sstream>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "logger.h"
#include "http_request.h"

namespace detail {

    class Connection {
        int connection_fd;
        struct sockaddr_in connection_address;
        long packet_size;
        unsigned long socket_read_timeout_millis;

    public:
        Connection(int connFd, struct sockaddr_in connAddress, long packetSize,
                unsigned long socket_read_timeout_millis) : connection_fd(connFd), connection_address(connAddress),
                packet_size(packetSize), socket_read_timeout_millis(socket_read_timeout_millis) {}

        ~Connection() {
            if (connection_fd >= 0) {
                log("Closing connection with fd ", connection_fd, "...");
                close(connection_fd);
                log("Connection closed");
            }
        }

        Connection(const Connection&) = delete;

        Connection& operator=(const Connection&) = delete;

        Connection(Connection&& other) noexcept : connection_fd(std::exchange(other.connection_fd, -1)),
                 connection_address(other.connection_address), packet_size(std::exchange(other.packet_size, -1)),
                 socket_read_timeout_millis(std::exchange(other.socket_read_timeout_millis, -1)) {}

        Connection& operator=(Connection&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            connection_fd = std::exchange(other.connection_fd, -1);
            connection_address = other.connection_address;
            packet_size = std::exchange(other.packet_size, -1);
            socket_read_timeout_millis = std::exchange(other.socket_read_timeout_millis, -1);
            return *this;
        }

        bool operator!() const {
            return connection_fd < 0;
        }

        explicit operator bool() const {
            return connection_fd >= 0;
        }

        struct sockaddr_in get_connection_address() const {
            return connection_address;
        }

        void send_(const HttpResponse &response) {
            auto res = add_mandatory_headers_to_response(response);
            std::stringstream ss;
            //TODO if large responses are supported, change the type of the body to a stream
            // that way, the application memory won't overflow
            // and send data to the socket by buffering a packet-size worth of data and calling
            // ::send(conn_fd, buffer, packet_size, MSG_MORE)
            ss << res;
            std::string res_str = ss.str();
            const char* data = res_str.data();
            ssize_t data_len = strlen(data);

            char* result = new char[data_len];
            strcpy(result, data);

            log("Sending data to connection with fd ", connection_fd, "...");
            if (send(connection_fd, (void *) result, data_len, 0) != data_len) {
                perror("send");
            } else {
                log("Data sent");
            }
            delete [] result;
        }

        HttpRequest receive() {
            bool success = try_receive();
            if (!success) {
                throw std::runtime_error("socket read timeout");
            }

            std::stringstream packets;
            ssize_t bytes_received = -1;
            char packet[packet_size];
            memset(packet, '\0', sizeof(packet));

            log("Receiving data from connection with fd ", connection_fd, "...");
            while ((bytes_received = recv(connection_fd, (void *) packet, packet_size, 0)) != -1) {
                if (bytes_received == 0) { //EOF
                    break;
                }
                //we first need to examine the start of the request (start-line + headers_),
                // determine what to do,
                // then decide whether we need to read more (i.e. rest of body_)
                // if not, we need to free the memory that the body_ takes
                packets << packet;
                if (bytes_received < packet_size) {
                    break;
                }
                memset(packet, '\0', sizeof(packet));
            }
            log("Data received");
            return parse_http_request(packets);
        }

    private:
        bool try_receive() const {
            std::condition_variable timer;
            std::mutex t_mutex;

            std::thread([&timer, this]() {
                char packet[packet_size];
                recv(connection_fd, (void *) packet, packet_size, MSG_PEEK);
                timer.notify_one();
            }).detach();

            std::unique_lock<std::mutex> lock(t_mutex);
            if (timer.wait_for(lock, std::chrono::milliseconds(socket_read_timeout_millis)) == std::cv_status::timeout) {
                return false;
            }
            return true;
        }

        static HttpRequest parse_http_request(std::istream& packets) {
            using r_iter = std::sregex_token_iterator;

            std::string http_method;
            packets >> http_method;
            HttpMethod method = parse_http_method(http_method);

            //TODO check for the length of the url
            // if it is too long (e.g. more than 8000 symbols (encoded)) return 414 URI Too Long
            //TODO handle url encoding
            std::string url;
            packets >> url;

            std::string protocol;
            packets >> protocol;

            std::string line;
            std::multimap<std::string, std::string> headers;
            std::getline(packets, line);
            while (!line.empty()) {
                std::regex colon(R"(:\s+)");
                std::vector<std::string> header(r_iter(line.begin(), line.end(), colon, -1), r_iter());
                headers.insert({header[0], header[1]});
                std::getline(packets, line);
            }

            std::string body;
            while (packets) {
                char byte;
                packets.read(&byte, 1);
                if (byte == '\0') {
                    break;
                }
                body += byte;
            }

            return HttpRequest(method, url, protocol, headers, body);
        }

        static HttpMethod parse_http_method(std::string_view method) {
            if (method == "GET") {
                return HttpMethod::GET;
            }
            if (method == "POST") {
                return HttpMethod::POST;
            }
            if (method == "PUT") {
                return HttpMethod::PUT;
            }
            if (method == "OPTIONS") {
                return HttpMethod::OPTIONS;
            }
            if (method == "PATCH") {
                return HttpMethod::PATCH;
            }
            if (method == "DELETE") {
                return HttpMethod::DELETE;
            }
            if (method == "HEAD") {
                return HttpMethod::HEAD;
            }
            if (method == "CONNECT") {
                return HttpMethod::CONNECT;
            }
            if (method == "TRACE") {
                return HttpMethod::TRACE;
            }
            throw std::runtime_error("Invalid HTTP method");
        }

        static HttpResponse add_mandatory_headers_to_response(const HttpResponse &response) {
            auto result = HttpResponse::copy_from(response);

            using std::chrono::system_clock;
            std::time_t tt = system_clock::to_time_t(system_clock::now());
            struct std::tm* ptm = std::localtime(&tt);
            std::stringstream time;
            time << std::put_time(ptm, "%a %b %d %T %Z %Y");

            result.header("Date", time.str());
            result.header("Server", "cpp-server/0.1.0 (Alpine Linux)");
            //when switching to reading from and sending to a connection in a coroutine,
            // consider redesigning to allow for persistent connections
            // in which case, this header will be omitted
            result.header("Connection", "close");
            if (!result.contains_header("Content-Type") && result.contains_header("Content-Length")) {
                result.header("Content-Type", "text/plain");
            }
            return result.build();
        }
    };
}

#endif //TICTACTOE_SERVER_CONNECTION_H
