#include <string.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "../SDLHelper.h"
#include "../Entity.h"
#include "../Graphics.h"
#include "../Game.h"
#include "../Log.h"
#include "../Viewport.h"
#include "Text.h"


#define TILESET_NUMBERS_OFFSET 416
#define CHARACTER_WIDTH  16
#define CHARACTER_HEIGHT 16


SDL_Texture* tex_tileset;


Entity* EntityText_create(char* text) 
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

	entity->type = ET_TEXT;

	entity->draw = EntityText_draw;
	entity->destroy = EntityText_destroy;

	EntityText_set_text(entity, text);

	Log("EntityText", "created with text \"%s\"\n", text);

	return entity;
}


void EntityText_draw(Entity* entity, Viewport* viewport) 
{
	char* text = ((EntityTextData*)(entity->data))->text;
	if (!text) {
		return;
	}

	Rectangle in_tileset_rect = {
		.position = {
			.y = 0
		},
		.size = {
			.x = CHARACTER_WIDTH,
			.y = CHARACTER_HEIGHT
		}
	};



	Rectangle on_screen_rect = {
		.position = Vector2D_sum(viewport->visible.position, entity->rect.position),
		.size = {
			.x = CHARACTER_WIDTH,
			.y = CHARACTER_HEIGHT
		}
	};


	for (int i = 0; text[i] != '\0'; ++i) {
		char c = text[i];
		int tileset_x = 0;
		bool whitespace = false;

		if (c == ' ') {
			whitespace = true;
		} else if (c >= '0' && c <= '9') {
			tileset_x = TILESET_NUMBERS_OFFSET + (CHARACTER_WIDTH * (c - '0'));	
		}

		if (!whitespace) {
			in_tileset_rect.position.x = tileset_x;
			Viewport_draw_texture(viewport, &in_tileset_rect, &on_screen_rect, tex_tileset);
		}

		on_screen_rect.position.x += CHARACTER_WIDTH;
	}
}


void EntityText_destroy(Entity* entity)
{
	printf("[EntityText] destroyed.\n");

	EntityText_set_text(entity, NULL);
	free(entity->data);
	free(entity);
}


void EntityText_set_text(Entity* entity, char* text)
{
	EntityTextData* data = (EntityTextData*)(entity->data);
	data->text = text;
}


bool EntityText_serialize(Entity* entity, char* output) 
{
	return false;
}


Entity* EntityText_deserialize(char* input) 
{
	return NULL;
}