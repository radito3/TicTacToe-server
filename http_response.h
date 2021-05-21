#ifndef TICTACTOE_SERVER_HTTP_RESPONSE_H
#define TICTACTOE_SERVER_HTTP_RESPONSE_H

#include <cstring>
#include <map>

class HttpResponse {
public:
    class Builder {
        unsigned short status_code = 0;
        std::multimap<std::string, std::string> headers;
        std::string body_;

        friend class HttpResponse;

    public:
        Builder() = default;

        explicit Builder(const HttpResponse& res) : status_code(res.status_code),
            headers(res.headers), body_(res.body) {}

        Builder& status(unsigned short sc) {
            status_code = sc;
            return *this;
        }

        Builder& header(std::string header_key, std::string header_value) {
            headers.insert({header_key, header_value});
            return *this;
        }

        bool contains_header(std::string header_key) const {
            return headers.find(header_key) != headers.end();
        }

        Builder& body(const char* data) {
            body_ = data;
            headers.insert({"Content-Length", std::to_string(strlen(data) + 1)});
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
    std::multimap<std::string, std::string> headers;
    std::string body;

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

    friend std::ostream& operator<<(std::ostream& out, const HttpResponse &response) {
        if (response.status_code == 0) {
            throw std::runtime_error("Incomplete HTTP response: Status Code missing");
        }
        out << "HTTP/1.1 " << response.status_code << ' ' << parse_status_code(response.status_code) << std::endl;
        for (const auto& [header_key, header_val] : response.headers) {
            out << header_key << ": " << header_val << std::endl;
        }
        out << std::endl
            << response.body << std::endl;
        return out;
    }

private:
    static std::string parse_status_code(unsigned short status_code) {
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
