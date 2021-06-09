#include "HttpServer.h"
#include "SearchManager.h"
#include "LobbyManager.h"
#include "PlayerInfo.h"
#include "GameSessionsManager.h"

int main() {
    HttpServer server;
    SearchManager search_manager;
    LobbyManager lobby_manager;
    GameSessionsManager game_sessions_manager;

    server.register_handler<HttpMethod::GET>("/headers", [](const HttpRequestContext &context) {
        std::string s;
        for (auto& [h_key, h_val] : context.get_headers()) {
            s += h_key;
            s += ": ";
            s += h_val;
            s += '\n';
        }
        return HttpResponse::new_builder()
                .status(202)
                .body(s)
                .build();
    });

    server.register_handler<HttpMethod::GET>("/query", [](const HttpRequestContext &context) {
        std::string s;
        for (auto& [q_key, q_val] : context.get_query_params()) {
            s += q_key;
            s += ": ";
            s += q_val;
            s += '\n';
        }
        return HttpResponse::new_builder()
                .status(200)
                .body(s)
                .build();
    });

    server.register_handler<HttpMethod::GET>("/fragment", [](const HttpRequestContext &context) {
        return HttpResponse::new_builder()
                .status(200)
                .body(context.get_fragment())
                .build();
    });

    server.register_handler<HttpMethod::POST>("/body", [](const HttpRequestContext &context) {
        return HttpResponse::new_builder()
                .status(200)
                .body(context.get_body())
                .build();
    });

    server.register_handler<HttpMethod::POST>("/search/{id, ip_address}", [](const HttpRequestContext &context) {
        std::string player_id = context.path_params.at{"id"};
        std::string player_address = context.path_params.at{"ip_address"};
        PlayerInfo player(search_id, client_address);

        if(!search_manager.check_for_active_search(player)){
            return HttpResponse::new_builder().status(204).build();
        } else {
            std::pair<unsigned, PlayerInfo> active_searching_player = search_manager.get_player_info_with_search_id();
            lobby_manager.create_lobby(player, active_searching_player.second, active_searching_player.first);
            search_manager.erase_first_active_search();
        };

        return HttpResponse::new_builder()
                .status(200)
                .build();
    });

    server.register_handler<HttpMethod::GET>("/search/{search_id, player_id}", [](const HttpRequestContext &context) {
        unsigned search_id = context.path_params.at{"search_id"};
        std::string player_id = context.path_params.at{"player_id"};

        if(!lobby_manager.check_for_active_lobby_with_id(player_id)){
            return HttpResponse::new_builder().status(204).build();
        } else {
            auto [player1, player2, lobby_id] = lobby_manager.get_lobby_with_id(search_id);
            game_sessions_manager.start_game_session(player1, player2);
            lobby_manager.erase_first_active_lobby();
        }


        return HttpResponse::new_builder()
                .status(200)
                .body(search_id)
                .build();

    })

    server.start();
    return 0;
}
