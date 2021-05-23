#ifndef TICTACTOE_SERVER_HTTPSERVER_H
#define TICTACTOE_SERVER_HTTPSERVER_H

#include <unordered_map>
#include <functional>
#include <memory>
#include "request_matcher.h"
#include "http_request.h"
#include "http_response.h"
#include "http_request_context.h"
#include "thread_pool.h"
#include "connection.h"
#include "logger.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

class socket_exception;

class HttpServer {
public:
    struct Config {
        long packet_size;
        unsigned socket_connection_queue_size;
        unsigned long socket_read_timeout_millis;
        ThreadPool::Config th_pool_config;

        Config() : packet_size(52), socket_connection_queue_size(10), socket_read_timeout_millis(30 * 1000) {}
    };

private:
    int socket_fd;
    long packet_size;
    unsigned socket_connection_queue_size;
    unsigned long socket_read_timeout_millis;

    using handlers_map = std::unordered_map<RequestMatcher, std::function<HttpResponse(const HttpRequestContext &)>>;
    handlers_map handlers;

    ThreadPool connection_pool;

    class ServiceUnavailablePolicy : public ThreadPool::RejectedJobPolicy {
        void handle_rejected_job(const std::function<void()> &job) override;
    };

public:
    explicit HttpServer(HttpServer::Config server_config = {});

    ~HttpServer();

    void start();

    //a possible extension to path matching functionality is to support wildcards
    // e.g.: /path/** will match any subpath of "path/"
    // for this to be usable, the entire request URL needs to be added to HttpRequestContext
    template<HttpMethod Method>
    void register_handler(const char* path, std::function<HttpResponse(const HttpRequestContext &)> handler) {
        if (strlen(path) == 0) {
            throw std::runtime_error("path can't be empty");
        }
        handlers.insert({RequestMatcher(Method, path), std::move(handler)});
    }

private:
    void create_tcp_socket();

    void bind_socket();

    std::shared_ptr<detail::Connection> accept_connection();
};

#endif //TICTACTOE_SERVER_HTTPSERVER_H
