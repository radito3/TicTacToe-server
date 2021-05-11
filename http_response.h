#ifndef TICTACTOE_SERVER_HTTP_RESPONSE_H
#define TICTACTOE_SERVER_HTTP_RESPONSE_H

#include <cstring>

class HttpResponse {
public:
    class Builder {
        unsigned int status_code = -1;
        std::multimap<std::string_view, std::string_view> headers;
        std::string_view body_;

        friend class HttpResponse;

    public:
        Builder& status(unsigned int sc) {
            status_code = sc;
            return *this;
        }

        Builder& header(std::string_view header_key, std::string_view header_value) {
            headers.insert({header_key, header_value});
            return *this;
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
            if (status_code == -1) {
                throw std::runtime_error("Incomplete HTTP response: Status Code missing");
            }
            return HttpResponse(*this);
        }
    };

private:

    unsigned short status_code;
    std::string_view status_code_text;
    std::multimap<std::string_view, std::string_view> headers;

    std::string_view body;

    explicit HttpResponse(const HttpResponse::Builder& builder) : status_code(builder.status_code),
                                                                  body(builder.body_) {
        for (auto&& pair : builder.headers) {
            headers.insert(pair);
        }
    }

public:
    HttpResponse() = delete;

    static HttpResponse::Builder new_builder() {
        return HttpResponse::Builder();
    }

    void add_header(std::string_view header_key, std::string_view header_value) {
        headers.insert({header_key, header_value});
    }

    bool contains_header(std::string_view header_key) const {
        return headers.find(header_key) != headers.end();
    }

    char* to_c_str() const {
        std::stringstream res;
        res << "HTTP/1.1" << status_code << ' ' << status_code_text << std::endl;
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
};

#endif //TICTACTOE_SERVER_HTTP_RESPONSE_H
