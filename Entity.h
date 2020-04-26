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
	Rectangle rect_prev; 
	bool is_solid;

	/* viewport support */
	Direction (*get_direction)();

	void (*add)(struct Entity* entity);
	void (*update)(struct Entity* entity);
	void (*draw)(struct Entity* entity, Viewport* viewport);
	void (*destroy)(struct Entity* entity);
	void (*collide)(struct Entity* entity, struct Entity* entity_other);

	void (*message)(struct Entity* sender, struct Entity* receiver, EntityMessageType message_type, void* message_content);
};


Entity* Entity_create();


#endif
