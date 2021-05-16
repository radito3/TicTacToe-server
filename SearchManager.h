#ifndef TICTACTOE_SERVER_SEARCHMANAGER_H
#define TICTACTOE_SERVER_SEARCHMANAGER_H

#include <unordered_map>
#include <random>

class SearchManager {
public:
	struct PlayerInfo {
		std::string id;
		std::string adress;
	};

private:
	unordered_map<unsigned, PlayerInfo> active_searches;

public:
	
	bool check_for_active_search( PlayerInfo player) {
		if (active_searches.empty()) {
			active_searches[std::mt19937(std::random_device()())()] = player ;
			return false;
		} else {
			return true;
		}
	}

	PlayerInfo get_player_info() {
		return *active_searches.begin();
	}

	void erase_first_active_search() {
		active_searches.erase(active_searches.begin());
	}
};

#endif