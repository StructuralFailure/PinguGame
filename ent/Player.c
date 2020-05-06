//
// Created by fabian on 05.05.20.
//

#include "Player.h"

#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../Util.h"
#include "../Graphics.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "../Log.h"
#include "../IO.h"


/* world behavior */
#define DEFAULT_ENTITY_SIZE        (Vector2D) { 16, 21 }

#define GRAVITY                    0.3f
#define GRAVITY_MAX_VSPEED         4.5f

#define WALL_SLIDE_MAX_VSPEED      2.0f

#define WALK_HACCEL                0.3f
#define WALK_MAX_HSPEED            2.5f
#define WALK_BRAKE_SPEED           0.2f
#define WALK_SLED_MIN_SPEED        1.0f
#define WALK_SLED_BOOST_FACTOR     1.3f

#define FALL_ENTER_SLIDE_VSPEED    0.5f
#define FALL_BRAKE_SPEED           WALK_BRAKE_SPEED / 5

#define JUMP_MAX_CHARGE            12
#define JUMP_INITIAL_VSPEED       -3.5f
#define JUMP_BOUNCE_YSPEED_FACTOR  0.5f
#define JUMP_BRAKE_SPEED           FALL_BRAKE_SPEED

#define EJECT_VSPEED              -2.3f
#define EJECT_HSPEED               2.0f
#define EJECT_DURATION             10

#define SLED_ENTITY_SIZE           (Vector2D) { 25, 16 }
#define SLED_GROUND_BRAKE_FACTOR   0.985f
#define SLED_GET_UP_SPEED          0.7f
#define SLED_JUMP_TIMEOUT          10
#define SLED_JUMP_INITIAL_VSPEED   JUMP_INITIAL_VSPEED * 0.5

#define CLIMB_HSPEED               0.7f
#define CLIMB_VSPEED               0.7f

/* graphics behavior */
#define WALKING_ANIMATION_FRAME_THROTTLE 7.5f

#define TF_WIDTH      25
#define TF_HEIGHT     21

#define TF_ROW_OFFSET_WALKING  0
#define TF_COL_OFFSET_WALKING  0
#define TF_COUNT_WALKING       6

#define TF_ROW_OFFSET_JUMPING  2
#define TF_COL_OFFSET_JUMPING  0

#define TF_ROW_OFFSET_FALLING  0
#define TF_COL_OFFSET_FALLING  3

#define TF_ROW_OFFSET_SLEDDING 2
#define TF_COL_OFFSET_SLEDDING 2

#define STATE_EDGE(PREV, CUR) (data->previous_state == PREV && data->state == CUR)
#define SWITCH_STATES(NEXT) { data->state = NEXT; return; }


typedef EntityPlayerData Data;
typedef EntityPlayerState State;
typedef EntityPlayerFacing Facing;

static SDL_Texture* tex_player;


static void handle_state_edges(Entity*);
static void handle_state_walking(Entity*);
static void handle_state_jumping(Entity*);
static void handle_state_falling(Entity*);
static void handle_state_sledding(Entity*); /* TODO: implement Entity_resize with anchor. */
static void handle_state_wall_sliding(Entity*);
static void handle_state_ejecting(Entity*);
static void handle_state_climbing(Entity*);

static void draw_state_walking(Entity*, Viewport*);
static void draw_state_jumping(Entity*, Viewport*);
static void draw_state_falling(Entity*, Viewport*);
static void draw_state_sledding(Entity*, Viewport*);
static void draw_state_wall_sliding(Entity*, Viewport*);
static void draw_state_climbing(Entity*, Viewport*);
static void draw_state_ejecting(Entity* entity, Viewport* viewport);

static void         apply_gravity(Entity*);
static CollidedWith move_freely(Entity* entity, float hbrake_speed);
static void         draw_frame(Entity* entity, Viewport* viewport, SDL_Texture* texture, Vector2DInt* frame_index);
static bool         overlaps_ladder(Entity* entity);
static bool         resize_sled_to_default(Entity* entity);


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

	ENTITY_EVENT(Player, add);
	ENTITY_EVENT(Player, update);
	ENTITY_EVENT(Player, draw);
	ENTITY_EVENT(Player, destroy);
	ENTITY_EVENT(Player, collide);
	ENTITY_EVENT(Player, get_direction);
	ENTITY_EVENT(Player, message);

	entity->type = ET_PLAYER;
	entity->data = data;
	entity->rect = (Rectangle) {
		.position = { 16, 0 },
		.size = DEFAULT_ENTITY_SIZE
	};

	/* assign initial values to members of data. */
	data->state = data->previous_state = EPS_FALLING;
	data->velocity = (Vector2D) { .0f, .0f };
	data->facing = EPF_RIGHT;
	data->starting_pos = entity->rect.position;

	data->jump_charge_counter = 0;

	data->animation_frame = 0;
	data->animation_frame_satisfied = .0f;

	Log("EntityPlayer_create", "created.");
	return entity;
}


