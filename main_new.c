#include <stdio.h>

#include "Game.h"
#include "Log.h"

int main(int argc, char** argv)
{
	Log_set_flush_after_printing(false);

	Game* game = Game_create();
	if (!game) {
		Log_error("PinguGame", "failed to create game.");
		return 1;
	}

	Game_start(game);
	Game_destroy(game);

	Log("PinguGame", "game has ended.");
	return 0;
}