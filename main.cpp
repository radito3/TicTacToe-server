#include <sstream>
#include <map>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    const char* to_c_str() const {
        return "";
    }
};

http_request parse_http_request(std::stringstream& packets) {

    return http_request();
}

http_response handle_request(const http_request& request) {

    return http_response();
}

int main() {
    int sockfd = 0, connfd = 0;
    struct sockaddr_in serveraddr, connaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&serveraddr, '0', sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0) {
        perror("bind");
        return -1;
    }

    if (listen(sockfd, 10) != 0) {
        perror("listen");
        return -1;
    }

    while (true) {
        connfd = accept(sockfd, (struct sockaddr*) &connaddr, NULL);

        //these will be used for creating the gRPC stubs
        char *connection_address = inet_ntoa(connaddr.sin_addr);
        unsigned short connection_port = connaddr.sin_port;

        std::stringstream packets;
        ssize_t bytes_received = -1;
        char packet[1024];
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
        http_request request = parse_http_request(packets);
        http_response response = handle_request(request);
        const char* response_str = response.to_c_str();

        send(connfd, (const void *) response_str, strlen(response_str), 0);

        close(connfd);
    }

    if (close(sockfd) != 0) {
        perror("close");
        return -1;
    }
    return 0;
}
