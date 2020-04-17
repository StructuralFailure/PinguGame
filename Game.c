#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include "Game.h"
#include "Entity.h"
#include "Level.h"
#include "Graphics.h"
#include "SDLHelper.h"
#include "Viewport.h"


Game* Game_create()
{
	Game* game = calloc(1, sizeof(Game));
	if (!game) {
		return NULL;
	}

	Viewport* viewport = Viewport_create();
	viewport->game = game;
	viewport->total = (Rectangle) {
		.position = {
			.x = 0,
			.y = 0 
		},
		.size = {
			.x = 320,
			.y = 320
		}
	};
	viewport->visible = (Rectangle) {
		.size = {
			.x = 320,
			.y = 320
		}
	};
	viewport->camera_distance = (Vector2D) {
		.x = 200,
		.y = 160
	};
	viewport->camera_speed = (Vector2D) {
		.x = 3.5,
		.y = 3.5
	};
	game->viewport = viewport;
	return game;
}

/* tries to add an entity to the first available slot.
 * if there are no vacant slots, the entity is not added and false is returned,
 * otherwise the entity is added and true is returned.
 */
bool Game_add_entity(Game* game, Entity* entity)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		if (!game->entities[i]) {
			entity->game = game;
			game->entities[i] = entity;
			if (entity->add) {
				entity->add(entity);
			}
			return true;
		}
	}
	return false;
}


bool Game_remove_entity(Game* game, Entity* entity)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		if (game->entities[i] == entity) {
			if (entity->remove) {
				entity->remove(entity);
			}
			game->entities[i] = NULL;
			return true;
		}
	}
	return false;
}


void Game_draw(Game* game) {
	if (!game->viewport) {
		return;
	}
	Viewport_draw(game->viewport);
}

void Game_update(Game* game) 
{
	/* call Entity objects' update functions */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = game->entities[i];
		if (ent && ent->update) {
			ent->update(ent);
		}
	}

	/* check for collisions */
	for (int ea_i = 0; ea_i < MAX_ENTITY_COUNT; ++ea_i) {
		Entity* ea = game->entities[ea_i];
		if (!(ea && ea->collide)) {
			continue;
		}
		for (int eb_i = 0; eb_i < MAX_ENTITY_COUNT; ++eb_i) {
			Entity* eb = game->entities[eb_i];
			if ((ea_i == eb_i) || !eb) {
				continue;
			}
			if (Rectangle_overlap(&(eb->rect), &(ea->rect))) {
				ea->collide(ea, eb);
			}
		}
	}

	Viewport_update(game->viewport);
}


/* changes level and viewport */
void Game_set_level(Game* game, Level* level) 
{
	if (game->current_level) {
		Level_destroy(game->current_level);
	}
	game->current_level = level;

	if (level && game->viewport) {
		game->viewport->total.size = (Vector2D) {
			.x = level->width * CM_CELL_WIDTH,
			.y = level->height * CM_CELL_HEIGHT
		};
	}
}


/* attempts to move a Rectangle object's position by a specific amount.
 * ceases movement when a solid cell of the collision map is hit.
 *
 * returns which edge (sg.) of a solid cell was collided with or 0 (CW_NONE)
 * if no collision occurred.
 */
