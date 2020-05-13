//
// Created by fabian on 07.05.20.
//

#include "Tileset.h"


RectangleInt Animation_get_frame(Animation* animation, int ticks)
{
	if (animation->length == 1) {
		return Animation_get_frame_at(animation, 0);
	}

	int index = (ticks / animation->ticks_per_frame) % animation->length;
	return Animation_get_frame_at(animation, index);
}


RectangleInt Animation_get_frame_at(Animation* animation, int index)
{
	return (RectangleInt) {
		.position = {
				(animation->position.x + index) * animation->tileset->tile_width,
				animation->position.y * animation->tileset->tile_height
		},
		.size = {
				animation->tileset->tile_width,
				animation->tileset->tile_height
		}
	};
}