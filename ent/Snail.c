//
// Created by fabian on 29.04.20.
//

#include "Log.h"
#include "Util.h"
#include "Snail.h"

#define SPEED  0.6f
#define WIDTH  20
#define HEIGHT 16

#define TEX_FRAME_WIDTH  20
#define TEX_FRAME_HEIGHT 20
#define ENTITY_DETECTION_MARGIN -3

#define SHELL_WIDTH  13
#define SHELL_HEIGHT 15

#define GRAVITY 0.2f
#define MAX_FALLING_SPEED 3.0f
#define KICKING_COOLDOWN 20

typedef EntitySnailStickingDirection StickingDirection;
typedef EntitySnailState State;


static SDL_Texture* tex_snail;


void handle_state_crawling(Entity* entity);
void handle_state_falling(Entity* entity);
void handle_state_kickable(Entity* entity);
void handle_state_edges(Entity* entity);

void handle_collision_kickable(Entity* entity, Entity* entity_other);

StickingDirection mirror_sticking_direction(StickingDirection input);
bool has_line_of_sight(Entity* entity);

void do_inner_turn(Entity* entity);
void do_outer_turn(Entity* entity);
void reverse_crawling_direction(Entity* entity);


Entity* EntitySnail_create(void)
{
	Log("EntitySnail_create", "creating.");

	if (!tex_snail) {
		tex_snail = SDLHelper_load_texture("assets/gfx/snail.bmp");
	}

	Entity* snail = Entity_create();
	if (!snail) {
		Log_error("EntitySnail_create", "failed to create base entity.");
		return NULL;
	}

	EntitySnailData* data = calloc(1, sizeof(EntitySnailData));
	if (!data) {
		Log_error("EntitySnail_create", "failed to allocate memory for entity data.");
		free(snail);
		return NULL;
	}

	snail->type = ET_SNAIL;
	snail->add = EntitySnail_add;
	snail->update = EntitySnail_update;
	snail->draw = EntitySnail_draw;
	snail->destroy = EntitySnail_destroy;
	snail->message = EntitySnail_message;
	snail->collide = EntitySnail_collide;
	snail->added_other_entity = EntitySnail_added_other_entity;
	snail->removing_other_entity = EntitySnail_removing_other_entity;

	snail->rect = (Rectangle) {
		.position = { 0, 0 },
		.size = { 20, 16 }
	};

	data->sticking = ESSD_UPWARDS;
	data->crawling_clockwise = false;
	data->state = data->previous_state = ESS_CRAWLING; /* start off kickable for testing purposes. */
	data->shell_velocity = (Vector2D) { 0, 0 };
	data->falling_speed = 0;
	data->kicking_cooldown = 0;
	snail->data = data;

	Log("EntitySnail_create", "created.");
	return snail;
}


void EntitySnail_added_other_entity(Entity* entity, Entity* entity_other)
{
	ENTITY_DATA_ASSERT(Snail);
	if (data->locked_onto) {
		return;
	}

	if (entity_other->type == ET_PLAYER) {
		data->locked_onto = entity_other;
		Log("EntitySnail_added_other_entity", "found player.");
	}
}


void EntitySnail_removing_other_entity(Entity* entity, Entity* entity_other)
{
	ENTITY_DATA_ASSERT(Snail);
	if (!data->locked_onto) {
		return;
	}

	if (entity_other == data->locked_onto) {
		data->locked_onto = NULL;
	}
}


void EntitySnail_collide(Entity* entity, Entity* entity_other)
{
	/* when a player hits a shell, he should be able to bounce it around. */
	ENTITY_DATA_ASSERT(Snail);

	if (entity_other->type != ET_PLAYER) {
		return;
	}

	if (data->state == ESS_KICKABLE) {
		handle_collision_kickable(entity, entity_other);
	}

	/* 2:28:00
	 * 2:47:46
	 */
}


