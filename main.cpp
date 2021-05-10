#include <sstream>
#include <iostream>
#include <map>
#include <string_view>

#include <string.h>
#include <ctime>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iomanip>

#define check_err(func) if (errno != 0) \
                            perror(#func);

class http_request {
    std::string_view http_method;
    std::string_view url;
    std::string_view protocol;
    std::multimap<std::string_view, std::string_view> headers;

    std::string_view body;
};

class http_response {
public:

    class builder {
        unsigned int status_code = -1;
        std::multimap<std::string_view, std::string_view> headers;
        std::string_view body_;

        friend class http_response;

    public:
        builder& status(unsigned int sc) {
            status_code = sc;
            return *this;
        }

        builder& header(std::string_view header_key, std::string_view header_value) {
            headers.insert({header_key, header_value});
            return *this;
        }

        builder& body(const char* data) {
            body_ = data;
            return *this;
        }

        builder& body(std::string_view data) {
            body_ = data;
            return *this;
        }

        builder& body(std::string&& data) {
            body_ = std::move(data);
            return *this;
        }

        builder& body(const std::string& data) {
            body_ = data;
            return *this;
        }

        http_response build() const {
            if (status_code == -1) {
                throw std::runtime_error("Incomplete HTTP response: Status Code missing");
            }
            return http_response(*this);
        }
    };

private:

    std::string_view protocol;
    unsigned short status_code;
    std::string_view status_code_text;
    std::multimap<std::string_view, std::string_view> headers;

    std::string_view body;

    explicit http_response(const http_response::builder& builder) : status_code(builder.status_code),
                                                                    body(builder.body_) {
        for (auto pair : builder.headers) {
            headers.insert(pair);
        }
    }

public:

    static http_response::builder new_bulder() {
        return http_response::builder();
    }

    char* to_c_str() const {
        const char* data = body.data();
        size_t data_len = strlen(data);
        char* result = new char[data_len];
        std::copy(data, data + data_len, result);
        return result;
    }
};

void handle_connection(int connfd, struct sockaddr_in connaddr) {
    //these will be used for creating the gRPC stubs
    char *connection_address = inet_ntoa(connaddr.sin_addr);
    unsigned short connection_port = connaddr.sin_port;

    std::stringstream packets;
    ssize_t bytes_received = -1, total_bytes = 0;
    char packet[256]; //TODO make the packet size configurable
    memset(packet, '0', sizeof(packet));

    std::clog << "Receiving data from socket..." << std::endl;
    while ((bytes_received = recv(connfd, (void *) packet, 256, 0)) != -1) {
        if (bytes_received == 0) { //EOF
            break;
        }
        //if packet includes start of (or entire) body,
        // we first need to examine the start of the request (start-line + headers),
        // determine what to do,
        // then decide whether we need to read more (i.e. rest of body)
        // if not, we need to free the memory that the body takes
        packets << packet;
        total_bytes += bytes_received;
        if (bytes_received < 256) {
            break;
        }
    }
    std::clog << "Data received" << std::endl;
    check_err(recv);

//      http_request request = parse_http_request(packets);
//      http_response response = handle_request(request);

    std::time_t time_now = std::time(nullptr);

    const char *response_str = "Hello from server!";// response.to_c_str();
    size_t response_size = strlen(response_str) + 1;
    std::stringstream res;
    res << "HTTP/1.1 200 OK" << std::endl
        << "Date: " << std::put_time(std::localtime(&time_now), "%y-%m-%d %OH:%OM:%OS") << std::endl
        << "Content-Type: text/plain" << std::endl
        << "Content-Length: " << response_size << std::endl
        << "Server: cpp-server/0.1.0" << std::endl
        //if this header is set, we don't need to close the connection ourselves
        << "Connection: close" << std::endl
        //if, instead, this is set, we decide when to close the connection
//        << "Connection: keep-alive" << std::endl
        << std::endl
        << response_str << std::endl;

    auto str = res.str();
    size_t res_size = str.length();
    char* _res = new char[res_size];
    std::copy(str.begin(), str.end(), _res);

    std::clog << "Sending data to socket..." << std::endl;
    ssize_t bytes_sent = -1;
    while ((bytes_sent = send(connfd, (void *) _res, res_size, 0)) != -1) {
        if (bytes_sent >= res_size) {
            break;
        }
        response_str += bytes_sent;
        response_size -= bytes_sent;
    }
    delete [] _res;
    std::clog << "Data sent" << std::endl;
//      delete [] response_str;

//    check_err(send); //maybe this is unneeded

//    std::clog << "Closing connection..." << std::endl;
//    close(connfd);
//    std::clog << "Connection closed" << std::endl;
//    check_err(close);
}

int main() {
    int sockfd = 0;
    struct sockaddr_in serveraddr;

    std::clog << "Creating socket..." << std::endl;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    std::clog << "Socket created" << std::endl;

    memset(&serveraddr, '0', sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(80);

    std::clog << "Binding socket to local address..." << std::endl;
    if (bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0) {
        perror("bind");
        return -1;
    }
    std::clog << "Socket bound" << std::endl;

    if (listen(sockfd, 10) != 0) {
        perror("listen");
        return -1;
    }
    std::clog << "Listening for connections..." << std::endl;

    while (true) {
        struct sockaddr_in connaddr;
        socklen_t connlen;
        int connfd = accept(sockfd, (struct sockaddr*) &connaddr, &connlen);

        if (connfd >= 0)
            std::clog << "Connection established" << std::endl;

        switch (connfd) {
            case EINTR:
            case ECONNABORTED:
                continue;
            case ENOBUFS:
            case ENOMEM:
                close(sockfd);
                check_err(close);
                return -1;
            case EPERM:
                perror("accept");
                return -1;
            default:
                break;
        }

        if (connfd >= 0)
            //preferably spawn a new thread to handle the connection
            handle_connection(connfd, connaddr);
    }

    std::clog << "Closing socket..." << std::endl;
    if (close(sockfd) != 0) {
        perror("close");
        return -1;
    }
    std::clog << "Socket closed" << std::endl;
    return 0;
}
