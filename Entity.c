#include <stdlib.h>

#include "Entity.h"
#include "World.h"
#include "Graphics.h"
#include "Log.h"


Entity* Entity_create() {
	static unsigned long id_counter = 0;

	Entity* entity = calloc(1, sizeof(Entity));
	if (!entity) {
		Log_error("Entity", "failed to allocate memory for Entity base object.");
		return NULL;
	}
	entity->id = id_counter++;
	return entity;
}


void Entity_destroy(Entity* entity) {
	free(entity);
}


inline RectangleInt Entity_get_overlapping_cells(Entity* entity) {
	return World_get_overlapping_cells(entity->world, &(entity->rect));
}