void EntityPlayer_destroy(Entity* entity)
{
	Log("EntityPlayer_destroy", "destroying.");

	entity->world->viewport->locked_onto = NULL;
	free(entity->data);
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

	EntityType type;

	if (sscanf(input, "%d %f %f", &type, &(entity->rect.position.x), &(entity->rect.position.y)) != 3) {
		Log_error("EntityPlayer", "deserialize: invalid argument count");
		EntityPlayer_destroy(entity);
		return NULL;
	}

	Log("EntityPlayer_deserialize", "deserialized.");
	return entity;
}


void EntityPlayer_add(Entity* entity)
{
	entity->world->viewport->locked_onto = entity;
}


void EntityPlayer_collide(Entity* entity, Entity* entity_other)
{

}


void EntityPlayer_update(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	static void (*handle_state_functions[__EPS_COUNT])(Entity*) = {
			[EPS_WALKING]      = handle_state_walking,
			[EPS_JUMPING]      = handle_state_jumping,
			[EPS_FALLING]      = handle_state_falling,
			[EPS_WALL_SLIDING] = handle_state_wall_sliding,
			[EPS_EJECTING]     = handle_state_ejecting,
			[EPS_CLIMBING]     = handle_state_climbing,
			[EPS_SLEDDING]     = handle_state_sledding
	};

	void (*handle_state)(Entity*) = handle_state_functions[data->state];
	if (!handle_state) {
		Log_error("EntityPlayer_draw", "invalid state.");
		return;
	}

	handle_state(entity);
	handle_state_edges(entity);
}


Direction EntityPlayer_get_direction(Entity* entity)
{
	ENTITY_DATA(Player);
	if (!data) {
		Log_error("EntityPlayer_viewport_get_direction", "data == NULL");
		return 0;
	}

	Direction direction = DIR_UP | DIR_DOWN;
	if (data->facing == EPF_LEFT) {
		return direction |= DIR_LEFT;
	} else {
		return direction |= DIR_RIGHT;
	}
}


void* EntityPlayer_message(Entity* entity, Entity* sender, EntityMessageType message_type, void* payload)
{
	ENTITY_DATA(Player);
	if (!data) {
		Log_error("EntityPlayer_message", "data == NULL");
	}

	if (message_type == EMT_I_DAMAGED_YOU) {
		entity->rect.position = data->starting_pos;
	} else if (sender->type == ET_ENEMY && message_type == EMT_YOU_DAMAGED_ME) {
		data->velocity.y *= -0.8f;
	}

	return NULL;
}


static void handle_state_edges(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);
	if (data->previous_state == data->state) {
		return;
	}

	if (data->state == EPS_WALKING) {
		data->animation_frame = 0;
		data->animation_frame_satisfied = .0f;
	} else if (data->state == EPS_SLEDDING) {
		data->sled_jump_timeout = SLED_JUMP_TIMEOUT;
	}



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


