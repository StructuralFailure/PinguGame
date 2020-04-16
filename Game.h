#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "Forward.h"
#include "Entity.h"
#include "Level.h"
#include "Graphics.h"

#define MAX_ENTITY_COUNT 32
#define CM_CELL_WIDTH  16
#define CM_CELL_HEIGHT 16

typedef enum CollidedWith {
	/* bit field as there can be multiple collisions in one step */
	CW_NOTHING = 0,
	CW_LEFT    = 1,
	CW_TOP     = 2,
	CW_RIGHT   = 4,
	CW_BOTTOM  = 8,
} CollidedWith;


typedef enum Direction {
	DIR_LEFT = 0,
	DIR_UP,
	DIR_RIGHT,
	DIR_DOWN,
} Direction;


struct Game {
	Level* current_level;
	Entity* entities[MAX_ENTITY_COUNT];
	int entity_count;
};

/*               */
/* basic methods */
/*               */
Game* Game_create();
void Game_update(Game* game);
void Game_draw(Game* game);
void Game_destroy(Game* game);

/*                */
/* engine methods */
/*                */

/* entities and level properties */
bool Game_add_entity(Game* game, Entity* entity);
bool Game_remove_entity(Game* game, Entity* entity);
void Game_set_level(Game* game, Level* level);

/* collision detection */
CollidedWith Game_move_until_collision(Game* game, Rectangle* rect, const Vector2D* delta_pos);
CollidedWith Game_move(Game* game, Entity* entity, Vector2D* delta_pos);
RectangleInt Game_get_overlapping_cells(Game* game, Rectangle* rect);
Rectangle Game_get_cell_rectangle(Game* game, Vector2DInt* grid_position);


#endif