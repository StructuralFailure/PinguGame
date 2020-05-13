//
// Created by fabian on 13.05.20.
//

#ifndef ENTITY_FLAME_SPIRIT_H
#define ENTITY_FLAME_SPIRIT_H


#include "../Graphics.h"
#include "../Entity.h"
#include "../Viewport.h"


typedef enum EntityFlameSpiritState {
	EFSS_DEFAULT,
	EFSS_CHASING
} EntityFlameSpiritState;


typedef struct EntityFlameSpiritData {
	EntityFlameSpiritState state;
	Vector2D velocity;
	bool has_line_of_sight;
} EntityFlameSpiritData;


Entity* EntityFlameSpirit_create();
void    EntityFlameSpirit_destroy(Entity* entity);
bool    EntityFlameSpirit_serialize(char* output);
Entity* EntityFlameSpirit_deserialize(char* input);

void EntityFlameSpirit_update(Entity*);
void EntityFlameSpirit_draw(Entity*, Viewport*);
void EntityFlameSpirit_collide(Entity*, Entity* other_entity);


#endif