static void handle_state_walking(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	data->animation_frame_satisfied += abs_float(data->velocity.x) / WALKING_ANIMATION_FRAME_THROTTLE;

	/* check whether to start sledding. */
	if (IO_key_pressed(IO_KEY_DOWN) && (abs_float(data->velocity.x) >= WALK_SLED_MIN_SPEED)) {
		/* resize entity, see if he fits, if not, slide. */
		Rectangle walking_rect = entity->rect;
		entity->rect.size = SLED_ENTITY_SIZE;

		float delta_x;
		if (data->facing == EPF_LEFT) {
			delta_x = -(entity->rect.size.x - walking_rect.size.x);
		} else /* facing right */ {
			delta_x = 0;
		}
		entity->rect.position.x += delta_x;
		entity->rect.position.y += (walking_rect.size.y - entity->rect.size.y);

		CollidedWith collided_with = World_move_until_collision(
			entity->world, &(entity->rect), &(Vector2D) { 0, 0 } /* don't actually move, just test if stuck. */
		);
		if (collided_with == CW_STUCK) {
			/* oops, got stuck in a wall. keep walking. */
			entity->rect = walking_rect;
		} else {
			data->velocity.x *= WALK_SLED_BOOST_FACTOR;
			SWITCH_STATES(EPS_SLEDDING);
		}
	}

	/* check whether to enter climbing state or jump. */
	if (IO_key_pressed(IO_KEY_UP)) {
		if (overlaps_ladder(entity)) {
			data->velocity = (Vector2D) { 0, 0 };
			SWITCH_STATES(EPS_CLIMBING);
		} else {
			/* no ladder, so jump. */
			data->jump_charge_counter = 0;
			data->velocity.y = JUMP_INITIAL_VSPEED;
			SWITCH_STATES(EPS_JUMPING);
		}
	}

	/* neither jump nor sled -> walk. */
	{
		CollidedWith collided_with = move_freely(entity, WALK_BRAKE_SPEED);
		if (!(collided_with & CW_TOP)) {
			SWITCH_STATES(EPS_FALLING);
		}
	}
}


static void handle_state_jumping(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	if (IO_key_down(IO_KEY_UP) && data->jump_charge_counter < JUMP_MAX_CHARGE) {
		data->velocity.y = JUMP_INITIAL_VSPEED;
		++data->jump_charge_counter;
	} else {
		data->jump_charge_counter = JUMP_MAX_CHARGE;
	}

	/* move_freely may reset velocity, so store previous speed
	 * for bouncing.
	 */
	float previous_y_speed = data->velocity.y;
	CollidedWith collided_with = move_freely(entity, JUMP_BRAKE_SPEED);

	/* don't brake as much when jumping. */

	if (data->velocity.y > 0) {
		SWITCH_STATES(EPS_FALLING);
	}

	if (collided_with & CW_BOTTOM) {
		/* switch state to falling. */

		data->velocity.y = -JUMP_BOUNCE_YSPEED_FACTOR * previous_y_speed;
		SWITCH_STATES(EPS_FALLING);
	} else if (collided_with & CW_TOP) {
		SWITCH_STATES(EPS_WALKING);
	}
}


static void handle_state_falling(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	CollidedWith collided_with = move_freely(entity, FALL_BRAKE_SPEED);
	if (collided_with & CW_TOP) {
		SWITCH_STATES(EPS_WALKING);
	}

	bool enter_wall_slide_state = false;

	if (collided_with & CW_RIGHT) {
		if (data->velocity.y >= FALL_ENTER_SLIDE_VSPEED
		 && IO_key_down(IO_KEY_LEFT)) {
			data->wall_slide_direction = DIR_LEFT;
			enter_wall_slide_state = true;
		}
	} else if (collided_with & CW_LEFT) {
		//Log("EntityPlayer", "handle_state_falling: ")
		if (data->velocity.y >= FALL_ENTER_SLIDE_VSPEED
		 && IO_key_down(IO_KEY_RIGHT)) {
			data->wall_slide_direction = DIR_RIGHT;
			enter_wall_slide_state = true;
		}
	}

	if (enter_wall_slide_state) {
		SWITCH_STATES(EPS_WALL_SLIDING);
	}
}


static void handle_state_sledding(Entity* entity) {
	ENTITY_DATA_ASSERT(Player);

	apply_gravity(entity);

	CollidedWith collided_with = World_move(entity->world, entity, &(data->velocity));
	if (collided_with & (CW_TOP | CW_BOTTOM)) {
		data->velocity.y = 0;
	}
	bool is_on_ground = collided_with & CW_TOP;
	bool crashed_into_wall = collided_with & (CW_LEFT | CW_RIGHT);

	if (is_on_ground) {
		data->velocity.x *= SLED_GROUND_BRAKE_FACTOR;
	}
	bool is_too_slow = abs_float(data->velocity.x) <= SLED_GET_UP_SPEED;

	/* check whether to get up. */
	if (is_too_slow || crashed_into_wall) {
		//Log("EntityPlayer", "handle_state_sledding: trying to get up.");
		data->velocity.x = .0f;
		if (resize_sled_to_default(entity)) {
			SWITCH_STATES(EPS_WALKING);
		}
	}

	/* check whether we can jump out of it. */
	if (IO_key_pressed(IO_KEY_UP)
	 && data->sled_jump_timeout <= 0
	 && resize_sled_to_default(entity)) {
		data->jump_charge_counter = JUMP_MAX_CHARGE;
		data->velocity.y = JUMP_INITIAL_VSPEED / 2.0f;
		SWITCH_STATES(EPS_JUMPING);
	}

	--data->sled_jump_timeout;
}


