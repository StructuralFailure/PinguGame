#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#include "Game.h"
#include "Entity.h"
#include "Level.h"
#include "Log.h"
#include "Graphics.h"
#include "SDLHelper.h"
#include "Viewport.h"
#include "World.h"
#include "IO.h"


void loop(Game* game);


Game* Game_create()
{
	Log("Game", "creating.");

	IO_init();

	Game* game = calloc(1, sizeof(Game));
	if (!game) {
		Log_error("Game", "failed to allocate memory");
		return NULL;
	}

	World* world = World_load_from_path("assets/lvl/wall_slide_test.lvl", true);
	if (!world) {
		free(game);
		Log_error("Game", "failed to load world");
		return NULL;
	}
	/*world->viewport->locked_onto = world->entities[0];*/

	game->world = world;
	Log("Game", "created.");
	return game;
}


void Game_start(Game* game) 
{
	Log("Game", "starting main game loop.");
	loop(game);
	Log("Game", "main game loop finished.");
}


void Game_destroy(Game* game) 
{
	Log("Game", "destroying");
	World_destroy(game->world);
	Log("Game", "destroyed.");
}


void loop(Game* game)
{
	double tick_rate = TICKS_PER_SECOND;
	unsigned int ms_last_tick = 0;
	unsigned int ms_current;

	SDL_Event e;
	int quit = 0;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}

		ms_current = SDL_GetTicks();
		unsigned int ms_per_tick = 1000 / tick_rate;
		unsigned int ms_delta = ms_current - ms_last_tick;

		if (ms_delta >= ms_per_tick) {
			ms_last_tick = ms_current;
			if (game->world) {
				IO_update_keys();
				World_update(game->world);
			}
		}

		SDL_RenderClear(sdl_renderer);
		if (game->world) {
			World_draw(game->world);
		}
		SDL_RenderPresent(sdl_renderer);
	}
}

#if 0
void loop(Game* game) 
{
	/*Entity* entity_player = EntityPlayer_create();
	Entity* entity_enemy = EntityEnemy_create();

	Game_add_entity(game, entity_player);
	Game_add_entity(game, entity_enemy);*/

	double tick_rate = TICKS_PER_SECOND;
	struct timespec time_last_tick;
	struct timespec time_current;

	clock_gettime(CLOCK_MONOTONIC, &time_last_tick);
	
	SDL_Event e;
	int quit = 0;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			} 
		}

		clock_gettime(CLOCK_MONOTONIC, &time_current);
		long us_per_tick = (long)(1000000 / tick_rate);
		long us_last_tick = (time_last_tick.tv_sec * 1000000) + (time_last_tick.tv_nsec / 1000);
		long us_current = (time_current.tv_sec * 1000000) + (time_current.tv_nsec / 1000);
		long us_delta = us_current - us_last_tick;

		if (us_delta >= us_per_tick) {
			time_last_tick = time_current;

			if (game->world) {
				World_update(game->world);
			}
		}

		SDL_RenderClear(sdl_renderer);
		if (game->world) {
			World_draw(game->world);
		}
		SDL_RenderPresent(sdl_renderer);
	}

}
#endif