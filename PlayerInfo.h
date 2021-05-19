#ifndef TICTACTOE_SERVER_PLAYERINFO_H
#define TICTACTOE_SERVER_PLAYERINFO_H

#include <string>

struct PlayerInfo {
	std::string id;
	std::string address;
	bool is_active;
};

#endif
