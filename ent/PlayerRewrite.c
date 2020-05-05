//
// Created by fabian on 05.05.20.
//

#include "PlayerRewrite.h"

#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../Util.h"
#include "../Graphics.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "../Log.h"


/* world behavior */
#define GRAVITY             0.2f
#define JUMP_INITIAL_CHARGE 7
#define JUMP_INITIAL_VSPEED -3.5f
#define WALK_HACCEL         0.3f
#define WALK_MAX_HSPEED     2.5f
#define WALK_BRAKE_SPEED    0.35f
#define CLIMB_HSPEED        0.7f
#define CLIMB_VSPEED        0.7f
#define GRAVITY_MAX_VSPEED  4.5f

/* graphics behavior */
#define TEX_WALK_FRAME_COUNT 6
#define TEX_FRAME_WIDTH      16
#define TEX_FRAME_HEIGHT     20

#define TEX_FRAME_ROW_OFFSET_WALKING 0
#define TEX_FRAME_COL_OFFSET_WALKING 0
#define TEX_FRAME_COUNT_WALKING      6

#define TEX_FRAME_ROW_OFFSET_JUMPING 2
#define TEX_FRAME_COL_OFFSET_JUMPING 0

#define TEX_FRAME_ROW_OFFSET_FALLING 0
#define TEX_FRAME_COL_OFFSET_FALLING 3


#define STATE_EDGE(PREV, CUR) (data->previous_state == PREV && data->state == CUR)
#define SWITCH_STATES(NEXT) data->state = NEXT; return;


typedef EntityPlayerData Data;
typedef EntityPlayerState State;
typedef EntityPlayerFacing Facing;

static SDL_Texture* tex_player;


void handle_state_edges(Entity*);
void handle_state_walking(Entity*);
void handle_state_jumping(Entity*);
void handle_state_falling(Entity*);
void handle_state_sledding(Entity*); /* TODO: implement Entity_resize with anchor. */
void handle_state_wall_sliding(Entity*);
void handle_state_climbing(Entity*);

void draw_state_walking(Entity*, Viewport*);
void draw_state_jumping(Entity*, Viewport*);
void draw_state_falling(Entity*, Viewport*);
void draw_state_sledding(Entity*, Viewport*);
void draw_state_wall_sliding(Entity*, Viewport*);
void draw_state_climbing(Entity*, Viewport*);

void         apply_gravity(Entity*);
CollidedWith move_freely(Entity* entity);


Entity* EntityPlayer_create(void)
{
	Log("EntityPlayer_create", "created.");

	if (!tex_player) {
		tex_player = SDLHelper_load_texture("assets/gfx/player.bmp");
	}

	Entity* entity = Entity_create();
	if (!entity) {
		Log_error("EntityPlayer_create", "failed to create entity.");
		return NULL;
	}

	Data* data = calloc(1, sizeof(Data));
	if (!data) {
		Log_error("EntityPlayer_create", "failed to allocate memory for entity data.");
		free(entity);
		return NULL;
	}

	entity->data = data;
	entity->rect = (Rectangle) {
		0, 0,
		16, 20
	};

	/* assign initial values to members of data. */
	data->state = data->previous_state = EPS_FALLING;
	data->velocity = (Vector2D) { .0f, .0f };
	data->facing = EPF_RIGHT;
	data->starting_pos = entity->rect.position;

	data->animation_frame = 0;
	data->animation_frame_satisfied = .0f;

	data->key_up_pressed_prev = false;

	Log("EntityPlayer_create", "created.");
	return entity;
}


void EntityPlayer_destroy(Entity* entity)
{
	Log("EntityPlayer_destroy", "destroying.");

	ENTITY_DATA(Player);
	if (data) {
		free(data);
	}
	free(entity);

	Log("EntityPlayer_destroy", "destroyed.");
}


bool EntityPlayer_serialize(Entity* entity, char* output)
{
	return false;
}


Entity* EntityPlayer_deserialize(char* input)
{
	Log("EntityPlayer_deserialize", "deserializing.");

	Entity* entity = EntityPlayer_create();
	if (!entity) {
		Log_error("EntityPlayer_deserialize", "failed to deserialize.");
	}

	Log("EntityPlayer_deserialize", "deserialized.");
	return entity;
}


void EntityPlayer_add(Entity* entity)
{

}


void EntityPlayer_collide(Entity* entity, Entity* entity_other)
{

}


void EntityPlayer_update(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	switch (data->state) {
	case EPS_WALKING:
		handle_state_walking(entity);
		break;
	case EPS_JUMPING:
		handle_state_jumping(entity);
		break;
	case EPS_FALLING:
		handle_state_falling(entity);
		break;
	default:;
	}

	handle_state_edges(entity);
}


void handle_state_edges(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);
	if (data->previous_state == data->state) {
		return;
	}

	/* probably not even necessary now that i think of it.
	 */


#if 0
	// handle state edges.
	if (STATE_EDGE(EPS_WALKING, EPS_JUMPING)) {
		// do nothing.
	} else if (STATE_EDGE(EPS_WALKING, EPS_FALLING)) {
		// do nothing.
	} else if (STATE_EDGE(EPS_JUMPING, EPS_WALKING)) {
		// do nothing.
	} else if (STATE_EDGE(EPS_FALLING, EPS_WALKING)) {
		// do nothing.
	}
#endif


	data->previous_state = data->state;
}