static void handle_state_wall_sliding(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	/* cap vertical speed. */
	data->velocity.y = min(WALL_SLIDE_MAX_VSPEED, data->velocity.y);

	CollidedWith collided_with = move_freely(entity, 0);
	if ((data->wall_slide_direction == DIR_LEFT  && !(collided_with & CW_RIGHT))
	 || (data->wall_slide_direction == DIR_RIGHT && !(collided_with & CW_LEFT))) {
		SWITCH_STATES(EPS_FALLING);
	}

	if (collided_with & CW_TOP) {
		SWITCH_STATES(EPS_WALKING);
	}

	if (IO_key_pressed(IO_KEY_UP)) {
		data->eject_counter = 0;
		data->eject_velocity.y = EJECT_VSPEED;
		if (data->wall_slide_direction == DIR_LEFT) {
			data->eject_velocity.x = EJECT_HSPEED;
			data->facing = EPF_RIGHT;
		} else {
			data->eject_velocity.x = -EJECT_HSPEED;
			data->facing = EPF_LEFT;
		}
		SWITCH_STATES(EPS_EJECTING);
	}
}


static void handle_state_climbing(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	Vector2D delta_pos = {
			.x = CLIMB_HSPEED * (float)(IO_key_down(IO_KEY_RIGHT) - IO_key_down(IO_KEY_LEFT)),
			.y = CLIMB_VSPEED * (float)(IO_key_down(IO_KEY_DOWN) - IO_key_down(IO_KEY_UP))
	};

	CollidedWith collided_with = World_move(entity->world, entity, &delta_pos);
	if (collided_with & CW_TOP || !overlaps_ladder(entity)) {
		SWITCH_STATES(EPS_WALKING);
	}
}


static void handle_state_ejecting(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	data->velocity = data->eject_velocity;
	CollidedWith collided_with = World_move(entity->world, entity, &(data->eject_velocity));
	if (collided_with) {
		SWITCH_STATES(EPS_FALLING);
	}

	++data->eject_counter;
	if (data->eject_counter > EJECT_DURATION) {
		data->eject_counter = 0;
		SWITCH_STATES(EPS_JUMPING);
	}
}


void EntityPlayer_draw(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	static void (*draw_state_functions[__EPS_COUNT])(Entity*, Viewport*) = {
		[EPS_WALKING]      = draw_state_walking,
		[EPS_JUMPING]      = draw_state_jumping,
		[EPS_FALLING]      = draw_state_falling,
		[EPS_WALL_SLIDING] = draw_state_wall_sliding,
		[EPS_EJECTING]     = draw_state_ejecting,
		[EPS_CLIMBING]     = draw_state_climbing,
		[EPS_SLEDDING]     = draw_state_sledding
	};

	void (*draw_state)(Entity*, Viewport*) = draw_state_functions[data->state];
	if (!draw_state) {
		Log_error("EntityPlayer_draw", "invalid state.");
		return;
	}
	draw_state(entity, viewport);
}


static void draw_state_walking(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	if (data->animation_frame_satisfied >= 1.0) {
		data->animation_frame_satisfied = 0;
		data->animation_frame = (data->animation_frame + 1) % TF_COUNT_WALKING;
	}

	int frame_row = TF_ROW_OFFSET_WALKING + (data->facing == EPF_LEFT);
	int frame_col = TF_COL_OFFSET_WALKING + data->animation_frame;

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TF_WIDTH,
					.y = frame_row * TF_HEIGHT,
			},
			.size = entity->rect.size
	};

	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}


static void draw_state_wall_sliding(Entity* entity, Viewport* viewport)
{
	draw_state_walking(entity, viewport);
}


static void draw_state_ejecting(Entity* entity, Viewport* viewport)
{
	draw_state_jumping(entity, viewport);
}


