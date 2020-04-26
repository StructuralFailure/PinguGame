#ifndef ENTITY_PLATFORM_H
#define ENTITY_PLATFORM_H

#include <stdbool.h>

#include "../Graphics.h"
#include "../Entity.h"


typedef enum EntityPlatformMovementType {
	EPMT_LINEAR      = 1,
	EPMT_CIRCULAR    = 2,
	EPMT_RECTANGULAR = 3,
	__EPMT_COUNT
} EntityPlatformMovementType;


typedef struct EntityPlatformData {
	Direction stickiness;

	EntityPlatformMovementType movement_type;
	Vector2D origin;
	union {
		struct {
			Vector2D destination;
			float speed;
		} lm; /* linear movement */
		struct {
			float radius;
		} cm; /* circular movement */
	};
	unsigned long ticks;
	unsigned long movement_duration;
} EntityPlatformData;


void EntityPlatform_add(Entity* entity);
void EntityPlatform_update(Entity* entity);
void EntityPlatform_draw(Entity* entity, Viewport* viewport);
void EntityPlatform_destroy(Entity* entity);

bool EntityPlatform_serialize(Entity* entity, char* output);
Entity* EntityPlatform_deserialize(char* input);

#endif