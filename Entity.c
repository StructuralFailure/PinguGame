#include <stdlib.h>

#include "Entity.h"


Entity* Entity_create() {
	Entity* entity = calloc(1, sizeof(Entity));
	if (!entity) {
		return NULL;
	}
	return entity;
}
