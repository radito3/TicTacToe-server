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

    static const int URL_LENGTH_LIMIT = 8000;

    class Connection {
        int connection_fd;
        struct sockaddr_in connection_address;
        long packet_size;
        unsigned long socket_read_timeout_millis;

    public:
        Connection(int connFd, struct sockaddr_in connAddress, long packetSize,
                unsigned long socket_read_timeout_millis) : connection_fd(connFd), connection_address(connAddress),
                packet_size(packetSize), socket_read_timeout_millis(socket_read_timeout_millis) {
            struct timeval timeout{};
            timeout.tv_sec = (long) socket_read_timeout_millis / 1000;
            timeout.tv_usec = (long) (socket_read_timeout_millis % 1000) * 1000;
            setsockopt(connection_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        }

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
                 socket_read_timeout_millis(std::exchange(other.socket_read_timeout_millis, 0)) {}

        Connection& operator=(Connection&& other) noexcept {
            if (this == &other) {
                return *this;
            }
            connection_fd = std::exchange(other.connection_fd, -1);
            connection_address = other.connection_address;
            packet_size = std::exchange(other.packet_size, -1);
            socket_read_timeout_millis = std::exchange(other.socket_read_timeout_millis, 0);
            return *this;
        }

        bool operator!() const {
            return connection_fd < 0;
        }

        explicit operator bool() const {
            return connection_fd >= 0;
        }

        sockaddr_in get_connection_address() const {
            return connection_address;
        }

        void send_response(const HttpResponse &response) {
            auto res = add_mandatory_headers_to_response(response);
            std::stringstream ss;
            //if large responses are supported, change the type of the body to a stream
            // that way, the application memory won't overflow
            // and send data to the socket by buffering a packet-size worth of data and calling
            // ::send(conn_fd, buffer, packet_size, MSG_MORE)
            ss << res;
            std::string res_str = ss.str();
            const char* data = res_str.data();
            size_t data_len = strlen(data) + 1;

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

        std::tuple<HttpMethod, std::string> read_method_and_path() {
            ssize_t bytes_received;
            char packet[packet_size];
            memset(packet, '\0', sizeof(packet));

            log("Reading data from connection with fd ", connection_fd, "...");
            bytes_received = recv(connection_fd, (void *) packet, packet_size, MSG_PEEK);
            if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                //TODO make a custom timeout error class
                // so that we can return a 408 Request Timeout
                throw std::runtime_error("Connection timed out");
            }
            if (bytes_received == 0) {
                throw std::runtime_error("Connection didn't send any data");
            }

            size_t line_length = strcspn(packet, "\r\n");
            if (line_length == packet_size) {
                //read more data until configured URI length limit
            }

            if (line_length >= URL_LENGTH_LIMIT) {
                //TODO make a custom 414 error class
                throw std::runtime_error("Request Too Long");
            }

            //TODO truncate packet or push to a stringstream until only the method and the path are in memory
            // parse them
            char line[line_length]; //maybe use string?
            strncpy(line, packet, line_length);

            size_t method_length = strcspn(line, " ");
            std::string method_raw(line, method_length);
            HttpMethod method = parse_http_method(method_raw);

            size_t url_length = strcspn(line + method_length + 1, " ");
            std::string url(line + method_length + 1, url_length - method_length - 1);

            return {method, url};
        }

        HttpRequest read_request() {
            std::stringstream packets;
            ssize_t bytes_received;
            char packet[packet_size];
            memset(packet, '\0', sizeof(packet));

            log("Reading data from connection with fd ", connection_fd, "...");
            while ((bytes_received = recv(connection_fd, (void *) packet, packet_size, 0)) > 0) {
                packets << packet;
                if (bytes_received < packet_size) {
                    break;
                }
                memset(packet, '\0', sizeof(packet));
            }

            if (bytes_received == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    //TODO make a custom timeout error class
                    // so that we can return a 408 Request Timeout
                    throw std::runtime_error("Connection timed out");
                }
                //handle other recv errors
            }
            log("Data received");
            return parse_http_request(packets);
        }

    private:
        static HttpRequest parse_http_request(std::istream& packets) {
            std::string line;
            std::getline(packets, line);
            std::istringstream start_line(line);

            std::string http_method;
            start_line >> http_method;
            HttpMethod method = parse_http_method(http_method);

            //check for the length of the url
            // if it is too long (e.g. more than 8000 symbols (encoded)) return 414 URI Too Long
            //handle url encoding
            std::string url;
            start_line >> url;

            std::string protocol;
            start_line >> protocol;

            std::multimap<std::string, std::string> headers;

            //TODO simplify to splitting on : and assigning the keys and values
            while (std::getline(packets, line) && !std::regex_match(line, std::regex("\\s+"))) {
                std::regex header_regex(R"(^([-A-Za-z]+):[ \t]+(\S+)$)");
                std::smatch header_match;
                if (std::regex_search(line, header_match, header_regex)) {
                    headers.insert({header_match[1], header_match[2]});
                }
            }

            //TODO make this read lazy
            // only invoke this logic if the handler calls request.get_body()
            // and change to read from socket on-demand because currently the whole body is loaded in memory
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

        static HttpMethod parse_http_method(const std::string& method) {
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

        //these aren't really mandatory....
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