CollidedWith Game_move_until_collision(Game* game, Rectangle* rect, const Vector2D* delta_pos) 
{
	float delta_length = sqrt(delta_pos->x * delta_pos->x + delta_pos->y * delta_pos->y);
	Vector2D delta_pos_norm = {
		.x = delta_pos->x / delta_length,
		.y = delta_pos->y / delta_length
	};

	Vector2D pos_original = rect->position;

	/* move object to desired position */
	rect->position.x += delta_pos->x;
	rect->position.y += delta_pos->y;

	/* check whether the entity is now overlapping a
	 * solid cell. if so, continuously move the entity
	 * back 1 unit at a time until no collision takes place
	 * any longer or until the entity has moved back enough
	 * to reach its original position.
	 */
	bool has_collided = false;
	bool is_colliding = false;
	int dist_moved_back = 0;
	Rectangle rect_last_collision;

	do {
		/* (should be done, gotta test) TODO: 
		 * add support for rectangles bigger than the cells' size
		 */
		RectangleInt cells = Game_get_overlapping_cells(game, rect);

		is_colliding = false;
		for (int dy = 0; dy < cells.size.y; ++dy) {
			for (int dx = 0; dx < cells.size.x; ++dx) {
				Vector2DInt current_cell = {
					.x = cells.position.x + dx,
					.y = cells.position.y + dy
				};

				if (!Level_is_solid(game->current_level, current_cell.x, current_cell.y)) {
					/* cell not solid -> no collision */
					continue;
				}

				is_colliding = true;
				has_collided = true;
				rect_last_collision = Game_get_cell_rectangle(game, &current_cell);
			}
		}

		if (is_colliding) {
			rect->position.x -= delta_pos_norm.x;
			rect->position.y -= delta_pos_norm.y;
			dist_moved_back += 1;

			if (dist_moved_back >= delta_length) {
				/* TODO (maybe?):
				 * fix rect being unable to free itself if it gets stuck
				 * inside a solid cell.
				 */
				rect->position = pos_original;
				break;
			}
		}

	} while (is_colliding);

	if (has_collided) {
		/* moving the rectangale back outside the cell it collided with
		 * likely leaves a gap which needs to be closed.
		 */
		if (rect->position.y >= rect_last_collision.position.y + rect_last_collision.size.y) {
			rect->position.y = rect_last_collision.position.y + rect_last_collision.size.y;
			return CW_BOTTOM;
		} else if (rect->position.y + rect->size.y <= rect_last_collision.position.y) {
			/* since collision checks with an entity's overlapping cells are evaluated from
			 * top to bottom and there's no early exit of the loop (TODO ?),
			 * in the case of collision with multiple cells, the bottom-most cell will be the
			 * one the engine sees as the point of last collision. this leads to the following
			 * edge case:
			 *
			 * if there's a perfect diagonal movement, like this:
			 *
			 * ....    .    = empty
			 * .OA.    O    = entity moving diagonally to the right and downwards
			 * ..B.    A, B = solid cells
			 *
			 * the engine thinks the entity collided with the top of B, so it won't allow it
			 * to continue its downward movement. to mitigate this, there is a check to see
			 * if there is another solid cell A on top of B, which would render a top collision
			 * with B impossible. in this case, the logic falls through to the part where we check
			 * for lateral collisions, which do not impede vertical movement.
			 * in case A is not a solid cell, the engine will register the collision as a
			 * top collision, allowing the movement to complete its horizontal part. this means
			 * that if an entity hits an edge perfectly and there is space above it, then
			 * it will be treated as if it had hit the top.
			 */

			/* calc = cell above last collision */
			int calc_x = rect_last_collision.position.x / CM_CELL_WIDTH;
			int calc_y = rect_last_collision.position.y / CM_CELL_HEIGHT - 1;
			if (!Level_is_solid(game->current_level, calc_x, calc_y)) {
				rect->position.y = rect_last_collision.position.y - rect->size.y;
				return CW_TOP;
			}
		} if (rect->position.x + rect->size.x <= rect_last_collision.position.x) {
			rect->position.x = rect_last_collision.position.x - rect->size.x;
			return CW_LEFT;
		} else if (rect->position.x >= rect_last_collision.position.x + rect_last_collision.size.x) {
			rect->position.x = rect_last_collision.position.x + rect_last_collision.size.x;
			return CW_RIGHT;
		} else {
			fprintf(stderr, "Fabi should learn how to write collision engines properly.\n");
		}
	}
	return CW_NOTHING;
}

/* DEBUG METHOD
 */
void print_collision(CollidedWith cw) 
{
	if (cw & CW_TOP) {
		printf("top ");
	} else if (cw & CW_BOTTOM) {
		printf("bottom ");
	}

	if (cw & CW_LEFT) {
		printf("left");
	} else if (cw & CW_RIGHT) {
		printf("right");
	}

	printf("\n");
	fflush(stdout);
}


/* attempts to move a Rectangle object.
 * will continue movement even when a collision is encountered,
 * but will not pass through solid cells attempting to do so.
 *
 * returns, in case of collision, the edge or edges of the solid
 * cell or cells that were run into.
 */