void handle_collision_kickable(Entity* entity, Entity* player)
{
	ENTITY_DATA_ASSERT(Snail);

	if (data->kicking_cooldown > 0) {
		return;
	}

	Entity* shell = entity;

	Circle shell_circle = {
			.radius = shell->rect.size.x / 2,
			.origin = Rectangle_center(&(shell->rect))
	};

	Circle player_circle = {
			.radius = player->rect.size.x / 2,
			.origin = Rectangle_center(&(player->rect))
	};


	/* collide event works based on rectangle, but we only want to act when there is a
 	 * collision of the circles enclosed by the rectangles.
 	 */
	if (!Circle_overlap(&shell_circle, &player_circle)) {
		return;
	}

	/* we've established that the circles are overlapping, so we use
	 * a do loop to start moving back until there is no
	 * overlap any longer.
	 */
	Vector2D player_prev_center = Rectangle_center(&(player->previous_rect));
	Vector2D shell_prev_center = Rectangle_center(&(shell->previous_rect));

	Vector2D player_delta_pos = Vector2D_difference(player_circle.origin, player_prev_center);

	Vector2D shell_delta_pos = Vector2D_difference(shell_circle.origin, shell_prev_center);
	Vector2D shell_delta_pos_trans = Vector2D_difference(shell_delta_pos, player_delta_pos);
	Vector2D shell_delta_pos_trans_unit_rev = Vector2D_create_with_length(shell_delta_pos_trans, -1);

	/* TODO: change the way we move the circle out to be more efficient.
	 *       there is a formula for it, but i'm too lazy to look it up
	 *       or figure it out myself. just using the rectangle method for now.
	 */
	float dist_moved_back = 0;
	float shell_delta_pos_length = Vector2D_length(shell_delta_pos);
	do {
		World_move_until_collision(entity->world, &(entity->rect), &shell_delta_pos_trans_unit_rev);

		/*entity->rect.position.x -= shell_delta_pos_trans_unit_rev.x;
		entity->rect.position.y -= shell_delta_pos_trans_unit_rev.y;

		/* moving a rectangle makes its center move by the same amount. */
		/*shell_circle.origin.x -= shell_delta_pos_trans_unit.x;
		shell_circle.origin.y -= shell_delta_pos_trans_unit.y;*/

		shell_circle.origin = Rectangle_center(&(entity->rect));
		dist_moved_back += 1.0f;
	} while ((dist_moved_back < shell_delta_pos_length) &&
	         Circle_overlap(&shell_circle, &player_circle));

	/* now that we've moved  back outside, we can establish the angle of
	 * the reflector vector that we'll use to bounce off entity_other.
	 */
	Vector2D reflector_normal_unit = Vector2D_create_with_length(
		Vector2D_difference(shell_circle.origin, player_circle.origin),
		1
	);

	/* assume the player is stationary. transfer player's velocity in the opposite direction to achieve this. */


	/* reflected→ = { [ -2 * ( reflector_normal→ ⋅ ent_dp→ ) ] * reflector_normal→ } + ent_dp→ */
	Vector2D reflected =
		Vector2D_sum(
			shell_delta_pos_trans,
			Vector2D_product(
				reflector_normal_unit,
			-2 * Vector2D_dot_product(
					reflector_normal_unit,
					shell_delta_pos_trans
				)
			)
		);

	data->shell_velocity = (Vector2D) { reflected.x * 1.2f, reflected.y - 2.0f };
	data->kicking_cooldown = KICKING_COOLDOWN;
}


void EntitySnail_update(Entity* entity)
{
	ENTITY_DATA_ASSERT(Snail);

	/* check player and perhaps start falling. */

	switch (data->state) {
	case ESS_CRAWLING:
		handle_state_crawling(entity);
		break;
	case ESS_FALLING:
		handle_state_falling(entity);
		break;
	case ESS_KICKABLE:
		handle_state_kickable(entity);
		break;
	default:
		;
	}

	/* handle state edges. */
	if (data->state != data->previous_state) {
		handle_state_edges(entity);
	}

	//Log("EntitySnail_update", "position = { x = %f | y = %f }", entity->rect.position.x, entity->rect.position.y);
}


