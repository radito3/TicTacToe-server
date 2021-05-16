#ifndef TICTACTOE_SERVER_LOBBYMANAGER_H
#define TICTACTOE_SERVER_LOBBYMANAGER_H

#include "SearchManager.h"
#include <unordered_map>
#include <random>

class LobbyManager {
public:
	struct Lobby {
		SearchManager::PlayerInfo player1;
		SearchManager::PlayerInfo player2;
		bool player1_active;
		bool player2_active;
		unsigned search_id;

		Lobby(SearchManager::PlayerInfo player1, SearchManager::PlayerInfo player2, unsigned id) : player1(player1), player2(player2), search_id(id){}

	};

private:
	unordered_map<unsigned, Lobby> active_lobbies;

public:

	void create_lobby(SearchManager::PlayerInfo player1, SearchManager::PlayerInfo player2, unsigned id) {
		active_lobby[std::mt19937(std::random_device()())()] = Lobby(player1, player2, id);
	}

	bool check_for_active_lobby_with_id(std::string player_id ) {
		if (active_lobbies.empty()) {
			return false;
		}
		auto lobby = *active_lobbies.begin();
		if (lobby.player1_active  && lobby.player2_active) {
			return true;
		} else if (lobby.player1.id == player_id) {
			lobby.player1_active = true;
			return false;
		} else if (lobby.player2.id == player_id){
			lobby.player2_active = true;
			return false;
		} 
		return false;
	}
	 
	Lobby get_lobby_with_id(unsigned id) {
		if (active_lobbies.find(id) == active_lobbies.end()) {
			throw std::runtime_error("No active lobby with this ID");
		} 
		return active_lobbies.at(id);
	}

	void erase_first_active_lobby() {
		active_lobby.erase(active_lobby.begin());
	}
};


#endif