CollidedWith Game_move(Game* game, Entity* entity, Vector2D* delta_pos) 
{
	Rectangle rect_start = entity->rect;
	Vector2D rect_start_pos = entity->rect.position;


	/* first part of movement */
	CollidedWith collided_with = Game_move_until_collision(game, &(entity->rect), delta_pos);
	/*if (collided_with) {
		printf("step 1: ");
		print_collision(collided_with);	
	}*/

	/* second part of movement */
	switch (collided_with) {
	case CW_LEFT:
	case CW_RIGHT: {
		/* complete vertical movement */
		float remaining_y = rect_start_pos.y + delta_pos->y - entity->rect.position.y;
		Vector2D remaining_delta_pos = {
			.x = 0,
			.y = remaining_y
		};
		collided_with |= Game_move_until_collision(game, &(entity->rect), &remaining_delta_pos);
		break;
	}
	case CW_TOP:
	case CW_BOTTOM: {
		/* complete horizontal movement */
		float remaining_x = rect_start_pos.x + delta_pos->x - entity->rect.position.x;
		Vector2D remaining_delta_pos = {
			.x = remaining_x,
			.y = 0
		};
		collided_with |= Game_move_until_collision(game, &(entity->rect), &remaining_delta_pos);
		break;
	}
	default:
		; /* movement was completed in one part */
	}

	/*if (collided_with) {
		printf("  step 2: ");
		print_collision(collided_with);	
	}*/
	entity->rect_prev = rect_start;
	return collided_with;
}

void Game_destroy(Game* game)
{
	/* TODO: call all entities' remove functions */
	free(game->viewport);
	free(game->current_level);
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = game->entities[i];
		if (!(ent && ent->remove)) {
			continue;
		}
		ent->remove(ent);
	}
	free(game);
}


RectangleInt Game_get_overlapping_cells(Game* game, Rectangle* rect) 
{
	Vector2DInt cell_top_left = {
		.x = rect->position.x / CM_CELL_WIDTH,
		.y = rect->position.y / CM_CELL_HEIGHT
	};

	Vector2DInt cell_bottom_right = {
		.x = (rect->position.x + rect->size.x) / CM_CELL_WIDTH,
		.y = (rect->position.y + rect->size.y) / CM_CELL_HEIGHT
	};

	/* if the bottom/right side of a rectangle is perfectly aligned with a
	 * cell's top/left side, cell_bottom_right will erroneously point an index too far.
	 * in case this happpens, we need to subtract one.
	 */

	Vector2D edge_quotients = {
		.x = (rect->position.x + rect->size.x) / CM_CELL_WIDTH,
		.y = (rect->position.y + rect->size.y) / CM_CELL_HEIGHT
	};

	bool on_right_edge = floorf(edge_quotients.x) == edge_quotients.x;
	bool on_bottom_edge = floorf(edge_quotients.y) == edge_quotients.y;

	/*if (on_right_edge) {
		--cell_bottom_right.x;
	}
	if (on_bottom_edge) {
		--cell_bottom_right.y;
	}*/

	return (RectangleInt) {
		.position = {
			.x = cell_top_left.x,
			.y = cell_top_left.y
		},
		.size = {
			.x = cell_bottom_right.x - cell_top_left.x + 1 - on_right_edge,
			.y = cell_bottom_right.y - cell_top_left.y + 1 - on_bottom_edge
		}
	};

}


/* returns the Rectangle a cell covers.
 */
Rectangle Game_get_cell_rectangle(Game* game, Vector2DInt* grid_position) 
{
	Rectangle result = {
		.position = {
			.x = CM_CELL_WIDTH * grid_position->x,
			.y = CM_CELL_HEIGHT * grid_position->y
		},
		.size = {
			.x = CM_CELL_WIDTH,
			.y = CM_CELL_HEIGHT
		}
	};
	return result;
}


bool Game_rectangle_overlaps_cell_of_type(Game* game, Rectangle* rect, LevelCellType cell_type)
{
	RectangleInt overlapping_cells = Game_get_overlapping_cells(game, rect);

	for (int dy = 0; dy < overlapping_cells.size.y; ++dy) {
		for (int dx = 0; dx < overlapping_cells.size.x; ++dx) {
			LevelCellTypeProperties* properties = Level_get_cell_type_properties(
				game->current_level,
				overlapping_cells.position.x + dx,
				overlapping_cells.position.y + dy
			);

			if (properties->type == cell_type) {
				return true;
			}
		}
	}
	return false;
}
