#ifndef ENTITY_PLATFORM_H
#define ENTITY_PLATFORM_H

#include <stdbool.h>

#include "../Graphics.h"
#include "../Entity.h"


typedef struct EntityPlatformData {
	Vector2D origin;
	Vector2D destination;
	float speed;
	bool moving_towards_destination;
} EntityPlatformData;


void EntityPlatform_add(Entity* entity);
void EntityPlatform_update(Entity* entity);
void EntityPlatform_draw(Entity* entity, Viewport* viewport);
void EntityPlatform_destroy(Entity* entity);

#endif