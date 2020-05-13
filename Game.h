#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "Forward.h"
#include "Entity.h"
#include "Level.h"
#include "Graphics.h"
#include "Viewport.h"
#include "World.h"

#define TICKS_PER_SECOND 60


typedef enum GameState {
	GS_MENU,
	GS_STAGE,
	GS_LEVEL_EDITOR,
	GS_QUITTING,
	GS_CREDITS,
	GS_SETTINGS
} GameState;


struct Game {
	GameState state;
	World* world;
	int current_stage;
	bool stop;
};


Game* Game_create();
bool Game_start(Game*);
void Game_stop(Game*);
void Game_destroy(Game*);

bool Game_load_menu(Game*);
bool Game_load_stage(Game*, int index);
bool Game_load_level_editor(Game* game);


#endif
