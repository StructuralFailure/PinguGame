#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Util.h"
#include "Graphics.h"
#include "Level.h"
#include "SDLHelper.h"


typedef enum LevelCellType LCT;
typedef enum LevelCellTypeFlags LCTFlags;
typedef struct LevelCellTypeProperties LCTProperties;

LevelCellTypeProperties* cell_type_properties;

void init_cell_type_properties();


#if 0
LevelCellTypeFlags cell_type_flags[__LCT_COUNT] = {
	/* 0 = empty cell */
	0,
	/* 1 = solid block */
	LCTF_SOLID,
	/* 2 = ladder */
	0,
	/* 3 = item block */
	LCTF_SOLID
};
#endif



void init_cell_type_properties() 
{
	if (cell_type_properties) {
		return;
	}

	cell_type_properties = calloc(__LCT_COUNT, sizeof(LCTProperties));

	cell_type_properties    [LCT_EMPTY]         = (LCTProperties) {
		.type = LCT_EMPTY,
		.flags = 0,
		.texture = NULL
	};

	cell_type_properties    [LCT_SOLID_BLOCK]   = (LCTProperties) {
		.type = LCT_SOLID_BLOCK,
		.flags = LCTF_SOLID,
		.texture = SDLHelper_load_texture("assets/gfx/solid_block_tileset.bmp")
	};
	
	cell_type_properties    [LCT_LADDER]        = (LCTProperties) {
		.type = LCT_LADDER,
		.flags = 0,
		.texture = SDLHelper_load_texture("assets/gfx/ladder.bmp")
	};

	cell_type_properties    [LCT_ITEM_BLOCK]    = (LCTProperties) {
		.type = LCT_ITEM_BLOCK,
		.flags = LCTF_SOLID,
		.texture = SDLHelper_load_texture("assets/gfx/item_block.bmp")
	};
}


Level* Level_create_from_file(const char* filename) 
{
	init_cell_type_properties();

	FILE* file = fopen(filename, "r");
	if (!file) {
		return NULL;
	}
	/* determine width and height of level */
	int row_count = 1;
	int max_column_count = 0;
	int cur_column_count = 0;
	bool newline_terminated = true;
	int c;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			newline_terminated = true;
			cur_column_count = 0;
			++row_count;
		} else {
			newline_terminated = false;
			++cur_column_count;
			max_column_count = max(max_column_count, cur_column_count);
		}
	}
	if (newline_terminated) {
		--row_count;
	}

	Level* level = malloc(sizeof(Level));
	if (!level) {
		return NULL;
	}
	level->height = row_count;
	level->width = max_column_count;
	level->colmap = calloc(sizeof(LevelCellType*), row_count);

	for (int y = 0; y < row_count; ++y) {
		level->colmap[y] = calloc(sizeof(LevelCellType), max_column_count);
	}

	/* actually read level */
	rewind(file);
	int row = 0;
	int col = 0;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n') {
			++row;
			col = 0;
		} else {
			/* characters for '0' to '9' are guaranteed to be
			 * contiguous in character set
			 */
			int cell_type = c - '0';
			level->colmap[row][col] = cell_type;
			++col;
		}
	}

	/* dump level */
	printf("[Level] loaded level from %s:\n", filename);
	for (int y = 0; y < row_count; ++y) {
		for (int x = 0; x < max_column_count; ++x) {
			printf("%d ", level->colmap[y][x]);
		}
		printf("\n");
	}

	fclose(file);
	return level;
}

void Level_destroy(Level* level)
{
	for (int y = 0; y < level->height; ++y) {
		free(level->colmap[y]);
	}
	free(level);
}


LevelCellTypeProperties* Level_get_cell_type_properties(Level* level, int x, int y) {
	if (x < 0 || x >= level->width ||
		y < 0 || y >= level->height) {
		return NULL;;
	}
	return &(cell_type_properties[level->colmap[y][x]]);
}


LevelCellTypeFlags Level_get_cell_type_flags(Level* level, int x, int y) {
	if (x < 0 || x >= level->width ||
		y < 0 || y >= level->height) {
		return LCTF_INVALID;
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
	return level->colmap[y][x] & LCTF_SOLID;
}

bool Level_is_solid_v2d(Level* level, Vector2DInt* cell) 
{
	return Level_is_solid(level, cell->x, cell->y);
}
