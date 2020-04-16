#include <string.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "../SDLHelper.h"
#include "../Entity.h"
#include "../Graphics.h"
#include "../Game.h"
#include "Text.h"


#define TILESET_NUMBERS_OFFSET 416
#define CHARACTER_WIDTH 16


SDL_Texture* tex_tileset;


Entity* EntityText_create(const char* text) 
{
	if (!tex_tileset) {
		tex_tileset = SDLHelper_load_texture("assets/gfx/letter_tileset.bmp");
	}

	Entity* entity = Entity_create();
	if (!entity) {
		return NULL;
	}

	EntityTextData* data = calloc(1, sizeof(EntityTextData));
	if (!data) {
		free(entity);
		return NULL;
	}
	entity->data = data;

	entity->draw = EntityText_draw;
	entity->remove = EntityText_remove;

	EntityText_set_text(entity, text);
	return entity;
}


void EntityText_draw(Entity* entity) 
{
	char* text = ((EntityTextData*)(entity->data))->text;

	Rectangle in_tileset_rect = {
		.position = {
			.y = 0
		},
		.size = {
			.x = CHARACTER_WIDTH,
			.y = CHARACTER_WIDTH /* TODO: add definition of height */
		}
	};

	Rectangle on_screen_rect = {
		.position = {
			.y = entity->rect.position.y
		},
		.size = entity->rect.size
	};

	for (int i = 0; text[i] != '\0'; ++i) {
		char c = text[i];
		int tileset_x = 0;

		if (c >= '0' && c <= '9') {
			tileset_x = TILESET_NUMBERS_OFFSET + (CHARACTER_WIDTH * (c - '0'));
		}

		in_tileset_rect.position.x = tileset_x;
		on_screen_rect.position.x = entity->rect.position.x + (CHARACTER_WIDTH * i);

		SDL_Rect sdl_source_rect = SDLHelper_get_sdl_rect(&in_tileset_rect);
		SDL_Rect sdl_dest_rect = SDLHelper_get_sdl_rect(&on_screen_rect);
		SDL_RenderCopy(sdl_renderer, tex_tileset, &sdl_source_rect, &sdl_dest_rect);;
	}
}


void EntityText_remove(Entity* entity)
{
	EntityText_set_text(entity, NULL);
	free(entity->data);
}


void EntityText_set_text(Entity* entity, const char* text)
{
	EntityTextData* data = (EntityTextData*)(entity->data);
	if (data->text) {
		free(data->text);
		data->text = NULL;
	}
	if (text) {
		data->text = strdup(text);
	}
}