// created by fabian (2020-05-07)

#include <stdlib.h>

#include <SDL2/SDL.h>
#include <Util.h>
#include "StageController.h"
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


typedef struct StageController ss;


static void draw_hud(Controller* ss);
static void draw_frame(SDL_Texture* texture, Vector2D* position, Vector2DInt* frame_dimensions, Vector2DInt* frame_index);

static SDL_Texture* tex_hud;


Controller* StageController_create(void)
{
	Log("StageController_create", "creating.");

	if (!tex_hud && !(tex_hud = SDLHelper_load_texture("assets/gfx/hud.bmp"))) {
		Log_error("StageController_create", "failed to load texture for HUD.");
		return NULL;
	}

	Controller* ss = Controller_create();
	if (!ss) {
		Log_error("StageController_create", "failed to create base controller.");
		return NULL;
	}
	ss->type = CT_STAGE_CONTROLLER;

	StageControllerData* data = calloc(1, sizeof(StageControllerData));
	if (!data) {
		Log_error("StageController_create", "failed to allocate memory for data.");
		free(ss);
		return NULL;
	}
	ss->data = data;

	ss->added_entity = StageController_added_entity;
	ss->removing_entity = StageController_removing_entity;
	ss->finalize_update = StageController_finalize_update;
	ss->draw = StageController_draw;
	ss->destroy = StageController_destroy;

	data->current_score = 123;

	Log("StageController_create", "created.");
	return ss;
}


void StageController_prepare_update(Controller* ss)
{
	// pass.
}


void StageController_finalize_update(Controller* ss)
{
	StageControllerData* data = ss->data;
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


void StageController_draw(Controller* ss, Viewport* viewport)
{
	draw_hud(ss);
}


void StageController_destroy(Controller* ss)
{
	Log("StageController_destroy", "destroying.");

	free(ss->data);
	free(ss);

	Log("StageController_destroy", "destroyed.");
}


void StageController_add(Controller* ss)
{
	
}


void StageController_added_entity(Controller* ss, Entity* entity)
{
	StageControllerData* data = ss->data;
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


void StageController_removing_entity(Controller* ss, Entity* entity)
{
	StageControllerData* data = ss->data;
	if (!data) {
		return;
	}

	if (entity == data->player) {
		data->player = NULL;
	}
}


static void draw_hud(Controller* ss)
{
	StageControllerData* data = ss->data;
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
