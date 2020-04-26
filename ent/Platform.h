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
	Direction stickiness;
} EntityPlatformData;


void EntityPlatform_add(Entity* entity);
void EntityPlatform_update(Entity* entity);
void EntityPlatform_draw(Entity* entity, Viewport* viewport);
void EntityPlatform_destroy(Entity* entity);

bool EntityPlatform_serialize(Entity* entity, char* output);
Entity* EntityPlatform_deserialize(char* input);

#endif