#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "SDLHelper.h"
#include "Graphics.h"
#include "Level.h"
#include "Game.h"
#include "Entity.h"
#include "ent/Player.h"
#include "ent/Enemy.h"

#define START_TICK_RATE 60
#define LEVEL_PATH "assets/lvl/test"




void game(void);

int main(int argc, char** argv) 
{
	/* SDL stuff */
	E_SDL sdl_e;
	if ((sdl_e = SDLHelper_init()) != E_SDL_SUCCESS) {
		printf("failed to initialize SDL. error: %d\n", sdl_e);
		return 1;
	}
	game();
	SDLHelper_quit();

	return 0;
}


void game(void) 
{
	Game* game = Game_create();
	Level* level = Level_create_from_file(LEVEL_PATH);
	if (!level) {
		fprintf(stderr, "[game] failed to load level from %s.\n", LEVEL_PATH);
		return;
	}
	Game_set_level(game, level);
	Entity* entity_player = EntityPlayer_create();
	Entity* entity_enemy = EntityEnemy_create(32, 32);
	Game_add_entity(game, entity_player);
	Game_add_entity(game, entity_enemy);

	double tick_rate = START_TICK_RATE;
	struct timespec time_last_tick;
	struct timespec time_current;

	clock_gettime(CLOCK_MONOTONIC, &time_last_tick);
	
	SDL_Event e;
	int quit = 0;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			} /*else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_LEFT:
					move_x = -1;
					break;
				case SDLK_UP:
					move_y = -1;
					break;
				case SDLK_RIGHT:
					move_x = 1;
					break;
				case SDLK_DOWN:
					move_y = 1;
					break;
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			} else if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
				case SDLK_LEFT:
				case SDLK_RIGHT:
					move_x = 0;
					break;
				case SDLK_UP:
				case SDLK_DOWN:
					move_y = 0;
					break;
				}
			}*/
		}
		/* perform game tick if necessary */
		clock_gettime(CLOCK_MONOTONIC, &time_current);
		long us_per_tick = (long)(1000000 / tick_rate);
		long us_last_tick = (time_last_tick.tv_sec * 1000000) + (time_last_tick.tv_nsec / 1000);
		long us_current = (time_current.tv_sec * 1000000) + (time_current.tv_nsec / 1000);
		long us_delta = us_current - us_last_tick;

		if (us_delta >= us_per_tick) {
			time_last_tick = time_current;

			Game_update(game);
		}

		SDL_RenderClear(sdl_renderer);
		Game_draw(game);
		SDL_RenderPresent(sdl_renderer);
	}
}
