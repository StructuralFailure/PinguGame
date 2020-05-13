#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Util.h"
#include "Graphics.h"
#include "Level.h"
#include "Log.h"
#include "SDLHelper.h"
#include "World.h"
#include "Tileset.h"

#define CELL_TILESET_PATH "assets/gfx/tileset.bmp"
#define CELL_STATIC_IMAGE(TILESET_X, TILESET_Y) STATIC_ANIMATION(&cell_tileset, TILESET_X, TILESET_Y)

typedef enum LevelCellType LCT;
typedef enum LevelCellTypeFlags LCTFlags;
typedef struct LevelCellTypeProperties LCTProperties;


Tileset cell_tileset = {
		.tile_width = CM_CELL_WIDTH,
		.tile_height = CM_CELL_HEIGHT,
};


static LCTProperties cell_type_properties[__LCT_COUNT] = {
		[LCT_EMPTY] = {
				.type = LCT_EMPTY,
				.flags = LCTF_INVISIBLE
		},
		[LCT_SOLID_BLOCK] = {
				.type = LCT_SOLID_BLOCK,
				.flags = LCTF_SOLID,
				.animation = {
						.tileset = &cell_tileset,
						.length = 16,
						.position = {
								.x = 0,
								.y = 1
						}
				}
		},
		[LCT_LADDER] = {
				.type = LCT_LADDER,
				.animation = CELL_STATIC_IMAGE(0, 0)
		},
		[LCT_ITEM_BLOCK] = {
				.type = LCT_ITEM_BLOCK,
				.animation = CELL_STATIC_IMAGE(1, 0)
		},
		[LCT_SEMISOLID_BLOCK] = {
				.type = LCT_SEMISOLID_BLOCK,
				.flags = LCTF_SEMISOLID,
				.animation = CELL_STATIC_IMAGE(2, 0)
		},
		[LCT_EMPTY_ITEM_BLOCK] = {
				.type = LCT_EMPTY_ITEM_BLOCK,
				.animation = CELL_STATIC_IMAGE(3, 0)
		},
		[LCT_SNOWBALL] = {
				.type = LCT_SNOWBALL,
				.animation = CELL_STATIC_IMAGE(4, 0)
		},
		[LCT_ICE_BLOCK] = {
				.type = LCT_ICE_BLOCK,
				.flags = LCTF_SOLID,
				.animation = CELL_STATIC_IMAGE(5, 0)
		}
};


void load_tileset_texture(void)
{
	if (!cell_tileset.texture) {
		Log("Level", "load_tileset_texture: loading tileset texture.");
		cell_tileset.texture = SDLHelper_load_texture(CELL_TILESET_PATH);
	}
}


Level* Level_create(int width, int height)
{
	Level* level = calloc(1, sizeof(Level));
	level->width = width;
	level->height = height;
	level->colmap = calloc(height, sizeof(LevelCellType*));
	if (!level->colmap) {
		Log_error("Level_create", "failed to allocate memory for colmap rows.");
		free(level);
		return NULL;
	}

	for (int y = 0; y < height; ++y) {
		level->colmap[y] = calloc(width, sizeof(LevelCellType));
		if (!level->colmap[y]) {
			Log_error("Level_create", "failed to allocate memory for row %d.", y);
			for (int fy = 0; fy < y; ++fy) {
				free(level->colmap[fy]);
			}
			free(level->colmap);
			free(level);
			return NULL;
		}
	}

	return level;
}


