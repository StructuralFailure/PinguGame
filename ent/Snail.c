//
// Created by fabian on 29.04.20.
//

#include "Log.h"
#include "Util.h"
#include "Snail.h"

#define SPEED  0.6
#define WIDTH  20
#define HEIGHT 16


typedef EntitySnailStickingDirection StickingDirection;

static SDL_Texture* tex_snail_vertical;
static SDL_Texture* tex_snail_horizontal;

static SDL_Texture* tex_solid_block;


Rectangle imaginary_wall;


void update_do_inner_turn(Entity* entity);
void update_do_outer_turn(Entity* entity);
StickingDirection mirror_sticking_direction(StickingDirection input);


Entity* EntitySnail_create(void)
{
    if (!tex_snail_vertical) {
    	tex_snail_vertical = SDLHelper_load_texture("assets/gfx/snail_vertical.bmp");
    }
    if (!tex_snail_horizontal) {
    	tex_snail_horizontal = SDLHelper_load_texture("assets/gfx/snail_horizontal.bmp");
    }
    if (!tex_solid_block) {
    	tex_solid_block = SDLHelper_load_texture("assets/gfx/item_block.bmp");
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

    snail->rect = (Rectangle) {
    	.position = { 0, 0 },
    	.size = { 20, 16 }
    };

    data->sticking = ESSD_UPWARDS;
    data->crawling_clockwise = true;
    data->state = ESS_CRAWLING;
    snail->data = data;

    Log("EntitySnail_create", "created.");
    return snail;
}


void EntitySnail_update(Entity* entity)
{
    ENTITY_DATA_ASSERT(Snail);



    Vector2D delta_pos_unit = { 0 };

    switch (data->sticking) {
    	case ESSD_RIGHTWARDS:
    		delta_pos_unit.y = +1;
    		break;
    	case ESSD_DOWNWARDS:
    		delta_pos_unit.x = -1;
    		break;
    	case ESSD_LEFTWARDS:
    		delta_pos_unit.y = -1;
    		break;
    	case ESSD_UPWARDS:
    		delta_pos_unit.x = +1;
    		break;
    	default:
    		;
    }

    Vector2D delta_pos = (Vector2D) { delta_pos_unit.x * SPEED, delta_pos_unit.y * SPEED };

    if (!data->crawling_clockwise) {
    	delta_pos.x *= -1;
    	delta_pos.y *= -1;
    }

	RectangleInt overlapping_cells = World_get_overlapping_cells(entity->world, &(entity->rect));
	Vector2DInt cell_to_check = { 0 };
	Vector2DInt cell_imaginary_wall = { 0 }; /* where the imaginary wall will be placed should cell_to_check not be solid. */


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
		/* TODO: implement this. */
	}
	if (!Level_is_solid_v2d(entity->world->level, &cell_to_check)) {

		Log("EntitySnail_update", "cell_to_check = { %d | %d }", cell_to_check.x, cell_to_check.y);
		Log("EntitySnail_update", "imaginary_wall.pos = { %f | %f }", imaginary_wall.position.x, imaginary_wall.position.y);
		Log("EntitySnail_update", "snail.pos = { %f | %f }", entity->rect.position.x, entity->rect.position.y);

		/* prefer outer turns:
		 * XXXXXXX    XXXXXXX   (. = empty space,
		 * ....S.. -> ....SS.    X = solid cell,
		 * XX..SXX    XX...XX    S = cell occupied by snail)
		 * XX...XX    XX...XX
		 * /
		/* check collision with imaginary wall */

		Log("EntitySnail_update", "space to go!");

		/*Rectangle*/ imaginary_wall = (Rectangle) {
			.position = {
					.x = cell_imaginary_wall.x * CM_CELL_WIDTH,
					.y = cell_imaginary_wall.y * CM_CELL_HEIGHT
			},
			.size = {
					.x = CM_CELL_WIDTH,
					.y = CM_CELL_HEIGHT
			}
		};

		//Rectangle_print(&imaginary_wall);

		CollidedWith collided_with = World_move_until_collision_with_flags(
			entity->world, &(entity->rect), &delta_pos, CC_RECTANGLE, &imaginary_wall
		);
		if (collided_with) {
			/* imaginary wall was hit, do outwards turn. */
			update_do_outer_turn(entity);
			Log("EntitySnail_update", "doing outer turn.");
		}
	} else {
		/* no imaginary wall was hit, continue movement and perhaps do inner turn. */
		CollidedWith collided_with = World_move_until_collision(
			entity->world, &(entity->rect), &delta_pos
		);
		if (collided_with) {
			update_do_inner_turn(entity);
		}
	}
}


