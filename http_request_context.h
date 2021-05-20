#ifndef TICTACTOE_SERVER_HTTP_REQUEST_CONTEXT_H
#define TICTACTOE_SERVER_HTTP_REQUEST_CONTEXT_H

#include <map>
#include <unordered_map>
#include <utility>

class HttpRequestContext {
public:
    class Builder {
        std::string client_address_;
        std::multimap<std::string, std::string> headers_;
        std::unordered_map<std::string, std::string> path_params_;
        std::multimap<std::string, std::string> query_params_;
        std::string fragment_;
        std::string body_;

        friend class HttpRequestContext;

    public:
        Builder& client_address(std::string address) {
            client_address_ = address;
            return *this;
        }

        Builder& headers(std::multimap<std::string, std::string> headers) {
            headers_ = std::move(headers);
            return *this;
        }

        Builder& path_params(std::unordered_map<std::string, std::string> path_params) {
            path_params_ = std::move(path_params);
            return *this;
        }

        Builder& query_params(std::multimap<std::string, std::string> query_params) {
            query_params_ = std::move(query_params);
            return *this;
        }

        Builder& fragment(std::string fragment) {
            fragment_ = fragment;
            return *this;
        }

        Builder& body(std::string body) {
            body_ = body;
            return *this;
        }

        HttpRequestContext build() {
            return HttpRequestContext(*this);
        }
    };

    const std::string client_address;
    const std::multimap<std::string, std::string> headers;
    const std::unordered_map<std::string, std::string> path_params;
    const std::multimap<std::string, std::string> query_params;
    const std::string fragment;
    const std::string body;

    HttpRequestContext() = delete;

    static Builder new_builder() {
        return Builder();
    }

private:
    explicit HttpRequestContext(const Builder& builder) : client_address(builder.client_address_),
                                                            headers(builder.headers_),
                                                            path_params(builder.path_params_),
                                                            query_params(builder.query_params_),
                                                            fragment(builder.fragment_),
                                                            body(builder.body_) {}
};

#endif //TICTACTOE_SERVER_HTTP_REQUEST_CONTEXT_H
