#ifndef ENTITY_H
#define ENTITY_H

#include "Forward.h"
#include "Game.h"
#include "Graphics.h"
#include "World.h"
#include "SDLHelper.h"


typedef enum EntityType {
	ET_PLAYER      = 0,
	ET_ENEMY       = 1,
	ET_TEXT        = 2,
	ET_LINE_DRAWER = 3,
	ET_PLATFORM    = 4,
	ET_ITEM        = 5,
	__ET_COUNT
} EntityType;


typedef enum EntityMessageType {
	EMT_DAMAGE,
	EMT_DO_SOMETHING_ELSE,
	__EMT_COUNT
} EntityMessageType;


struct Entity {
	unsigned long id;

	EntityType type;
	World* world;
	void* data;
	Rectangle rect; /* describes the collision rectangle */
	Rectangle previous_rect; 
	bool is_solid;

	/* viewport support */
	Direction (*get_direction)();

	void (*add)(struct Entity* entity);
	void (*update)(struct Entity* entity);
	void (*draw)(struct Entity* entity, Viewport* viewport);
	void (*destroy)(struct Entity* entity);
	void (*collide)(struct Entity* entity, struct Entity* entity_other);

	void* (*message)(struct Entity* entity, struct Entity* sender, EntityMessageType message_type, void* message_payload);
};


Entity* Entity_create();


#endif
