#ifndef WORLD_H
#define WORLD_H

#include "Level.h"
#include "Entity.h"
#include "Viewport.h"
#include "Forward.h"
#include "Game.h"


#define MAX_ENTITY_COUNT 32
#define CM_CELL_WIDTH  16
#define CM_CELL_HEIGHT 16


typedef struct World {
	Game* game;
	Level* level;
	Entity* entities[MAX_ENTITY_COUNT];
	Viewport* viewport;
} World;


World* World_load_from_path(const char* file_name, bool load_entities);
void World_destroy(World* world);

bool World_add_entity(World* world, Entity* entity);
bool World_remove_entity(World* world, Entity* entity);

void World_set_level(World* world, Level* level);

CollidedWith World_move_until_collision(World* world, Rectangle* rect, const Vector2D* delta_pos);
CollidedWith World_move(World* world, Entity* entity, Vector2D* delta_pos);

RectangleInt World_get_overlapping_cells(World* world, Rectangle* rect);
Rectangle World_get_cell_rectangle(World* world, Vector2DInt* grid_position);
bool World_rectangle_overlaps_cell_of_type(World* world, Rectangle* rect, LevelCellType cell_type);

#endif