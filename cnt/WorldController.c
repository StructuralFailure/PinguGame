// created by fabian (2020-05-07)

#include <stdlib.h>

#include <SDL2/SDL.h>
#include <Util.h>
#include "WorldController.h"
#include "Controller.h"
#include "../ent/Player.h"
#include "../Log.h"
#include "../Entity.h"
#include "../Graphics.h"
#include "../Viewport.h"


#define HUD_FRAME_WIDTH  16
#define HUD_FRAME_HEIGHT 16

#define TF_HUD_HEARTS_ROW 0
#define TF_HUD_HEARTS_COL 0
#define HUD_HEARTS_MARGIN 2
#define HUD_MARGIN        (Vector2D) { 12, 12 }

#define HEALTHBAR_FLASH_TICK_COUNT 47


typedef struct WorldController WC;


static void draw_hud(Controller* wc);
static void draw_frame(SDL_Texture* texture, Vector2D* position, Vector2DInt* frame_dimensions, Vector2DInt* frame_index);

static SDL_Texture* tex_hud;


Controller* WorldController_create(void)
{
	Log("WorldController_create", "creating.");

	if (!tex_hud && !(tex_hud = SDLHelper_load_texture("assets/gfx/hud.bmp"))) {
		Log_error("WorldController_create", "failed to load texture for HUD.");
		return NULL;
	}

	Controller* wc = Controller_create();
	if (!wc) {
		Log_error("WorldController_create", "failed to create base controller.");
		return NULL;
	}

	WorldControllerData* data = calloc(1, sizeof(WorldControllerData));
	if (!data) {
		Log_error("WorldController_create", "failed to allocate memory for data.");
		free(wc);
		return NULL;
	}
	wc->data = data;

	wc->added_entity = WorldController_added_entity;
	wc->removing_entity = WorldController_removing_entity;
	wc->finalize_update = WorldController_finalize_update;
	wc->draw = WorldController_draw;

	data->current_score = 123;

	Log("WorldController_create", "created.");
	return wc;
}


void WorldController_prepare_update(Controller* wc)
{
	// pass.
}


void WorldController_finalize_update(Controller* wc)
{
	WorldControllerData* data = wc->data;
	if (!data) {
		return;
	}

	EntityPlayerData* player_data;
	if (!data->player || !(player_data = data->player->data)) {
		return;
	}


	if (data->healthbar_flash_counter > 0) {
		--data->healthbar_flash_counter;
	}

	if (player_data->health != data->previous_player_health) {
		data->healthbar_flash_counter = HEALTHBAR_FLASH_TICK_COUNT;
	}

	data->previous_player_health = player_data->health;
}


void WorldController_draw(Controller* wc, Viewport* viewport)
{
	draw_hud(wc);
}


void WorldController_destroy(Controller* wc)
{
	free(wc->data);
	free(wc);
}


void WorldController_added_entity(Controller* wc, Entity* entity)
{
	WorldControllerData* data = wc->data;
	if (!data) {
		return;
	}

	if (data->player || entity->type != ET_PLAYER) {
		return;
	}
	data->player = entity;

	EntityPlayerData* player_data = entity->data;
	if (!player_data) {
		return;
	}
	data->previous_player_health = player_data->health;
}


void WorldController_removing_entity(Controller* wc, Entity* entity)
{
	WorldControllerData* data = wc->data;
	if (!data) {
		return;
	}

	if (entity == data->player) {
		data->player = NULL;
	}
}


static void draw_hud(Controller* wc)
{
	WorldControllerData* data = wc->data;
	if (!data) {
		return;
	}

	EntityPlayerData* player_data;
	if (!data->player || (player_data = data->player->data) == NULL) {
		return;
	}

	Vector2DInt frame_dimensions = { HUD_FRAME_WIDTH, HUD_FRAME_HEIGHT };
	Vector2D current_pos = HUD_MARGIN;

	if (data->healthbar_flash_counter > 0) {
		if ((data->healthbar_flash_counter / 8) % 2 == 1) {
			return;
		}
	}

	/* draw hearts. */
	int hp_drawn = 0;
	for (hp_drawn = 0; hp_drawn < ENTITY_PLAYER_MAX_HEALTH; hp_drawn += 2) {
		Vector2DInt frame_index = { 2, 0 };
		if (player_data->health - hp_drawn >= 2) {
			frame_index.x = 0;
		} else if (player_data->health - hp_drawn == 1) {
			frame_index.x = 1;
		}
		draw_frame(
			tex_hud,
			&current_pos,
			&frame_dimensions,
			&frame_index
		);
		current_pos.x += HUD_FRAME_WIDTH + HUD_HEARTS_MARGIN;
	}
}


static void draw_frame(SDL_Texture* texture, Vector2D* position, Vector2DInt* frame_dimensions, Vector2DInt* frame_index)
{
	SDL_Rect source_rect = {
			.x = frame_index->x * frame_dimensions->x,
			.y = frame_index->y * frame_dimensions->y,
			.w = frame_dimensions->x,
			.h = frame_dimensions->y
	};

	SDL_Rect dest_rect = {
			.x = (int)position->x,
			.y = (int)position->y,
			.w = frame_dimensions->x,
			.h = frame_dimensions->y
	};

	SDL_RenderCopy(sdl_renderer, texture, &source_rect, &dest_rect);
}
