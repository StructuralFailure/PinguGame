#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "SDLHelper.h"
#include "Graphics.h"
#include "Level.h"
#include "Log.h"
#include "Game.h"
#include "Entity.h"
#include "EntityCollection.h"
#include "ent/Entities.h"
#include "World.h"

#define START_TICK_RATE 60
#define LEVEL_PATH "assets/lvl/viewport_test"


static void test_game_creation(void);
static void test_world_loading(void);
static void test_line_segment_intersect(void);
static void test_vector_sum_array(void);
static void scanf_line_segment(LineSegment* ls);


int main(int argc, char** argv) 
{

    if (argc >= 1) {
        printf("%s\n", argv[0]);
    }

	Log_set_flush_after_printing(true);
	E_SDL sdl_e;
	if ((sdl_e = SDLHelper_init()) != E_SDL_SUCCESS) {
		Log_error("PinguGame", "failed to initialize SDL (error %d). aborting.", sdl_e);
		return 1;
	}
	test_game_creation();
	SDLHelper_quit();
	return 0;
}


static void test_entity_collection(void)
{
	EntityCollection ent_col;
	if (!EntityCollection_init(&ent_col, 4)) {
		Log_error("test_entity_collection", "failed to initialize entity collection.");
		return;
	}

	EntityCollection_add(&ent_col, NULL);
	EntityCollection_add(&ent_col, NULL);
	EntityCollection_add(&ent_col, NULL);
	Log("test_entity_collection", "size == %d | capacity == %d", ent_col.size, ent_col.capacity);
	EntityCollection_add(&ent_col, NULL);
	EntityCollection_add(&ent_col, NULL);
	Log("test_entity_collection", "size == %d | capacity == %d", ent_col.size, ent_col.capacity);
}


static void test_line_segment_intersect(void)
{
	LineSegment ls_1;
	LineSegment ls_2;


	printf("line segment 1:\n");
	scanf_line_segment(&ls_1);
	printf("\nline_segment 2:\n");
	scanf_line_segment(&ls_2);

	bool intersect = LineSegment_intersect(&ls_1, &ls_2);
	if (intersect) {
		printf("intersect!\n");
	} else {
		printf("do not intersect.\n");
	}
}


static void scanf_line_segment(LineSegment* ls) {
	printf("point_a.x := ");
	scanf("%f", &(ls->point_a.x));
	printf("point_a.y := ");
	scanf("%f", &(ls->point_a.y));
	printf("\n");
	printf("point_b.x := ");
	scanf("%f", &(ls->point_b.x));
	printf("point_b.y := ");
	scanf("%f", &(ls->point_b.y));
}


static void test_game_creation(void)
{
	Game* game = Game_create();
	Game_start(game);
	Game_destroy(game);
}


static void test_vector_sum_array(void)
{
	Vector2D vecs[] = {
		{ .x = 12, .y = 23 },
		{ .x = 34, .y = 45 },
		{ .x = 56, .y = 67 }
	};
	Vector2D result = Vector2D_sum_array(vecs, 3);
	Log("test_vector_sum_array", "result: [ x = %f | y = %f ]", result.x, result.y);
}


static void test_world_loading(void)
{
	World* world = World_load_from_path("assets/lvl/entity_test.lvl", NULL, true);
	if (!world) {
		Log_error("PinguGame", "failed to load world.");
		return;
	}

	for (int i = 0; i < 32; ++i) {
		Entity* entity = world->entities[i];
		if (!entity) {
			continue;
		}
		printf(
			"level  [ w = %d | h = %d ]\n"
			"entity [ type = %d | x = %f | y = %f ]\n",
			world->level->width, world->level->height,
			entity->type, entity->rect.position.x, entity->rect.position.y
		);
	} 
	
	World_destroy(world);
}