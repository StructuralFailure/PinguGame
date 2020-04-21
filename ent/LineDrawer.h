#ifndef ENTITY_LINEDRAWER_H
#define ENTITY_LINEDRAWER_H


#include "../Graphics.h"
#include "../Entity.h"
#include "../World.h"

typedef struct EntityLineDrawerData {
	LineSegment ls_a;
	LineSegment ls_b;

	bool ls_a_is_finished;
	bool ls_b_is_finished;

	bool ls_a_is_current;
	bool mouse_held_down;
} EntityLineDrawerData;


Entity* EntityLineDrawer_create();
void EntityLineDrawer_update(Entity*);
void EntityLineDrawer_draw(Entity*, Viewport*);
void EntityLineDrawer_destroy(Entity*);

Entity* EntityLineDrawer_deserialize(char* data);

#endif