Level* Level_load_from_file(FILE* file) 
{
	Log("Level_load_from_file", "loading level.");
	load_tileset_texture();

	/* determine width and height of level */
	int row_count = 1;
	int max_column_count = 0;
	int cur_column_count = 0;
	bool newline_terminated = true;
	int c;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			if (newline_terminated) {
				/* second new line encountered: stop parsing */
				break;
			}
			newline_terminated = true;
			cur_column_count = 0;
			++row_count;
		} else {
			newline_terminated = false;
			++cur_column_count;
			if (cur_column_count > max_column_count) {
				max_column_count = cur_column_count;
			}
		}
	}
	if (newline_terminated) {
		--row_count;
	}
	Log("Level", "loading level with width = %d and height = %d.", max_column_count, row_count);

	/*Level* level = Level_create(max_column_count, row_count);
	if (!level) {
		Log_error("Level_load_from_file", "failed to create level object.");
		return NULL;
	}*/

	Level* level = calloc(1, sizeof(Level));
	if (!level) {
		Log_error("Level", "failed to allocate memory for struct Level.");
		return NULL;
	}
	level->height = row_count;
	level->width = max_column_count;
	level->colmap = calloc(sizeof(LevelCellType*), row_count);
	if (!level->colmap) {
		Log_error("Level", "unable to allocate memory for colmap.");
		return NULL;
	}

	for (int y = 0; y < row_count; ++y) {
		level->colmap[y] = calloc(sizeof(LevelCellType), max_column_count);
		if (!level->colmap[y]) {
			Log_error("Level", "unable to allocate memory for colmap[%d].", y);
			return NULL;
		}
	}

	/* actually read level */
	rewind(file);
	int row = 0;
	int col = 0;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			++row;
			if (row >= row_count) {
				break;
			}
			col = 0;
			continue;
		}

        if (c == ' ') {
            level->colmap[row][col] = 0;
        } else {
			/* characters for '0' to '9' are guaranteed to be
			 * contiguous in character set
			 */
			int cell_type = c - '0';
			level->colmap[row][col] = cell_type;
		}
        ++col;
    }

	/* dump level */
	Log("Level", "content dump:");

	for (int y = 0; y < row_count; ++y) {
		for (int x = 0; x < max_column_count; ++x) {
			printf("%d ", level->colmap[y][x]);
		}
		printf("\n");
	}

	Log("Level", "loaded.");
	return level;
}


Level* Level_load_from_path(const char* file_path) 
{
	Log("Level", "loading level from path %s.", file_path);

	FILE* file = fopen(file_path, "r");
	if (!file) {
		return NULL;
	}
	Level* level = Level_load_from_file(file);
	fclose(file);
	return level;
}


void Level_destroy(Level* level)
{
	if (!level) {
		return;
	}

	for (int y = 0; y < level->height; ++y) {
		free(level->colmap[y]);
	}
	free(level);

	Log("Level", "destroyed.");
}


LevelCellTypeProperties* Level_get_cell_type_properties(Level* level, int x, int y) 
{
	if (x < 0 || x >= level->width ||
	    y < 0 || y >= level->height) {
		return NULL;;
	}
	return &(cell_type_properties[level->colmap[y][x]]);
}

bool Level_set_cell_type(Level* level, int x, int y, LevelCellType type) 
{
	if (x < 0 || x >= level->width ||
	    y < 0 || y >= level->height) {
		return false;
	}
	level->colmap[y][x] = type;
	return true;
}


LevelCellTypeFlags Level_get_cell_type_flags(Level* level, int x, int y)
{
	if (x < 0 || x >= level->width ||
	    y < 0 || y >= level->height) {
		return LCTF_NONE;
	}
	/*LevelCellType cell_type = level->colmap[y][x];
	if (cell_type < 0 || cell_type >= __LCT_COUNT) {
		return LCTF_INVALID;
	}*/
	return cell_type_properties[level->colmap[y][x]].flags;
}


bool Level_is_solid(Level* level, int x, int y) 
{
	if (x < 0 || x >= level->width ||
	    y < 0 || y >= level->height) {
		return false;
	}
	return cell_type_properties[level->colmap[y][x]].flags & LCTF_SOLID;
	//return level->colmap[y][x] & LCTF_SOLID;
}

bool Level_is_solid_v2d(Level* level, Vector2DInt* cell) 
{
	return Level_is_solid(level, cell->x, cell->y);
}
