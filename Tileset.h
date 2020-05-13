//
// Created by fabian on 07.05.20.
//

#ifndef TILESET_H
#define TILESET_H

#include <SDL2/SDL.h>
#include "Graphics.h"


#define STATIC_ANIMATION(TILESET, POSITION_X, POSITION_Y)   \
	(Animation) {                                           \
        .tileset = TILESET,                                 \
        .position = { POSITION_X, POSITION_Y },             \
        .length = 1                                         \
	}



typedef struct Tileset {
	int tile_width;
	int tile_height;
	SDL_Texture* texture;
} Tileset;


typedef struct Animation {
	Tileset* tileset;
	Vector2DInt position;
	int length; /* size in horizontal tiles */
	int ticks_per_frame;
} Animation;


RectangleInt Animation_get_frame(Animation*, int ticks);
RectangleInt Animation_get_frame_at(Animation*, int index);


#endif
