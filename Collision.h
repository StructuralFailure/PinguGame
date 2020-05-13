#ifndef H_COLLISION
#define H_COLLISION

#include "Graphics.h"
#include "Entity.h"


typedef enum CollisionType {
	CT_COLMAP,
	CT_SOLID_ENTITY,
	CT_RECTANGLE,
	__CT_COUNT
} CollisionType;


typedef struct Collision {
	CollisionType type;
	union {
		Entity* solid_entity;
		Rectangle rect;
		Vector2DInt colmap_pos;
	};
	CollidedWith collided_with;
} Collision;


#endif