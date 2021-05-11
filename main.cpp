#include "HttpServer.h"

int main() {
    HttpServer server;
    server.register_connection_handler({HttpMethod::GET, "/"},
                                       [](const HttpRequestContext& context) {
                                           return HttpResponse::new_builder()
                                                   .status(200)
                                                   .build();
                                       });
    server.start();
    return 0;
}
