#ifndef H_LEVEL
#define H_LEVEL

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "Graphics.h"


typedef enum LevelCellType {
	LCT_EMPTY = 0,
	LCT_SOLID_BLOCK = 1,
	LCT_LADDER = 2,
	LCT_ITEM_BLOCK = 3,
	__LCT_COUNT
} LevelCellType;


typedef enum LevelCellTypeFlags {
	LCTF_INVALID = -1,
	LCTF_SOLID = 1,
	LCTF_INVISIBLE = 2
} LevelCellTypeFlags;


typedef struct LevelCellTypeProperties {
	LevelCellType type;
	LevelCellTypeFlags flags;
	SDL_Texture* texture;
} LevelCellTypeProperties;


typedef struct Level {
	int width;
	int height;
	LevelCellType** colmap;
} Level;


Level* Level_create_from_file(const char* filename);
void Level_destroy(Level* level);

bool Level_is_solid(Level* level, int x, int y);
bool Level_is_solid_v2d(Level* level, Vector2DInt* cell);
LevelCellTypeFlags Level_get_cell_type_flags(Level* level, int x, int y);
LevelCellTypeProperties* Level_get_cell_type_properties(Level* level, int x, int y);


#endif
