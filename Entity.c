#include <stdlib.h>

#include "Entity.h"
#include "World.h"


Entity* Entity_create() {
	static unsigned long id_counter = 0;

	Entity* entity = calloc(1, sizeof(Entity));
	if (!entity) {
		return NULL;
	}
	entity->id = id_counter++;
	return entity;
}
