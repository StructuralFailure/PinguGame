#include <stdlib.h>

#include <SDL2/SDL.h>
#include "SDLHelper.h"
#include "Viewport.h"
#include "Game.h"
#include "Entity.h"
#include "Level.h"


void update(Viewport* viewport)


Viewport* Viewport_create(Game* game) 
{
	Viewport* viewport = calloc(1, sizeof(Viewport));
	if (!viewport) {
		return NULL;
	}
	viewport->game = game;
	return viewport;
}


void Viewport_draw_texture(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, Texture* texture) 
{
	SDL_Rect sdl_rect_source = SDLHelper_get_sdl_rect(rect_source);
	SDL_Rect sdl_rect_dest = SDLHelper_get_sdl_rect(rect_dest);

	sdl_rect_dest.x -= viewport->visible.x;
	sdl_rect_dest.y -= viewport->visible.y;

	SDL_RenderCopy(sdl_renderer, sdl_rect_source, sdl_rect_dest);
}

void Viewport_draw(Viewport* viewport) 
{
	update(viewport);

	Game* game = viewport->game;

	SDL_Rect sdl_rect_dest = {
		.w = CM_CELL_WIDTH,
		.h = CM_CELL_HEIGHT
	};

	SDL_Rect sdl_rect_source = {
		.y = 0,
		.w = CM_CELL_WIDTH,
		.h = CM_CELL_HEIGHT
	};


	static bool has_dumped = false;

	for (int y = 0; y < game->current_level->height; ++y) {
		for (int x = 0; x < game->current_level->width; ++x) {
			LevelCellTypeProperties* ct_properties = Level_get_cell_type_properties(game->current_level, x, y);
			SDL_Texture* cell_texture = ct_properties->texture;

			if (ct_properties->type == LCT_SOLID_BLOCK) {
				/* change texture of solid block depending on its neighbors */

				int tileset_index = 0;
				tileset_index |= 1 * Level_is_solid(game->current_level, x - 1, y);
				tileset_index |= 2 * Level_is_solid(game->current_level, x, y - 1);
				tileset_index |= 4 * Level_is_solid(game->current_level, x + 1, y);
				tileset_index |= 8 * Level_is_solid(game->current_level, x, y + 1);;

				sdl_rect_source.x = tileset_index * CM_CELL_WIDTH;
			} else {
				sdl_rect_source.x = 0;
			}

			sdl_rect_dest.x = x * CM_CELL_WIDTH;
			sdl_rect_dest.y = y * CM_CELL_HEIGHT;
			SDL_RenderCopy(sdl_renderer, cell_texture, &sdl_rect_source, &sdl_rect_dest);
		}
	}

	has_dumped = true;

	/* call Entity objects' render functions */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = game->entities[i];
		if (ent && ent->draw) {
			ent->draw(ent);
		}		
	}
}


void update(Viewport* viewport)
{
	/* this function updates visible and inner */

}


void Game_draw(Game* game) {
	/* render collision map */

	SDL_Rect sdl_rect_dest = {
		.w = CM_CELL_WIDTH,
		.h = CM_CELL_HEIGHT
	};

	SDL_Rect sdl_rect_source = {
		.y = 0,
		.w = CM_CELL_WIDTH,
		.h = CM_CELL_HEIGHT
	};


	static bool has_dumped = false;

	for (int y = 0; y < game->current_level->height; ++y) {
		for (int x = 0; x < game->current_level->width; ++x) {
			LevelCellTypeProperties* ct_properties = Level_get_cell_type_properties(game->current_level, x, y);
			SDL_Texture* cell_texture = ct_properties->texture;

			if (ct_properties->type == LCT_SOLID_BLOCK) {
				/* change texture of solid block depending on its neighbors */

				int tileset_index = 0;
				tileset_index |= 1 * Level_is_solid(game->current_level, x - 1, y);
				tileset_index |= 2 * Level_is_solid(game->current_level, x, y - 1);
				tileset_index |= 4 * Level_is_solid(game->current_level, x + 1, y);
				tileset_index |= 8 * Level_is_solid(game->current_level, x, y + 1);;

				sdl_rect_source.x = tileset_index * CM_CELL_WIDTH;
			} else {
				sdl_rect_source.x = 0;
			}

			sdl_rect_dest.x = x * CM_CELL_WIDTH;
			sdl_rect_dest.y = y * CM_CELL_HEIGHT;
			SDL_RenderCopy(sdl_renderer, cell_texture, &sdl_rect_source, &sdl_rect_dest);
		}
	}

	has_dumped = true;

	/* call Entity objects' render functions */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = game->entities[i];
		if (ent && ent->draw) {
			ent->draw(ent);
		}		
	}
}


Viewport* Viewport_destroy(Viewport* viewport)
{
	free(viewport);
}