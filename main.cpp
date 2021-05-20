#include "HttpServer.h"

int main() {
    HttpServer server;

    server.register_handler<HttpMethod::GET>("/", [](const HttpRequestContext &context) {
                                            return HttpResponse::new_builder()
                                                    .status(200)
                                                    .build();
                                        });

    server.register_handler<HttpMethod::POST>("/search/{search_id}", [](const HttpRequestContext &context) {
        auto search_id = context.path_params.at("search_id");
        return HttpResponse::new_builder()
                .status(200)
                .body(search_id)
                .build();
    });

    server.register_handler<HttpMethod::PUT>("/test/{search_id}/{path_id}", [](const HttpRequestContext &context) {
        auto search_id = context.path_params.at("search_id");
        auto path_id = context.path_params.at("path_id");
        return HttpResponse::new_builder()
                .status(201)
                .body(std::string(search_id) + "/" + std::string(path_id))
                .build();
    });

    server.register_handler<HttpMethod::GET>("/headers", [](const HttpRequestContext &context) {
        std::string s;
        for (auto& [h_key, h_val] : context.headers) {
            s += h_key;
            s += ": ";
            s += h_val;
        }
        return HttpResponse::new_builder()
                .status(202)
                .body(s)
                .build();
    });

    server.register_handler<HttpMethod::GET>("/query", [](const HttpRequestContext &context) {
        std::string s;
        for (auto& [h_key, h_val] : context.query_params) {
            s += h_key;
            s += ": ";
            s += h_val;
        }
        return HttpResponse::new_builder()
                .status(200)
                .body(s)
                .build();
    });

    server.register_handler<HttpMethod::GET>("/fragment", [](const HttpRequestContext &context) {
        return HttpResponse::new_builder()
                .status(200)
                .body(context.fragment)
                .build();
    });

    server.register_handler<HttpMethod::POST>("/body", [](const HttpRequestContext &context) {
        return HttpResponse::new_builder()
                .status(200)
                .body(context.body)
                .build();
    });

    server.register_handler<HttpMethod::POST>("/cl-address", [](const HttpRequestContext &context) {
        return HttpResponse::new_builder()
                .status(200)
                .body(context.client_address)
                .build();
    });

    server.start();
    return 0;
}
