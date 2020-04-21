#include <stdlib.h>

#include "Entity.h"
#include "World.h"
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