void handle_state_walking(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	CollidedWith collided_with = move_freely(entity);



	if (!(collided_with & CW_TOP)) {
		SWITCH_STATES(EPS_FALLING);
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	bool key_up = keys[SDL_SCANCODE_UP] != 0;
	if (key_up) {
		/* switch state to jumping. */

		data->velocity.y = JUMP_INITIAL_VSPEED;
		data->jump_charge_counter = 0;
		SWITCH_STATES(EPS_JUMPING);
	}
}


void handle_state_jumping(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	/* move_freely may reset velocity, so store previous speed
	 * for bouncing.
	 */
	float previous_y_speed = data->velocity.y;
	CollidedWith collided_with = move_freely(entity);
	if (collided_with & CW_BOTTOM) {
		/* switch state to falling. */

		data->velocity.y = -0.5f * previous_y_speed;
		SWITCH_STATES(EPS_FALLING);
	}
}


void handle_state_falling(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	CollidedWith collided_with = move_freely(entity);
	if (collided_with & CW_TOP) {
		SWITCH_STATES(EPS_WALKING);
	}
}


void handle_state_sledding(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);
}


void handle_state_wall_sliding(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);
}


void handle_state_climbing(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);
}


void EntityPlayer_draw(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	switch (data->state) {
	case EPS_WALKING:
		draw_state_walking(entity, viewport);
		break;
	case EPS_JUMPING:
		draw_state_jumping(entity, viewport);
		break;
	case EPS_FALLING:
		draw_state_falling(entity, viewport);
		break;
	}
}


void draw_state_walking(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	if (data->animation_frame_satisfied >= 1.0) {
		data->animation_frame_satisfied = 0;
		data->animation_frame = 0;
	}

	int frame_row = TEX_FRAME_ROW_OFFSET_WALKING + (data->facing == EPF_LEFT);
	int frame_col = TEX_FRAME_COL_OFFSET_WALKING + data->animation_frame;

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TEX_FRAME_WIDTH,
					.y = frame_row * TEX_FRAME_HEIGHT,
			},
			.size = entity->rect.size
	};

	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}


void draw_state_jumping(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	int frame_row = TEX_FRAME_ROW_OFFSET_JUMPING;
	int frame_col = TEX_FRAME_COL_OFFSET_JUMPING + (data->facing == EPF_LEFT);

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TEX_FRAME_WIDTH,
					.y = frame_row * TEX_FRAME_HEIGHT
			},
			.size = entity->rect.size
	};
	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}


void draw_state_falling(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	int frame_row = TEX_FRAME_ROW_OFFSET_FALLING + (data->facing == EPF_LEFT);
	int frame_col = TEX_FRAME_COL_OFFSET_FALLING;

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TEX_FRAME_WIDTH,
					.y = frame_row * TEX_FRAME_HEIGHT
			},
			.size = entity->rect.size;
	};
	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}


/* applies gravity to the entity's velocity.
 * does not move them.
 */
void apply_gravity(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	/* player can fall faster than gravity if accelerated downwards by other factors. */
	if (data->velocity.y < GRAVITY_MAX_VSPEED) {
		data->velocity.y = min(GRAVITY_MAX_VSPEED, data->velocity.y + GRAVITY);
	}
}


/* lets the player move around the map using the keyboard,
 * stops them when they run into an obstacle,
 * and returns the face of the obstacle that was hit.
 */
CollidedWith move_freely(Entity* entity)
{
	/* for states: walking, jumping, and falling */
	ENTITY_DATA(Player);
	if (!data) {
		Log_error("EntityPlayer", "move_freely: data == NULL");
		return CW_NOTHING;
	}


	const Uint8* keys = SDL_GetKeyboardState(NULL);

	bool keys_up =    keys[SDL_SCANCODE_UP] != 0;
	bool keys_left =  keys[SDL_SCANCODE_LEFT] != 0;
	bool keys_right = keys[SDL_SCANCODE_RIGHT] != 0;

	if (keys_left) {
		data->velocity.x = max(-WALK_MAX_HSPEED, data->velocity.x - WALK_HACCEL);
		if (data->velocity.x < 0) {
			data->facing = EPF_LEFT;
		}
	} else if (keys_right) {
		data->velocity.x = min(WALK_MAX_HSPEED, data->velocity.x + WALK_HACCEL);
		if (data->velocity.x > 0) {
			data->facing = EPF_RIGHT;
		}
	}
	if (!(keys_left || keys_right)) {
		/* come to a halt. */
		float previous_velocity_x = data->velocity.x;
		if (data->velocity.x > 0) {
			data->velocity.x = max(0, data->velocity.x - WALK_BRAKE_SPEED);
		} else if (data->velocity.x < 0) {
			data->velocity.x = min(0, data->velocity.x + WALK_BRAKE_SPEED);
		}

		/* snap to whole number position (so you can go down single-cell-width holes. */
		if (data->velocity.x == 0 && (data->velocity.x != previous_velocity_x)) {
			float delta_x_rounded = (float)round(entity->rect.position.x) - entity->rect.position.x;
			Vector2D delta_pos = { delta_x_rounded, 0 };

			World_move(entity->world, entity, &delta_pos);
		}
	}

	apply_gravity(entity);

	CollidedWith collided_with = World_move(entity->world, entity, &(data->velocity));
	if (collided_with & (CW_LEFT | CW_RIGHT)) {
		data->velocity.x = 0;
	}
	if (collided_with & (CW_TOP | CW_BOTTOM)) {
		data->velocity.y = 0;
	}

	return collided_with;
}