#ifndef ENTITY_H
#define ENTITY_H

#include "Forward.h"
#include "Game.h"
#include "Graphics.h"
#include "SDLHelper.h"


typedef enum EntityType {
	ET_PLAYER = 1,
	ET_ENEMY,
} EntityType;

struct Entity {
	EntityType type;
	Game* game;
	void* data;
	Rectangle rect; /* describes the collision rectangle */
	Rectangle rect_prev; 

	/* viewport support */
	Direction (*get_direction)();

	void (*add)(struct Entity* entity);
	void (*update)(struct Entity* entity);
	void (*draw)(struct Entity* entity, Viewport* viewport);
	void (*remove)(struct Entity* entity);
	void (*collide)(struct Entity* entity, struct Entity* entity_collide);
};


Entity* Entity_create();


#endif
