// created by fabian (2020-05-07)

#include <stdlib.h>

#include <SDL2/SDL.h>
#include <Util.h>
#include "WorldController.h"
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


typedef struct WorldController WC;


static void draw_hud(WC* wc);
static void draw_frame(SDL_Texture* texture, Vector2D* position, Vector2DInt* frame_dimensions, Vector2DInt* frame_index);

static SDL_Texture* tex_hud;


WC* WorldController_create(void)
{
	Log("WorldController_create", "creating.");

	if (!tex_hud && !(tex_hud = SDLHelper_load_texture("assets/gfx/hud.bmp"))) {
		Log_error("WorldController_create", "failed to load texture for HUD.");
		return NULL;
	}

	WC* wc = calloc(1, sizeof(WC));
	if (!wc) {
		Log_error("WorldController_create", "failed to allocate memory.");
		return NULL;
	}
	wc->current_score = 123;

	Log("WorldController_create", "created.");
	return wc;
}


void WorldController_prepare_update(WC* wc)
{
	// pass.
}


void WorldController_finalize_update(WC* wc)
{

}


void WorldController_draw(WC* wc, Viewport* viewport)
{
	draw_hud(wc);
}


void WorldController_destroy(WC* wc)
{
	free(wc);
}


void WorldController_added_entity(WorldController* wc, Entity* entity)
{
	if (wc->player || entity->type != ET_PLAYER) {
		return;
	}
	wc->player = entity;
}


void WorldController_removing_entity(WorldController* wc, Entity* entity)
{
	if (entity == wc->player) {
		wc->player = NULL;
	}
}


static void draw_hud(WC* wc)
{
	EntityPlayerData* player_data;
	if (!wc->player || (player_data = wc->player->data) == NULL) {
		return;
	}


	Vector2DInt frame_dimensions = { HUD_FRAME_WIDTH, HUD_FRAME_HEIGHT };
	Vector2D current_pos = HUD_MARGIN;

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
