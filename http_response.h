#ifndef TICTACTOE_SERVER_HTTP_RESPONSE_H
#define TICTACTOE_SERVER_HTTP_RESPONSE_H

#include <cstring>
#include <map>
#include <string_view>

class HttpResponse {
public:
    class Builder {
        unsigned short status_code = 0;
        std::multimap<std::string_view, std::string_view> headers;
        std::string_view body_;

        friend class HttpResponse;

    public:
        Builder() = default;

        explicit Builder(const HttpResponse& res) : status_code(res.status_code),
            headers(res.headers), body_(res.body) {}

        Builder& status(unsigned short sc) {
            status_code = sc;
            return *this;
        }

        Builder& header(std::string_view header_key, std::string_view header_value) {
            headers.insert({header_key, header_value});
            return *this;
        }

        bool contains_header(std::string_view header_key) const {
            return headers.find(header_key) != headers.end();
        }

        Builder& body(const char* data) {
            body_ = data;
            headers.insert({"Content-Length", std::to_string(strlen(data) + 1)});
            return *this;
        }

        Builder& body(std::string_view data) {
            body_ = data;
            headers.insert({"Content-Length", std::to_string(data.length())});
            return *this;
        }

        Builder& body(std::string&& data) {
            headers.insert({"Content-Length", std::to_string(data.length())});
            body_ = std::move(data);
            return *this;
        }

        Builder& body(const std::string& data) {
            body_ = data;
            headers.insert({"Content-Length", std::to_string(data.length())});
            return *this;
        }

        HttpResponse build() const {
            if (status_code == 0) {
                throw std::runtime_error("Incomplete HTTP response: Status Code missing");
            }
            return HttpResponse(*this);
        }
    };

private:
    unsigned short status_code;
    std::multimap<std::string_view, std::string_view> headers;
    std::string_view body;

    explicit HttpResponse(const HttpResponse::Builder& builder) : status_code(builder.status_code),
        headers(builder.headers), body(builder.body_) {}

public:
    HttpResponse() : status_code(0) {};

    static HttpResponse::Builder new_builder() {
        return HttpResponse::Builder();
    }

    static HttpResponse::Builder copy_from(const HttpResponse& response) {
        return HttpResponse::Builder(response);
    }

    unsigned get_status_code() const {
        return status_code;
    }

    char* to_c_str() const {
        if (status_code == 0) {
            throw std::runtime_error("Incomplete HTTP response: Status Code missing");
        }
        std::stringstream res;
        res << "HTTP/1.1 " << status_code << ' ' << parse_status_code(status_code) << std::endl;
        for (const auto& [header_key, header_val] : headers) {
            res << header_key << ": " << header_val << std::endl;
        }
        res << std::endl
            << body << std::endl;

        std::string res_str = res.str();

        const char* data = res_str.data();
        size_t data_len = strlen(data);
        char* result = new char[data_len];
        std::copy(data, data + data_len, result);
        return result;
    }

private:
    static std::string_view parse_status_code(unsigned short status_code) {
        switch (status_code) {
            case 200:
                return "OK";
            case 201:
                return "Created";
            case 202:
                return "Accepted";
            case 404:
                return "Not Found";
            case 408:
                return "Request Timeout";
            case 500:
                return "Internal Server Error";
            case 503:
                return "Service Unavailable";
            //...
            default:
                throw std::runtime_error("Unsupported status code: " + std::to_string(status_code));
        }
    }
};

#endif //TICTACTOE_SERVER_HTTP_RESPONSE_H
