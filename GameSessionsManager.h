#ifndef TICTACTOE_GAMESESSIONSMANAGER_H
#define TICTACTOE_GAMESESSIONSMANAGER_H

#include <thread_pool.h>
#include <GrpcDelegatingWriter.h>
#include <GrpcDelegatingReader.h>
#include <Player.h>
#include "PlayerInfo.h"
#include <GameSession.h>

class GameSessionsManager {
	ThreadPool game_sessions_pool;

public:
	void start_game_session(PlayerInfo player1, PlayerInfo player2) {
		Player first_player(player1.id, symbol::CROSS, new GrpcDelegatingWriter(player1.address), new GrpcDelegatingReader(player1.address));
		Player second_player(player2.id, symbol::CIRCLE, new GrpcDelegatingWriter(player2.address), new GrpcDelegatingReader(player2.address));

		game_sessions_pool.submit_job([=]() {
			GameSession session(first_player, second_player);
			session.play();
		});
	}
};

#endif
