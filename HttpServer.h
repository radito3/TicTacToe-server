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
        unsigned socket_read_timeout_millis;
        ThreadPool::Config th_pool_config;

        Config() : packet_size(52), socket_connection_queue_size(10), socket_read_timeout_millis(30 * 1000) {}
    };

private:
    int socket_fd;
    unsigned long packet_size;
    unsigned socket_connection_queue_size;
    unsigned socket_read_timeout_millis;

    using handlers_map = std::unordered_map<RequestMatcher, std::function<HttpResponse(const HttpRequestContext &)>>;
    handlers_map handlers;

    ThreadPool connection_pool;

public:
    explicit HttpServer(HttpServer::Config server_config = {});

    ~HttpServer();

    void start();

    template<HttpMethod Method>
    void register_handler(const char*, std::function<HttpResponse(const HttpRequestContext &)>);

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

    bool check_for_client_timeout(int);

    bool read_data_from_socket(int, std::iostream &);

    static HttpRequest parse_http_request(std::istream&);

    static HttpMethod parse_http_method(const std::string_view&);

    auto find_request_handler(const HttpRequest&) -> handlers_map::iterator;

    static HttpResponse add_mandatory_headers_to_response(const HttpResponse &);

    static void send_response_to_socket(int, HttpResponse);
};

#endif //TICTACTOE_SERVER_HTTPSERVER_H
