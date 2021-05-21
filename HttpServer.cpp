#include "HttpServer.h"
#include <unistd.h>
#include <cerrno>
#include "handle_connection_job.h"

class socket_exception : public std::runtime_error {
public:
    explicit socket_exception(const std::string &arg) : runtime_error(arg) {}

    explicit socket_exception(const char *string) : runtime_error(string) {}
};

HttpServer::HttpServer(HttpServer::Config server_config)
    : packet_size(server_config.packet_size),
      socket_connection_queue_size(server_config.socket_connection_queue_size),
      socket_read_timeout_millis(server_config.socket_read_timeout_millis),
      connection_pool(server_config.th_pool_config)
{
    create_tcp_socket();
    bind_socket();
}

HttpServer::~HttpServer() {
    log("Closing socket...");
    close(socket_fd);
    log("Socket closed");
}

void HttpServer::start() {
    if (listen(socket_fd, socket_connection_queue_size) != 0) {
        throw socket_exception("Error listening on socket: " + std::string(strerror(errno)));
    }
    log("Listening for connections...");

    while (true) {
        auto connection = accept_connection();
        if (!*connection) {
            continue;
        }
        connection_pool.submit_job(detail::HandleConnectionJob(connection, &handlers));
    }
}

void HttpServer::create_tcp_socket() {
    log("Creating socket...");
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw socket_exception("Error creating socket: " + std::string(strerror(errno)));
    }
    log("Socket with fd ", socket_fd, " created");
}

void HttpServer::bind_socket() {
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(80);

    log("Binding socket to local address...");
    if (bind(socket_fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0) {
        throw socket_exception("Error binding socket: " + std::string(strerror(errno)));
    }
    log("Socket bound");
}

std::shared_ptr<detail::Connection> HttpServer::accept_connection() {
    struct sockaddr_in connaddr;
    socklen_t connlen;
    int connfd = accept(socket_fd, (struct sockaddr*) &connaddr, &connlen);

    if (connfd >= 0) {
        log("Connection established");
    } else {
        switch (connfd) {
            case ENOBUFS:
            case ENOMEM:
            case EPERM:
                throw socket_exception("Error accepting connection: " + std::string(strerror(errno)));
            default:
                return std::make_shared<detail::Connection>(-1, connaddr, -1, 0);
        }
    }
    return std::make_shared<detail::Connection>(connfd, connaddr, packet_size, socket_read_timeout_millis);
}
