#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "SDLHelper.h"
#include "Graphics.h"
#include "Level.h"
#include "Log.h"
#include "Game.h"
#include "Entity.h"
#include "ent/Entities.h"
#include "World.h"

#define START_TICK_RATE 60
#define LEVEL_PATH "assets/lvl/viewport_test"


void test_game_creation(void);
void test_world_loading(void);
void game(void);


int main(int argc, char** argv) 
{
	Log_set_flush_after_printing(true);
	E_SDL sdl_e;
	if ((sdl_e = SDLHelper_init()) != E_SDL_SUCCESS) {
		Log_error("PinguGame", "failed to initialize SDL (error %d). aborting.", sdl_e);
		return 1;
	}
	test_game_creation();
	SDLHelper_quit();
	return 0;
}


void test_game_creation(void)
{
	Game* game = Game_create();
	Game_start(game);
	Game_destroy(game);
}


void test_world_loading(void) 
{
	World* world = World_load_from_path("assets/lvl/entity_test.lvl", true);
	if (!world) {
		Log_error("PinguGame", "failed to load world.");
		return;
	}

	for (int i = 0; i < 32; ++i) {
		Entity* entity = world->entities[i];
		if (!entity) {
			continue;
		}
		printf(
			"level  [ w = %d | h = %d ]\n"
			"entity [ type = %d | x = %f | y = %f ]\n",
			world->level->width, world->level->height,
			entity->type, entity->rect.position.x, entity->rect.position.y
		);
	} 
	
	World_destroy(world);
}


void game(void) 
{
	#if 0
	Game* game = Game_create();
	Level* level = Level_create_from_path(LEVEL_PATH);
	if (!level) {
		fprintf(stderr, "[game] failed to load level from %s.\n", LEVEL_PATH);
		return;
	}
	Game_set_level(game, level);

	Entity* entity_player = EntityPlayer_create();
	Entity* entity_enemy = EntityEnemy_create();

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
			} 
		}
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

	Game_destroy(game);
	#endif
}
