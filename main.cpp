#include "HttpServer.h"
#include <GameSession.h>
#include <Player.h>
#include "io/GrpcDelegatingWriter.h"
#include "io/GrpcDelegatingReader.h"

int main() {
    GameSession session(Player("pl_1", Symbol::CROSS, new GrpcDelegatingWriter, new GrpcDelegatingReader),
                        Player("pl_2", Symbol::CIRCLE, new GrpcDelegatingWriter, new GrpcDelegatingReader));

    HttpServer server;
    server.register_handler({HttpMethod::GET, "/"},
                            [](const HttpRequestContext &context) {
                                return HttpResponse::new_builder()
                                        .status(200)
                                        .build();
                            });

    session.play();

    server.start();
    return 0;
}
