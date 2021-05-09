#include <sstream>
#include <map>
#include <string_view>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
    std::string_view protocol;
    unsigned short status_code;
    std::string_view status_code_text;
    std::multimap<std::string_view, std::string_view> headers;

    std::string_view body;

public:

    char* to_c_str() const {
        return "";
    }
};

http_request parse_http_request(std::stringstream& packets) {

    return http_request();
}

http_response handle_request(const http_request& request) {

    return http_response();
}

//TODO configure signal handles for SIGTERM, SIGKILL (and maybe others) to close the open connections & call appropriate dtors

int main() {
    int sockfd = 0;
    struct sockaddr_in serveraddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&serveraddr, '0', sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(80);

    if (bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0) {
        perror("bind");
        return -1;
    }

    if (listen(sockfd, 10) != 0) {
        perror("listen");
        return -1;
    }

    while (true) {
        struct sockaddr_in connaddr;
        int connfd = accept(sockfd, (struct sockaddr*) &connaddr, NULL);
        
        //is this really necessary?
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

        //these will be used for creating the gRPC stubs
        char *connection_address = inet_ntoa(connaddr.sin_addr);
        unsigned short connection_port = connaddr.sin_port;

        std::stringstream packets;
        ssize_t bytes_received = -1;
        char packet[1024]; //TODO make the packet size configurable
        memset(packet, '0', sizeof(packet));

        //recv
        //parse
        //compute
        //send
        while ((bytes_received = recv(connfd, (void *) packet, 1024, 0)) != -1) {
            if (bytes_received == 0) { //EOF
                break;
            }
            packets << packet;
        }
        check_err(recv);
        
        http_request request = parse_http_request(packets);
        http_response response = handle_request(request);
        char* response_str = response.to_c_str();
        size_t response_size = strlen(response_str);

        ssize_t bytes_sent = -1;
        while ((bytes_sent = send(connfd, (void *) response_str, response_size, 0)) > 0) {
            if (bytes_sent < response_size) {
                response_str += bytes_sent;
                response_size -= bytes_sent;
            }
        }
        check_err(send);

        close(connfd);
        check_err(close);
    }

    if (close(sockfd) != 0) {
        perror("close");
        return -1;
    }
    return 0;
}