void EntitySnail_draw(Entity* entity, Viewport* viewport)
{
    ENTITY_DATA_ASSERT(Snail);

    Viewport_draw_texture(viewport, &imaginary_wall, NULL, tex_solid_block);

    SDL_Texture* tex = NULL;
	bool flip_h = false;
	bool flip_v = false;

	switch (data->sticking) {
	case ESSD_RIGHTWARDS:
		tex = tex_snail_vertical;
		if (data->crawling_clockwise) {
			flip_h = true;
		} else {
			flip_h = true;
			flip_v = true;
		}
		break;
	case ESSD_DOWNWARDS:
		tex = tex_snail_horizontal;
		if (data->crawling_clockwise) {
			flip_h = true;
		}
		break;
	case ESSD_LEFTWARDS:
		tex = tex_snail_vertical;
		if (data->crawling_clockwise) {
			flip_v = true;
		}
		break;
	case ESSD_UPWARDS:
		tex = tex_snail_horizontal;
		if (data->crawling_clockwise) {
			flip_v = true;
		} else {
			flip_h = true;
			flip_v = true;
		}
	default:
		;
    }

    SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (flip_h) {
		flip |= SDL_FLIP_HORIZONTAL;
	}
	if (flip_v) {
		flip |= SDL_FLIP_VERTICAL;
	}

	int animation_frame = (entity->world->ticks / 10) % 3;
	Rectangle tex_rect = {
			.position = {
					.x = entity->rect.size.x * animation_frame,
					.y = 0
			},
			.size = entity->rect.size
	};

	Viewport_draw_texture_extended(viewport, &tex_rect, &(entity->rect), tex, flip);
}


void* EntitySnail_message(Entity* entity, Entity* sender, EntityMessageType type, void* payload)
{
	return NULL;
}


void EntitySnail_add(Entity* entity)
{

}


void EntitySnail_destroy(Entity* entity)
{
    ENTITY_DATA(Snail);
    if (data) {
        free(data);
    }
    free(entity);
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


void update_do_inner_turn(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	EntitySnailStickingDirection sticking_old = data->sticking;
	/* turn right of going clockwise, turn left otherwise. */
	Rectangle rect_old = entity->rect;
	float w_minus_h = abs_float(rect_old.size.x - rect_old.size.y);

	entity->rect.size = (Vector2D) {
		.x = rect_old.size.y,
		.y = rect_old.size.x
	};

	Log("EntitySnail: update_do_inner_turn", "turning.");

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
	Log("", "sticked.");
}


void update_do_outer_turn(Entity* entity) {
	ENTITY_DATA_ASSERT(Snail);

	/* we can abuse update_do_inner_turn:
	 * 1. mirror snail's sticking direction and whether it's going clockwise.
	 * 2. do inward turn,
	 * 3. mirror snail's sticking direction and whether it's going clockwise again.
	 * /

	/* crawling_clockwise refers to inner turns, need to take this into account here. */
	data->sticking = mirror_sticking_direction(data->sticking);
	data->crawling_clockwise = !data->crawling_clockwise;
	update_do_inner_turn(entity);
	data->sticking = mirror_sticking_direction(data->sticking);
	data->crawling_clockwise = !data->crawling_clockwise;
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