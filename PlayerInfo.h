#ifndef TICTACTOE_SERVER_PLAYERINFO_H
#define TICTACTOE_SERVER_PLAYERINFO_H

#include <string>

struct PlayerInfo {
	std::string id;
	std::string address;

	PlayerInfo(std::string id, std::string address) : id(id), address(address){}
};

#endif
