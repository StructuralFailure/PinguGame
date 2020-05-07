//
// Created by fabian on 05.05.20.
//

#ifndef ENTITY_PLAYER_H
#define ENTITY_PLAYER_H

#include <stdbool.h>
#include "Graphics.h"
#include "Entity.h"


#define ENTITY_PLAYER_MAX_HEALTH 20


typedef enum EntityPlayerState {
	EPS_WALKING,
	EPS_JUMPING,
	EPS_FALLING,
	EPS_WALL_SLIDING,
	EPS_EJECTING,
	EPS_SLEDDING,
	EPS_CLIMBING,
	__EPS_COUNT
} EntityPlayerState;


typedef enum EntityPlayerFacing {
	EPF_LEFT,
	EPF_RIGHT
} EntityPlayerFacing;


typedef struct EntityPlayerData {
	EntityPlayerState state;
	EntityPlayerState previous_state;

	Vector2D starting_pos;
	Vector2D velocity;
	EntityPlayerFacing facing;

	int health;

	int jump_charge_counter;

	union {
		Direction wall_slide_direction;
		struct {
			Vector2D eject_velocity;
			int eject_counter;
		};
		int sled_jump_timeout;
	};

	double animation_frame_satisfied;
	int animation_frame;
} EntityPlayerData;


Entity* EntityPlayer_create();
void EntityPlayer_add(Entity* entity);
void EntityPlayer_update(Entity* entity);
void EntityPlayer_draw(Entity* entity, Viewport* viewport);
void EntityPlayer_destroy(Entity* entity);
void EntityPlayer_collide(Entity* entity, Entity* entity_other);
Direction EntityPlayer_get_direction(Entity* entity);
void* EntityPlayer_message(Entity* entity, Entity* sender, EntityMessageType message_type, void* payload);

/* saving and loading */
bool EntityPlayer_serialize(Entity* entity, char* output);
Entity* EntityPlayer_deserialize(char* input);



#endif