static void draw_state_jumping(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	int frame_row = TF_ROW_OFFSET_JUMPING;
	int frame_col = TF_COL_OFFSET_JUMPING + (data->facing == EPF_LEFT);

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TF_WIDTH,
					.y = frame_row * TF_HEIGHT
			},
			.size = entity->rect.size
	};
	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}


static void draw_state_falling(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	int frame_row = TF_ROW_OFFSET_FALLING + (data->facing == EPF_LEFT);
	int frame_col = TF_COL_OFFSET_FALLING;

	Rectangle tex_rect = {
			.position = {
					.x = (float)frame_col * TF_WIDTH,
					.y = (float)frame_row * TF_HEIGHT
			},
			.size = entity->rect.size
	};
	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);
}

static void draw_state_climbing(Entity* entity, Viewport* viewport)
{
	draw_frame(entity, viewport, tex_player, &(Vector2DInt) { 0, 0 });
}


static void draw_state_sledding(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Player);

	draw_frame(
		entity, viewport, tex_player,
		&(Vector2DInt) {
			.x = TF_COL_OFFSET_SLEDDING + (data->facing == EPF_LEFT),
			.y = TF_ROW_OFFSET_SLEDDING
		}
	);
}


static bool overlaps_ladder(Entity* entity)
{
	RectangleInt overlapping_cells = World_get_overlapping_cells(entity->world, &(entity->rect));
	bool overlaps_ladder = false;
	for (int dy = 0; dy < overlapping_cells.size.y; ++dy) {
		for (int dx = 0; dx < overlapping_cells.size.x; ++dx) {
			Vector2DInt current_cell = { overlapping_cells.position.x + dx, overlapping_cells.position.y + dy };
			if (Level_get_cell_type_properties(entity->world->level, current_cell.x, current_cell.y)->type == LCT_LADDER) {
				return true;
			}
		}
	}
	return false;
}


/* applies gravity to the entity's velocity.
 * does not move them.
 */
static void apply_gravity(Entity* entity)
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
 *
 * hbrake specifies by how much horizontal movement should diminish when no player input is given.
 */
static CollidedWith move_freely(Entity* entity, float hbrake)
{
	/* for states: walking, jumping, and falling */
	ENTITY_DATA(Player);
	if (!data) {
		Log_error("EntityPlayer", "move_freely: data == NULL");
		return CW_NOTHING;
	}

	bool keys_left  = IO_key_down(IO_KEY_LEFT);
	bool keys_right = IO_key_down(IO_KEY_RIGHT);

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
			data->velocity.x = max(0, data->velocity.x - hbrake);
		} else if (data->velocity.x < 0) {
			data->velocity.x = min(0, data->velocity.x + hbrake);
		}

		/* snap to whole number position (so you can go down single-cell-width holes). */
		if (data->velocity.x == 0 && (data->velocity.x != previous_velocity_x)) {
			float delta_x_rounded = (float)round(entity->rect.position.x) - entity->rect.position.x;
			Vector2D delta_pos = { delta_x_rounded, 0 };

			World_move(entity->world, entity, &delta_pos);
			data->animation_frame = 0;
			data->animation_frame_satisfied = .0f;
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


static void draw_frame(Entity* entity, Viewport* viewport, SDL_Texture* texture, Vector2DInt* frame_index)
{
	Rectangle tex_rect = {
			.position = {
					.x = TF_WIDTH  * (float)(frame_index->x),
					.y = TF_HEIGHT * (float)(frame_index->y)
			},
			.size = entity->rect.size
	};

	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), texture);
}


static bool resize_sled_to_default(Entity* entity)
{
	ENTITY_DATA(Player);
	if (!data) {
		Log_error("EntityPlayer", "resize_sled_to_default: data == NULL");
		return false;
	}

	Rectangle sledding_rect = entity->rect;
	entity->rect.size = DEFAULT_ENTITY_SIZE;
	entity->rect.position.y -= (entity->rect.size.y - sledding_rect.size.y);

	if (data->facing == EPF_LEFT) {
		entity->rect.position.x += (sledding_rect.size.x - entity->rect.size.x);
	}

	if (World_move_until_collision(entity->world, &(entity->rect), &(Vector2D) { 0, 0 }) == CW_STUCK) {
		entity->rect = sledding_rect;
		return false;
	}
	return true;
}