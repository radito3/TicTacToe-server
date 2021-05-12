#ifndef TICTACTOE_SERVER_HTTPSERVER_H
#define TICTACTOE_SERVER_HTTPSERVER_H

#include <unordered_map>
#include <functional>
#include "request_matcher.h"
#include "http_request.h"
#include "http_response.h"
#include "http_request_context.h"
#include "thread_pool.h"

class socket_exception;

template<typename... Args>
void log(Args&&... args);

class HttpServer {
public:
    struct Config {
        unsigned long packet_size;
        unsigned socket_connection_queue_size;
        ThreadPool::Config th_pool_config;

        Config() : packet_size(256), socket_connection_queue_size(10) {}
    };

private:
    int socket_fd;
    unsigned long packet_size;
    unsigned socket_connection_queue_size;

    using handlers_map = std::unordered_map<RequestMatcher, std::function<HttpResponse(const HttpRequestContext &)>>;
    handlers_map handlers;

    ThreadPool connection_pool;

public:
    explicit HttpServer(HttpServer::Config server_config = {});

    ~HttpServer();

    void start();

    void register_handler(const RequestMatcher&, std::function<HttpResponse(const HttpRequestContext &)>);

private:
    class HandleConnectionJob {
        int connection_fd;
        handlers_map::value_type pair;
        HttpRequestContext request_context;

    public:
        HandleConnectionJob(int, const handlers_map::value_type&, const HttpRequest&);

        void operator()();
    };

    void create_tcp_socket();

    void bind_socket();

    std::pair<int, struct sockaddr_in> accept_connection();

    void read_data_from_socket(int connection_fd, std::iostream &packets);

    static HttpRequest parse_http_request(std::istream&);

    static HttpMethod parse_http_method(const std::string_view&);

    auto find_request_handler(const HttpRequest&) -> handlers_map::iterator;

    static void add_mandatory_headers_to_response(HttpResponse &);

    static void send_response_to_socket(int, HttpResponse);
};

#endif //TICTACTOE_SERVER_HTTPSERVER_H
