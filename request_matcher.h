#ifndef TICTACTOE_SERVER_REQUEST_MATCHER_H
#define TICTACTOE_SERVER_REQUEST_MATCHER_H

#include <string_view>
#include <regex>
#include "http_methods.h"

struct RequestMatcher {
    const HttpMethod method;
    const std::string_view path;

    RequestMatcher(HttpMethod method, std::string_view path) : method(method), path(path) {
        if (path.find('?') != std::string_view::npos
            || path.find('#') != std::string_view::npos) {
            throw std::runtime_error("Path matcher can't contain query or fragment params");
        }
    }

    std::string normalize_path() const {
        std::string result(path);
        if (*path.begin() != '/') {
            result = "/" + result;
        }
        if (result.find('{') == std::string::npos) {
            return result;
        }
        return std::regex_replace(result, std::regex(R"({\w+})"), "*");
    }
};

namespace std {
    template<>
    struct hash<RequestMatcher> {
        size_t operator()(const RequestMatcher& matcher) const {
            return ((hash<HttpMethod>()(matcher.method) ^ (hash<string_view>()(matcher.path) << 1)) >> 1);
        }
    };

    template<>
    struct equal_to<RequestMatcher> {
        bool operator()(const RequestMatcher& lhs, const RequestMatcher& rhs) const {
            return lhs.method == rhs.method && lhs.normalize_path() == rhs.normalize_path();
        }
    };
}

#endif //TICTACTOE_SERVER_REQUEST_MATCHER_H
