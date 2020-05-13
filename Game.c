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
#include "Stage.h"
#include "cnt/MenuController.h"
#include "cnt/StageController.h"
#include "cnt/LevelEditorController.h"


void loop(Game* game);


static Stage level_editor_stage = {
		0
};

static const Stage stages[] = {
		[0] = {
				.realm = SR_OVERWORLD,
				.type = ST_DEFAULT,
				.name = "Los geht's!",
				.path = "assets/lvl/snail_test.lvl"
		},
		[1] = {
				.realm = SR_OVERWORLD,
				.type = ST_UNDERWATER,
				.name = "Abkühlung",
				.path = "assets/lvl/1-2.lvl"
		}
};


Game* Game_create()
{
	Log("Game", "creating.");

	IO_init();

	Game* game = calloc(1, sizeof(Game));
	if (!game) {
		Log_error("Game", "failed to allocate memory.");
		return NULL;
	}

	Log("Game", "created.");
	return game;
}


bool Game_start(Game* game)
{
	Log("Game", "starting game loop.");

	game->current_stage = 0;
	//Game_load_stage(game, 0);
	Game_load_menu(game);
	loop(game);
	Log("Game", "game loop finished.");
	return true;
}


void Game_destroy(Game* game) 
{
	Log("Game", "destroying");
	World_destroy(game->world);
	Log("Game", "destroyed.");
}


bool Game_load_stage(Game* game, int index)
{
	if (game->world) {
		World_destroy(game->world);
	}

	Controller* controller = StageController_create();
	if (!controller) {
		Log_error("Game_load_stage", "failed to create stage controller.");
		return false;
	}

	game->world = World_load_from_path(stages[index].path, controller, true);
	if (!game->world) {
		Log_error("Game_load_stage", "failed to load stage at index %d.");
		return false;
	}

	Log("Game_load_stage", "stage loaded.");
	return true;
}


void Game_stop(Game* game) {
	game->stop = true;
}


void loop(Game* game)
{
	unsigned int ms_per_tick = 1000 / TICKS_PER_SECOND;
	unsigned int ms_last_tick = 0;
	int ms_delay;

	SDL_Event e;

	while (!game->stop) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				game->stop = true;
			}
		}

		if (game->world) {
			IO_update();
			World_update(game->world);
		}

		SDL_RenderClear(sdl_renderer);
		if (game->world) {
			World_draw(game->world);
		}
		SDL_RenderPresent(sdl_renderer);

		ms_delay = ms_per_tick - (SDL_GetTicks() - ms_last_tick);
		if (ms_delay > 0) {
			SDL_Delay(ms_delay);
		}
		ms_last_tick = SDL_GetTicks();

		/* check whether we should change worlds. */
		Controller* controller = game->world->controller;
		if (controller->requested_world == RW_NONE) {
			continue;
		}

		switch (controller->type) {
		case CT_MENU_CONTROLLER:
			/* always switch to first level. */
			Game_load_stage(game, 0);
			break;
		case CT_LEVEL_EDITOR_CONTROLLER:
			Game_load_level_editor(game);
			break;
		case CT_STAGE_CONTROLLER:
			/* switch to next world, depending on stage. */
			/* TODO: implement. */
			break;
		default:;
		}
	}
}


bool Game_load_menu(Game* game)
{
	if (game->world) {
		World_destroy(game->world);
	}

	Controller* controller = MenuController_create();
	if (!controller) {
		Log_error("Game_load_menu", "failed to create controller.");
		return NULL;
	}
	if (!(game->world = World_create(controller))) {
		Log_error("Game_load_menu", "failed to create world.");
		MenuController_destroy(controller);
		return false;
	}

	game->state = GS_MENU;
	return true;
}


bool Game_load_level_editor(Game* game)
{
	if (game->world) {
		World_destroy(game->world);
	}

	Controller* controller = LevelEditorController_create();
	if (!controller) {
		Log_error("Game_load_level_editor", "failed to create controller.");
		return NULL;
	}

	if (!(game->world = World_create(controller))) {
		Log_error("Game_load_level_editor", "failed to create world.");
		LevelEditorController_destroy(controller);
		return false;
	}

	return true;
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