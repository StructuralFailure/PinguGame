//
// Created by fabian on 12.05.20.
//

#include "Text.h"
#include "../Entity.h"
#include "../Log.h"
#include "../Util.h"

#define TEX_PATH_FONT_SMALL "assets/gfx/font_small.bmp"
#define TEX_PATH_FONT_LARGE "assets/gfx/font_large.bmp"

#define DEFAULT_CHARACTER_PADDING_SMALL 2
#define DEFAULT_CHARACTER_PADDING_LARGE 4

#define ANIMS_ROW_LETTERS 0
#define ANIMS_ROW_NUMBERS 1


typedef EntityTextData Data;
typedef EntityTextFontType FontType;


static Tileset ts_small = {
		.tile_width = 5,
		.tile_height = 5
};


static Tileset ts_large = {
		.tile_width = 16,
		.tile_height = 16
};


static Animation anim_small_letters = {
	.tileset = &ts_small,
	.length = 26
};

static Animation anim_small_numbers = {
	.tileset = &ts_small,
	.length = 10,
	.position = { 0, 1 }
};

static Animation* anims_small[2] = {
	&anim_small_letters,
	&anim_small_numbers
};


static Animation anim_large_letters = {
	.tileset = &ts_large,
	.length = 26
};

static Animation anim_large_numbers = {
	.tileset = &ts_large,
	.length = 10,
	.position = { 0, 1 }
};

static Animation* anims_large[2] = {
		&anim_large_letters,
		&anim_large_numbers
};



Entity* EntityText_create(FontType font_type)
{
	if (!ts_small.texture) {
		ts_small.texture = SDLHelper_load_texture(TEX_PATH_FONT_SMALL);
	}
	if (!ts_large.texture) {
		ts_large.texture = SDLHelper_load_texture(TEX_PATH_FONT_LARGE);
	}

	Entity* text = Entity_create();
	if (!text) {
		Log_error("EntityText_create", "failed to create entity.");
		return NULL;
	}
	text->type = ET_TEXT;

	Data* data = calloc(1, sizeof(Data));
	if (!data) {
		Log_error("EntityText_create", "failed to allocate memory for data.");
		Entity_destroy(text);
		return NULL;
	}

	data->font_type = font_type;
	if (font_type == ETFT_SMALL) {
		data->character_padding = DEFAULT_CHARACTER_PADDING_SMALL;
		data->anims = anims_small;
	} else {
		data->character_padding = DEFAULT_CHARACTER_PADDING_LARGE;
		data->anims = anims_large;
	}

	return text;
}


void EntityText_destroy(Entity* entity)
{
	if (entity == NULL) {
		return;
	}
	free(entity->data);
	free(entity);
}


void EntityText_draw(Entity* entity)
{
	ENTITY_DATA_ASSERT(Text);

	if (!data->text) {
		return;
	}

	Vector2D current_position = entity->rect.position;
	for (int i = 0; data->text[i] != '\0'; ++i) {
		int c = data->text[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
			continue;
		}

		Animation* anim = &(anim[ANIMS_ROW_LETTERS]);

		int letter_index = data->text[i] - 'A';
		RectangleInt anim_frame = Animation_get_frame_at(anim, letter_index);
		current_position.x += (float)(anim->tileset->tile_width + data->character_padding);

		Rectangle rect_dest = {
				.position = current_position,
				.size = {
						anim->tileset->tile_width,
						anim->tileset->tile_height
				}
		};


		//SDL_RenderCopy(sdl_renderer, anim->tileset->texture, )
	}
}