void handle_state_edges(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	/* mainly adjusting entity->rect. */
	if ((data->previous_state == ESS_CRAWLING && data->state == ESS_FALLING)
	 || (data->previous_state == ESS_CRAWLING && data->state == ESS_KICKABLE)) {
		/* we're becoming smaller, so no need to check whether we fit. */
		if (data->crawling_clockwise) {
			/* no need to move. */
		} else {
			entity->rect.position.x += 7;
		}
		entity->rect.size = (Vector2D) { 13, 15 };
	} else if (data->previous_state == ESS_FALLING && data->state == ESS_CRAWLING) {
		/* TODO: check whether we fit before growing. */

		/* upon landing, snail should chase the player. */
		float lo_center_x = data->locked_onto->rect.position.x + data->locked_onto->rect.size.x / 2;
		float entity_center_x = entity->rect.position.x + entity->rect.size.x / 2;
		data->crawling_clockwise = lo_center_x < entity_center_x;

		if (data->crawling_clockwise) {
			entity->rect.position.x -= 7;
		} else {
			/* no need to move. */
		}
		entity->rect.size = (Vector2D) { 20, 16 };
		entity->rect.position.y -= 1;

		data->falling_speed = 0;
		data->sticking = ESSD_DOWNWARDS;
	}
	data->previous_state = data->state;
}


void handle_state_crawling(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	/* movement */
	Vector2D delta_pos = { 0 };
	switch (data->sticking) {
	case ESSD_RIGHTWARDS:
		delta_pos.y = +SPEED;
		break;
	case ESSD_DOWNWARDS:
		delta_pos.x = -SPEED;
		break;
	case ESSD_LEFTWARDS:
		delta_pos.y = -SPEED;
		break;
	case ESSD_UPWARDS:
		delta_pos.x = +SPEED;
		break;
	default:
		;
	}

	//Vector2D delta_pos = (Vector2D) { delta_pos_unit.x * SPEED, delta_pos_unit.y * SPEED };
	if (!data->crawling_clockwise) {
		delta_pos.x *= -1;
		delta_pos.y *= -1;
	}

	RectangleInt overlapping_cells = World_get_overlapping_cells(entity->world, &(entity->rect));
	Vector2DInt cell_to_check = { 0 };
	Vector2DInt cell_imaginary_wall = { 0 }; /* where the imaginary wall will be placed should cell_to_check not be solid. */

	/* checking whether to do outward turn. this is probably optimizable. */
	if (data->crawling_clockwise) {
		/* TODO: maybe add support for snails taller than a single cell. */
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x + 1,
					.y = overlapping_cells.position.y + overlapping_cells.size.y - 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y + overlapping_cells.size.y
			};
			break;
		case ESSD_UPWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x + overlapping_cells.size.x - 1,
					.y = overlapping_cells.position.y - 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x + overlapping_cells.size.x,
					.y = overlapping_cells.position.y
			};
			break;
		case ESSD_LEFTWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x - 1,
					.y = overlapping_cells.position.y
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y - 1
			};
			break;
		case ESSD_DOWNWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y + 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x - 1,
					.y = overlapping_cells.position.y
			};
			break;
		default:
			;
		}
	} else /* counter-clockwise */ {
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x + 1,
					.y = overlapping_cells.position.y
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y - 1
			};
			break;
		case ESSD_DOWNWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x + overlapping_cells.size.x - 1,
					.y = overlapping_cells.position.y + 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x + overlapping_cells.size.x,
					.y = overlapping_cells.position.y
			};
			break;
		case ESSD_LEFTWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x - 1,
					.y = overlapping_cells.position.y + overlapping_cells.size.y - 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y + overlapping_cells.size.y
			};
			break;
		case ESSD_UPWARDS:
			cell_to_check = (Vector2DInt) {
					.x = overlapping_cells.position.x,
					.y = overlapping_cells.position.y - 1
			};
			cell_imaginary_wall = (Vector2DInt) {
					.x = overlapping_cells.position.x - 1,
					.y = overlapping_cells.position.y
			};
			break;
		default:;
		}
	}
	LevelCellTypeFlags lct_flags = Level_get_cell_type_flags(entity->world->level, cell_to_check.x, cell_to_check.y);
	if (!(lct_flags & (LCTF_SOLID | LCTF_SEMISOLID))) {
		/* prefer outer turns:
		 * XXXXXXX    XXXXXXX   (. = empty space,
		 * ....S.. -> ....SS.    X = solid cell,
		 * XX..SXX    XX...XX    S = cell occupied by snail)
		 * XX...XX    XX...XX
		 */

		/* check collision with imaginary wall */
		Rectangle imaginary_wall = (Rectangle) {
			.position = {
				.x = cell_imaginary_wall.x * CM_CELL_WIDTH,
				.y = cell_imaginary_wall.y * CM_CELL_HEIGHT
			},
			.size = {
				.x = CM_CELL_WIDTH,
				.y = CM_CELL_HEIGHT
			}
		};

		CollidedWith collided_with = World_move_until_collision_with_flags(
			entity->world, &(entity->rect), &delta_pos, CC_RECTANGLE, &imaginary_wall
		);

		if (collided_with) {
			bool turn_around_on_collision = false;

			if (data->sticking == ESSD_DOWNWARDS) {
				/* crawling off the top of a semi-solid cell should result in turning around. */
				RectangleInt cells_below = World_get_overlapping_cells(entity->world, &(entity->rect));
				cells_below.position.y += 1;

				for (int dx = 0; dx < cells_below.size.x; ++dx) {
					if (Level_get_cell_type_flags(
							entity->world->level, cells_below.position.x + dx, cells_below.position.y
						) & LCTF_SEMISOLID) {
						turn_around_on_collision = true;
						break;
					}
				}
			}

			if (turn_around_on_collision) {
				data->crawling_clockwise = !data->crawling_clockwise;
				/*data->state = ESS_FALLING;
				Log("fnasi", "fall_on_collision");*/
			} else {
				do_outer_turn(entity);
			}
		}
	} else {
		/* no imaginary wall was hit -> no outer turn.
		 * continue movement and perhaps do inner turn.
		 */
		CollidedWith collided_with = World_move_until_collision(
				entity->world, &(entity->rect), &delta_pos
		);
		if (collided_with) {
			do_inner_turn(entity);
		}
	}

	if (has_line_of_sight(entity)) {
		Log("EntitySnail_update", "i see the player!");
		data->state = ESS_KICKABLE;
	}
}


