#ifndef ENTITY_PLATFORM_H
#define ENTITY_PLATFORM_H

#include <stdbool.h>

#include "Graphics.h"


typedef struct EntityPlatformData {
	Vector2D origin;
	Vector2D destination;
	float speed;
	bool moving_towards_destination;
} EntityPlatformData;


#endif