#include <stdbool.h>
#include <stdlib.h>

#include "World.h"
#include "Log.h"
#include "Level.h"
#include "Graphics.h"
#include "Entity.h"
#include "Viewport.h"
#include "ent/Entities.h"


/* what in the ever-loving fuck? */
bool (*entity_serializers[__ET_COUNT])(Entity* entity, char* output) = {
	[ET_PLAYER] = EntityPlayer_serialize,
	[ET_ENEMY]  = EntityEnemy_serialize,
	[ET_TEXT]  =  EntityText_serialize
};

Entity* (*entity_deserializers[__ET_COUNT])(char* string) = {
	[ET_PLAYER] = EntityPlayer_deserialize,
	[ET_ENEMY]  = EntityEnemy_deserialize,
	[ET_TEXT]   = EntityText_deserialize
};


World* World_load_from_path(const char* file_path, bool load_entities) 
{
	FILE* file = fopen(file_path, "r");
	if (!file) {
		Log_error("World", "failed to open file at %s.", file_path);
		goto fail_file;
	}

	World* world = calloc(1, sizeof(World));
	if (!world) {
		Log_error("World", "failed to allocate memory for struct World.");
		goto fail_world;
	}

	Level* level = Level_load_from_file(file);
	if (!level) {
		Log_error("World", "failed to load level from file %s.", file_path);
		goto fail_level;
	}

	Viewport* viewport = Viewport_create();
	if (!viewport) {
		Log_error("World", "failed to create viewport.");
		goto fail_viewport;
	}
	viewport->world = world;
	viewport->total.size = (Vector2D) {
		.x = level->width * CM_CELL_WIDTH,
		.y = level->height * CM_CELL_HEIGHT
	};

	world->level = level;
	world->viewport = viewport;

	/* initialize view port with some default values.
	 * TODO: maybe load viewport parameters from level file.
	 */
	
	if (load_entities) {
		char line_buffer[256];

		while (fgets(line_buffer, sizeof(line_buffer), file)) {
			if (line_buffer[0] == '\n') {
				continue;
			}
			int entity_id;
			sscanf(line_buffer, "%d ", &entity_id);

			if (entity_id < 0 || entity_id >= __ET_COUNT) {
				Log_error("World", "loading: invalid entity type %d", entity_id);
				continue;
			}

			if (!entity_deserializers[entity_id]) {
				Log_error("World", "loading: no deserializer for entity with type %d.", entity_id);
				continue;
			}

			Entity* entity = (entity_deserializers[entity_id])(line_buffer);
			if (!entity) {
				Log_error("World", "loading: failed to deserialize entity with type %d.", entity_id);
				continue; 
			}

			Log("World", "loading: adding entity with type %d.", entity_id);
			World_add_entity(world, entity);
		}
	}

	fclose(file);
	Log("World", "created.");
	return world;

	/* error handling */
fail_viewport:	Level_destroy(level);
fail_level:	World_destroy(world);
fail_world:	fclose(file);
fail_file:	Log_error("World", "failed to create.");
		return NULL;
}


void World_draw(World* world) 
{
	Viewport_draw(world->viewport);
}

void World_update(World* world) 
{
	/* call Entity objects' update functions */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = world->entities[i];
		if (ent && ent->update) {
			ent->update(ent);
		}
	}

	/* check for collisions */
	for (int ea_i = 0; ea_i < MAX_ENTITY_COUNT; ++ea_i) {
		Entity* ea = world->entities[ea_i];
		if (!(ea && ea->collide)) {
			continue;
		}
		for (int eb_i = 0; eb_i < MAX_ENTITY_COUNT; ++eb_i) {
			Entity* eb = world->entities[eb_i];
			if ((ea_i == eb_i) || !eb) {
				continue;
			}
			if (Rectangle_overlap(&(eb->rect), &(ea->rect))) {
				ea->collide(ea, eb);
			}
		}
	}

	Viewport_update(world->viewport);
}

bool World_add_entity(World* world, Entity* entity)
{
	if (!entity) {
		return false;
	}
	Log("World", "adding entity. type = %d | id = %d", entity->type, entity->id);

	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		if (!world->entities[i]) {
			entity->world = world;
			world->entities[i] = entity;
			if (entity->add) {
				entity->add(entity);
			}
			return true;
		}
	}
	return false;
}


bool World_remove_entity(World* world, Entity* entity)
{
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		if (world->entities[i] == entity) {
			if (entity->destroy) {
				entity->destroy(entity);
			}
			world->entities[i] = NULL;
			return true;
		}
	}
	return false;
}


/* changes level and viewport */
/*void World_set_level(World* world, Level* level) 
{
	if (world->level) {
		Level_destroy(world->level);
	}
	world->level = level;

	if (level && world->viewport) {
		world->viewport->total.size = (Vector2D) {
			.x = level->width * CM_CELL_WIDTH,
			.y = level->height * CM_CELL_HEIGHT
		};
	}
}*/


CollidedWith World_move_until_collision(World* world, Rectangle* rect, const Vector2D* delta_pos) 
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
		RectangleInt cells = World_get_overlapping_cells(world, rect);

		is_colliding = false;
		for (int dy = 0; dy < cells.size.y; ++dy) {
			for (int dx = 0; dx < cells.size.x; ++dx) {
				Vector2DInt current_cell = {
					.x = cells.position.x + dx,
					.y = cells.position.y + dy
				};

				if (!Level_is_solid(world->level, current_cell.x, current_cell.y)) {
					/* cell not solid -> no collision */
					continue;
				}

				is_colliding = true;
				has_collided = true;
				rect_last_collision = World_get_cell_rectangle(world, &current_cell);
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
			if (!Level_is_solid(world->level, calc_x, calc_y)) {
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

/* attempts to move a Rectangle object.
 * will continue movement even when a collision is encountered,
 * but will not pass through solid cells attempting to do so.
 *
 * returns, in case of collision, the edge or edges of the solid
 * cell or cells that were run into.
 */
CollidedWith World_move(World* world, Entity* entity, Vector2D* delta_pos) 
{
	Rectangle rect_start = entity->rect;
	Vector2D rect_start_pos = entity->rect.position;


	/* first part of movement */
	CollidedWith collided_with = World_move_until_collision(world, &(entity->rect), delta_pos);
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
		collided_with |= World_move_until_collision(world, &(entity->rect), &remaining_delta_pos);
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
		collided_with |= World_move_until_collision(world, &(entity->rect), &remaining_delta_pos);
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



RectangleInt World_get_overlapping_cells(World* world, Rectangle* rect) 
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
Rectangle World_get_cell_rectangle(World* world, Vector2DInt* grid_position) 
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


bool World_rectangle_overlaps_cell_of_type(World* world, Rectangle* rect, LevelCellType cell_type)
{
	RectangleInt overlapping_cells = World_get_overlapping_cells(world, rect);

	for (int dy = 0; dy < overlapping_cells.size.y; ++dy) {
		for (int dx = 0; dx < overlapping_cells.size.x; ++dx) {
			LevelCellTypeProperties* properties = Level_get_cell_type_properties(
				world->level,
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



void World_destroy(World* world) 
{
	if (!world) {
		return;
	}

	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = world->entities[i];
		if (ent && ent->destroy) {
			ent->destroy(ent);
		}
	}
	Level_destroy(world->level);
	Viewport_destroy(world->viewport);
	free(world);

	Log("World", "destroyed.");

}