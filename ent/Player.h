#ifndef H_ENT_PLAYER
#define H_ENT_PLAYER

#include "Text.h"
#include "../Entity.h"
#include "../Viewport.h"
#include "../Graphics.h"


#define TEXT_SCORE_SIZE 32


typedef enum EntityPlayerState {
	EPS_DEFAULT,
	EPS_JUMPING_CHARGING, /* longer presses of up lead to higher jumps */
	EPS_JUMPING,
	EPS_FALLING,
	EPS_CLIMBING,
} EntityPlayerState;


typedef enum EntityPlayerFacing {
	EPF_LEFT,
	EPF_RIGHT
} EntityPlayerFacing;


typedef struct EntityPlayerData {
	EntityPlayerState state;
	int powerup_level;
	int health;
	Vector2D velocity;
	EntityPlayerFacing facing;
	int jump_charge_counter;
	Vector2D starting_pos;

	Entity* entity_text;
	char entity_text_text[TEXT_SCORE_SIZE];

	bool key_up_pressed_prev;
} EntityPlayerData;


Entity* EntityPlayer_create();
void EntityPlayer_add(Entity* entity);
void EntityPlayer_update(Entity* entity);
void EntityPlayer_draw(Entity* entity, Viewport* viewport);
void EntityPlayer_destroy(Entity* entity);
void EntityPlayer_collide(Entity* entity, Entity* entity_other);
Direction EntityPlayer_viewport_get_direction(Entity* entity);
void* EntityPlayer_message(Entity* entity, Entity* sender, EntityMessageType message_type, void* payload);

/* saving and loading */
bool EntityPlayer_serialize(Entity* entity, char* output);
Entity* EntityPlayer_deserialize(char* input);


#endif
