#ifndef TICTACTOE_SERVER_SEARCHMANAGER_H
#define TICTACTOE_SERVER_SEARCHMANAGER_H

#include <unordered_map>
#include <random>
#include "PlayerInfo.h"

class SearchManager {

	std::unordered_map<unsigned, PlayerInfo> active_searches;

public:
	
	bool check_for_active_search( PlayerInfo player) {
		if (active_searches.empty()) {
			active_searches[std::mt19937(std::random_device()())()] = player ;
			return false;
		} 
		return true;
	}

	std::pair<unsigned, PlayerInfo> get_player_info_with_search_id() {
		return *active_searches.begin();
	}

	void erase_first_active_search() {
		active_searches.erase(active_searches.begin());
	}
};

#endif
