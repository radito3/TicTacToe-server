#ifndef TICTACTOE_SERVER_LOBBYMANAGER_H
#define TICTACTOE_SERVER_LOBBYMANAGER_H

#include "PlayerInfo.h"
#include <unordered_map>
#include <random>
#include <stdexcept>
#include <tuple>

class LobbyManager {
	struct Lobby {
		PlayerInfo player1;
		PlayerInfo player2;
		unsigned search_id;
		bool p1_is_active;
		bool p2_is_active;
		Lobby(PlayerInfo player1, PlayerInfo player2, unsigned search_id) : player1(player1), player2(player2), search_id(search_id){}
	};

	std::unordered_map<unsigned, Lobby> active_lobbies;

public:

	void create_lobby(PlayerInfo player1, PlayerInfo player2, unsigned lobby_id) {
		active_lobbies[lobby_id] = Lobby(player1, player2, lobby_id);
	}

	bool check_for_active_lobby_with_id(std::string player_id) {
		if (active_lobbies.empty()) {
			return false;
		}

		auto lobby = *active_lobbies.begin();
		if (lobby.second.p1_is_active && lobby.second.p2_is_active) {
			return true;
		}

		if (lobby.second.player1.id == player_id) {
			lobby.second.p1_is_active = true;
			return false;
		}

		if (lobby.second.player2.id == player_id) {
			lobby.second.p2_is_active = true;
			return false;
		} 

		return false;
	}
	 
	std::tuple<PlayerInfo, PlayerInfo, unsigned > get_lobby_with_id(unsigned id) {
		if (active_lobbies.find(id) == active_lobbies.end()) {
			throw std::runtime_error("No active lobby with this ID");
		} 
		auto requested_lobby = active_lobbies.at(id);
		return {requested_lobby.player1, requested_lobby.player2, requested_lobby.search_id};
	}

	void erase_first_active_lobby() {
		active_lobbies.erase(active_lobbies.begin());
	}
};

#endif