void handle_state_falling(Entity* entity)
{
	ENTITY_DATA_ASSERT(Snail);

	data->falling_speed = min(MAX_FALLING_SPEED, data->falling_speed + GRAVITY);

	Vector2D delta_pos = {
		.x = 0,
		.y = data->falling_speed
	};
	CollidedWith collided_with = World_move_until_collision(entity->world, &(entity->rect), &delta_pos);

	if (collided_with) {
		/* go back to crawling */
		Log("handle_state_falling", "collided");
		data->state = ESS_CRAWLING;
	}
}


void handle_state_kickable(Entity* entity)
{
	ENTITY_DATA_ASSERT(Snail);

	if (data->kicking_cooldown > 0) {
		--data->kicking_cooldown;
	}

	/* do bouncing and stuff. */
	CollidedWith collided_with = World_move(entity->world, entity, &(data->shell_velocity));

	/* bounce off walls, but lose a bit of speed. */
	switch (collided_with) {
	case CW_TOP:
	case CW_BOTTOM:
		data->shell_velocity.y *= -0.5f;
		data->shell_velocity.x *= 0.8f;
		break;
	case CW_LEFT:
	case CW_RIGHT:
		data->shell_velocity.x *= -0.75f;
		break;
	default:;
	}
	data->shell_velocity.x *= 0.99f;
	data->shell_velocity.y = min(data->shell_velocity.y + GRAVITY, MAX_FALLING_SPEED);
}


void EntitySnail_draw(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(Snail);

	//SDL_Texture* tex = NULL;

	int frame_col = 0;
	int frame_row = 0;
	int animation_frame = 0;
	Rectangle tex_rect = {
			.size = entity->rect.size
	};

	if (data->state == ESS_CRAWLING) {
		bool flip_v = false;
		bool flip_h = false;
		bool use_vertical_tex = entity->rect.size.y > entity->rect.size.x;

		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			flip_h = true;
			if (!data->crawling_clockwise) {
				flip_v = true;
			}
			break;
		case ESSD_DOWNWARDS:
			if (data->crawling_clockwise) {
				flip_h = true;
			}
			break;
		case ESSD_LEFTWARDS:
			if (data->crawling_clockwise) {
				flip_v = true;
			}
			break;
		case ESSD_UPWARDS:
			flip_v = true;
			if (!data->crawling_clockwise) {
				flip_h = true;
			}
		default:
			;
		}

		frame_col = 3 * use_vertical_tex;
		frame_row = (2 * flip_v) + flip_h;
		animation_frame = 2 - (entity->world->ticks / 10) % 3; /* TODO: fix moonwalk. */
		frame_col += animation_frame;
	} else /* animation for falling and kickable */ {
		frame_col = 6;
		frame_row = !(data->crawling_clockwise);
	}

	tex_rect.position = (Vector2D) {
		.x = TEX_FRAME_WIDTH * frame_col,
		.y = TEX_FRAME_HEIGHT * frame_row
	};

	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_snail);
}


