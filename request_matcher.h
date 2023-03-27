#ifndef TICTACTOE_SERVER_REQUEST_MATCHER_H
#define TICTACTOE_SERVER_REQUEST_MATCHER_H

#include <regex>
#include "http_methods.h"

struct RequestMatcher {
    HttpMethod method;
    std::string path;

    RequestMatcher(HttpMethod method, const std::string& path) : method(method), path(path) {
        if (path.find('?') != std::string::npos
            || path.find('#') != std::string::npos) {
            throw std::runtime_error("Path matcher can't contain query or fragment params");
        }
        if (path.size() > 1 && path.ends_with('/')) {
            this->path = path.substr(0, path.size() - 1);
        }
    }

    //TODO simplify this to just use std::regex_replace with \{.+} -> \*
    std::string normalize_path() const {
//        auto to_string = []<typename T> (const T& view) -> std::string {
//            auto common = view | std::views::common;
//            return std::string(common.begin(), common.end());
//        };

        std::string result(path);
//        if (path.front() != '/') {
//            result.reserve(1);
//            result.insert(result.begin(), '/');
//        }
//        if (result.find('{') == std::string::npos) {
//            return result;
//        }
//        std::stringstream ss;
//        for (const auto& view : result | std::views::split('/')) {
//            std::string path_part = to_string(view);
//            if (path_part.find('{') != std::string::npos) {
//                ss << "*";
//            } else {
//                ss << path_part;
//            }
//            ss << '/';
//        }
//        result = ss.str();
//        result.pop_back();
        return result;
    }
};

namespace std {
    template<>
    struct hash<RequestMatcher> {
        size_t operator()(const RequestMatcher& matcher) const {
            return ((hash<HttpMethod>()(matcher.method) ^ (hash<string>()(matcher.normalize_path()) << 1)) >> 1);
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
