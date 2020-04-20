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


struct Game {
	World* world;
};

/*               */
/* basic methods */
/*               */
Game* Game_create();
void Game_start(Game* game);
void Game_destroy(Game* game);


#endif
