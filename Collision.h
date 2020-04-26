#ifndef H_COLLISION
#define H_COLLISION

#include "Graphics.h"
#include "Entity.h"


typedef enum CollisionType {
	CT_NO_COLLISION,
	CT_COLMAP,
	CT_SOLID_ENTITY,
	__CT_COUNT
} CollisionType;


typedef struct Collision {
	CollisionType type;
	union {
		Entity* solid_entity;
		Rectangle* rect;
	};
	Direction direction;
} Collision;


#endif