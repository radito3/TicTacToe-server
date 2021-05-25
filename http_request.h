#ifndef TICTACTOE_SERVER_HTTP_REQUEST_H
#define TICTACTOE_SERVER_HTTP_REQUEST_H

#include <map>
#include <utility>

struct HttpRequest {
    HttpMethod http_method;
    std::string url;
    std::string protocol;
    std::multimap<std::string, std::string> headers;
    std::string body;

    HttpRequest() = default;

    HttpRequest(HttpMethod httpMethod, const std::string &url, const std::string &aProtocol,
                std::multimap<std::string, std::string> headers, const std::string &body)
            : http_method(httpMethod), url(url), protocol(aProtocol), headers(std::move(headers)), body(body) {}
};

#endif //TICTACTOE_SERVER_HTTP_REQUEST_H
