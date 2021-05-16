#ifndef TICTACTOE_GAMESESSION_H
#define TICTACTOE_GAMESESSION_H

#include "SearchManager.h"
#include <thread_pool.h>
#include <GameSession.h>

class GameSessionsManager {
	ThreadPool threadpool;

public:

	void start_game_session(SearchManager::PlayerInfo player1, SearchManager::PlayerInfo player2) {
		threadpool.submit_job([&]() {
			GameSession session(Player(player1.id, symbol::CROSS, new GrpcDelegatingWriter(player1.adress), new GrpcDelegatingReader(player1.adress)),
												Player(player2.id, symbol::CIRCLE, new GrpcDelegatingWriter(player2.adress), new GrpcDelegatingReader(player2.adress));
			session.play();
		});
	}
};

#endif