#ifndef H_LEVEL
#define H_LEVEL

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "Graphics.h"
#include "Tileset.h"


typedef enum LevelCellType {
	LCT_EMPTY = 0,
	LCT_SOLID_BLOCK = 1,
	LCT_LADDER = 2,
	LCT_ITEM_BLOCK = 3,
	LCT_SEMISOLID_BLOCK = 4,
	LCT_EMPTY_ITEM_BLOCK = 5,
	LCT_SNOWBALL = 6,
	LCT_ICE_BLOCK = 7,
	__LCT_COUNT
} LevelCellType;


typedef enum LevelCellTypeFlags {
	LCTF_NONE = 0,
	LCTF_SOLID = 1,
	LCTF_SEMISOLID = 2, /* can only be passed through from the bottom */
	LCTF_INVISIBLE = 4
} LevelCellTypeFlags;


typedef struct LevelCellTypeProperties {
	LevelCellType type;
	LevelCellTypeFlags flags;
	Animation animation;
} LevelCellTypeProperties;


typedef struct Level {
	int width;
	int height;
	LevelCellType** colmap;
} Level;


Tileset cell_tileset;


Level* Level_create(int width, int height);
Level* Level_load_from_path(const char* filename);
Level* Level_load_from_file(FILE* file);
void Level_destroy(Level* level);

bool Level_is_solid(Level* level, int x, int y);
bool Level_is_solid_v2d(Level* level, Vector2DInt* cell);
LevelCellTypeFlags Level_get_cell_type_flags(Level* level, int x, int y);
LevelCellTypeProperties* Level_get_cell_type_properties(Level* level, int x, int y);
bool Level_set_cell_type(Level* level, int x, int y, LevelCellType type);


#endif
