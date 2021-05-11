#ifndef TICTACTOE_SERVER_HTTP_REQUEST_H
#define TICTACTOE_SERVER_HTTP_REQUEST_H

#include <string_view>
#include <map>
#include <utility>

struct HttpRequest {
    const HttpMethod http_method;
    const std::string_view url;
    const std::string_view protocol;
    const std::multimap<std::string_view, std::string_view> headers;

    const std::string_view body;

    HttpRequest(const HttpMethod httpMethod, const std::string_view &url, const std::string_view &aProtocol,
                std::multimap<std::string_view, std::string_view> headers, const std::string_view &body)
            : http_method(httpMethod), url(url), protocol(aProtocol), headers(std::move(headers)), body(body) {}
};

#endif //TICTACTOE_SERVER_HTTP_REQUEST_H