void* EntitySnail_message(Entity* entity, Entity* sender, EntityMessageType type, void* payload)
{
	return NULL;
}


void EntitySnail_add(Entity* entity)
{
	/* look for player to see if he's already been spawned.
	 * if not, locked_onto will be set by added_other_entity.
	 */
	ENTITY_DATA_ASSERT(Snail);

	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = entity->world->entities[i];
		if (ent && ent->type == ET_PLAYER) {
			data->locked_onto = ent;
			Log("EntitySnail_add", "found player.");
			break;
		}
	}
}


void EntitySnail_destroy(Entity* entity)
{
	Log("EntitySnail", "destroying.");

	ENTITY_DATA(Snail);
	if (data) {
		free(data);
	}
	free(entity);

	Log("EntitySnail", "destroyed.");
}


bool EntitySnail_serialize(Entity* entity, char* output)
{
	return false;
}


Entity* EntitySnail_deserialize(char* input)
{
	Entity* entity = EntitySnail_create();
	if (!entity) {
		Log_error("EntitySnail_deserialize", "failed to create entity.");
		return NULL;
	}

	EntitySnailData* data = (EntitySnailData*)(entity->data);

	int type;
	int position_x;
	int position_y;

	int argument_count;
	if ((argument_count = sscanf(input, "%d %d %d", &type, &position_x, &position_y)) != 3) {
		Log_error("EntitySnail_deserialize", "invalid argument count. expected 3, got %d.", argument_count);
		free(entity);
		return NULL;
	}

	entity->rect.position = (Vector2D) { position_x, position_y };

	//Log("EntitySnail_deserialize", "position = { %f | %f }", entity->rect.position.x, entity->rect.position.y);

	return entity;
}


void do_inner_turn(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	//Log("EntitySnail", "update_do_inner_turn: turning.");

	/* turn right if going clockwise, turn left otherwise. */
	Rectangle rect_old = entity->rect;
	StickingDirection sticking_old = data->sticking;
	float w_minus_h = abs_float(rect_old.size.x - rect_old.size.y);

	entity->rect.size = (Vector2D) {
		.x = rect_old.size.y,
		.y = rect_old.size.x
	};

	if (data->crawling_clockwise) {
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			data->sticking = ESSD_DOWNWARDS;
			entity->rect.position.x -= w_minus_h;
			entity->rect.position.y += w_minus_h;
			/* no positional adjustment required. */
			break;
		case ESSD_DOWNWARDS:
			data->sticking = ESSD_LEFTWARDS;
			entity->rect.position.y -= w_minus_h;
			/* move to the right a bit to be flush with right wall. */
			break;
		case ESSD_LEFTWARDS:
			data->sticking = ESSD_UPWARDS;
			break;
		case ESSD_UPWARDS:
			data->sticking = ESSD_RIGHTWARDS;
			entity->rect.position.x += w_minus_h;
			break;
		default:
				;
		}
	} else /* counter-clockwise */ {
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			data->sticking = ESSD_UPWARDS;
			entity->rect.position.x -= w_minus_h;
			break;
		case ESSD_UPWARDS:
			data->sticking = ESSD_LEFTWARDS;
			break;
		case ESSD_LEFTWARDS:
			data->sticking = ESSD_DOWNWARDS;
			entity->rect.position.y += w_minus_h;
			break;
		case ESSD_DOWNWARDS:
			data->sticking = ESSD_RIGHTWARDS;
			entity->rect.position.x += w_minus_h;
			entity->rect.position.y -= w_minus_h;
			break;
		default:
			;
		}
	}

	/* dummy movement to check whether we're stuck now that we turned.
	 * if we are, we can't turn, so start going back where we came from.
	 */
	Vector2D delta_pos = { 0, 0 };
	CollidedWith collided_with = World_move_until_collision(entity->world, &(entity->rect), &delta_pos);
	if (collided_with == CW_STUCK) {
		//Log("EntitySnail", "update_do_inner_turn: stuck.");
		entity->rect = rect_old;
		data->sticking = sticking_old;
		data->crawling_clockwise = !data->crawling_clockwise;
	}
}


