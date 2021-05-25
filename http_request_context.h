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
            client_address_ = std::move(address);
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
            fragment_ = std::move(fragment);
            return *this;
        }

        Builder& body(std::string body) {
            body_ = std::move(body);
            return *this;
        }

        HttpRequestContext build() {
            return HttpRequestContext(*this);
        }
    };

private:
    const std::string client_address;
    const std::multimap<std::string, std::string> headers;
    const std::unordered_map<std::string, std::string> path_params;
    const std::multimap<std::string, std::string> query_params;
    const std::string fragment;
    const std::string body;

public:
    HttpRequestContext() = delete;

    static Builder new_builder() {
        return Builder();
    }

    std::string get_client_address() const {
        return client_address;
    }

    std::string get_header(const std::string& key) const {
        return headers.find(key)->second;
    }

    std::multimap<std::string, std::string> get_headers() const {
        return headers;
    }

    std::string get_path_param(const std::string& path_elem) const {
        return path_params.find(path_elem)->second;
    }

    std::string get_query_param(const std::string& param) const {
        return query_params.find(param)->second;
    }

    std::multimap<std::string, std::string> get_query_params() const {
        return query_params;
    }

    std::string get_fragment() const {
        return fragment;
    }

    std::string get_body() const {
        return body;
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