void do_outer_turn(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	/* we can use do_inner_turn:
	 * 1. mirror snail's sticking direction and whether it's going clockwise.
	 * 2. do inward turn,
	 * 3. mirror snail's sticking direction and whether it's going clockwise again.
	 */

	data->sticking = mirror_sticking_direction(data->sticking);
	data->crawling_clockwise = !data->crawling_clockwise;
	do_inner_turn(entity);
	data->sticking = mirror_sticking_direction(data->sticking);
	data->crawling_clockwise = !data->crawling_clockwise;
}


void reverse_crawling_direction(Entity* entity)
{
	ENTITY_DATA(Snail);
	if (!data) {
		return;
	}

	int fact_horizontal_pixels = 0;
	int fact_vertical_pixels = 0;

	if (data->crawling_clockwise) {
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			fact_vertical_pixels = -1;
			break;
		case ESSD_DOWNWARDS:
			fact_horizontal_pixels = 1;
			break;
		case ESSD_LEFTWARDS:
			fact_vertical_pixels = 1;
			break;
		case ESSD_UPWARDS:
			fact_horizontal_pixels = -1;
		}
	} else /* counter-clockwise */ {
		switch (data->sticking) {
		case ESSD_RIGHTWARDS:
			fact_vertical_pixels = 1;
			break;
		case ESSD_DOWNWARDS:
			fact_horizontal_pixels = -1;
			break;
		case ESSD_LEFTWARDS:
			fact_vertical_pixels = -1;
			break;
		case ESSD_UPWARDS:
			fact_horizontal_pixels = 1;
		}
	}

	Vector2D delta_pos = { 7 * fact_horizontal_pixels, 7 * fact_vertical_pixels };
	World_move_until_collision(entity->world, &(entity->rect), &delta_pos);
}


StickingDirection mirror_sticking_direction(StickingDirection input)
{
	switch (input) {
	case ESSD_RIGHTWARDS:
		return ESSD_LEFTWARDS;
	case ESSD_DOWNWARDS:
		return ESSD_UPWARDS;
	case ESSD_LEFTWARDS:
		return ESSD_RIGHTWARDS;
	case ESSD_UPWARDS:
		return ESSD_DOWNWARDS;
	default:
		Log_error("EntitySnail", "cannot mirror sticking direction. invalid input.");
		return 0;
	}
}


bool has_line_of_sight(Entity* entity)
{
	ENTITY_DATA(Snail);
	if (!data || !data->locked_onto || data->sticking != ESSD_UPWARDS) {
		return false;
	}

	Rectangle* lo_rect = &(data->locked_onto->rect);
	if (entity->rect.position.y > lo_rect->position.y ||
		lo_rect->position.x - ENTITY_DETECTION_MARGIN > entity->rect.position.x + entity->rect.size.x ||
		lo_rect->position.x + lo_rect->size.x + ENTITY_DETECTION_MARGIN < entity->rect.position.x) {
		return false;
	}

	Rectangle sight_rect = {
			.position = {
					.x = entity->rect.position.x - ENTITY_DETECTION_MARGIN,
					.y = entity->rect.position.y
			},
			.size = {
					.x = entity->rect.size.x + 2 * ENTITY_DETECTION_MARGIN,
					.y = lo_rect->position.y - entity->rect.position.y
			}
	};

	RectangleInt overlapping_cells = World_get_overlapping_cells(entity->world, &sight_rect);

	bool obstructed = false;
	for (int dy = 0; dy < overlapping_cells.size.y; ++dy) {
		for (int dx = 0; dx < overlapping_cells.size.x; ++dx) {
			if (Level_is_solid(entity->world->level, overlapping_cells.position.x + dx, overlapping_cells.position.y + dy)) {
				obstructed = true;
				break;
			}
		}
	}

	return !obstructed;
}