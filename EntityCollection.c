//
// Created by fabian on 10.05.20.
//

#include <stdlib.h>
#include "EntityCollection.h"
#include "Entity.h"
#include "Log.h"

#define ENLARGEMENT_FACTOR 2.0


static bool ensure_capacity(EntityCollection*);


bool EntityCollection_init(EntityCollection* entity_col, int initial_capacity)
{
	if (!(entity_col->elements = calloc(initial_capacity, sizeof(Entity*)))) {
		Log_error("EntityCollection_init", "failed to allocate memory. initial_capacity = %d", initial_capacity);
		return false;
	}

	entity_col->capacity = initial_capacity;
	entity_col->size = 0;
	if (!ensure_capacity(entity_col)) {
		Log_error("EntityCollection_init", "failed to ensure capacity.");
		return false;
	}

	return true;
}


void EntityCollection_deinit(EntityCollection* entity_col)
{
	free(entity_col->elements);
	entity_col->size = 0;
	entity_col->capacity = 0;
}


bool EntityCollection_add(EntityCollection* entity_col, Entity* entity)
{
	++entity_col->size;
	if (!ensure_capacity(entity_col)) {
		Log_error("EntityCollection_add", "failed to ensure capacity.");
		--entity_col->size;
		return false;
	}

	entity_col->elements[entity_col->size] = entity;
	return true;
}


bool EntityCollection_remove(EntityCollection* entity_col, Entity* entity)
{
	int index = EntityCollection_find(entity_col, entity);
	if (index == -1) {
		return false;
	}
	EntityCollection_remove_at(entity_col, index);
	return true;
}


bool EntityCollection_remove_at(EntityCollection* entity_col, int index)
{
	if (entity_col->size == 0) {
		Log_error("EntityCollection_remove_at", "invalid index: %d. size == 0", index);
		return false;
	}

	if (index < 0 || index >= entity_col->size) {
		Log_error("EntityCollection_remove_at", "invalid index: %d", index);
		return false;
	}

	entity_col->elements[index] = NULL;
	entity_col->elements[index] = entity_col->elements[entity_col->size - 1];
	entity_col->elements[entity_col->size - 1] = NULL;
	--entity_col->size;

	return true;
}


int EntityCollection_find(EntityCollection* entity_col, Entity* entity)
{
	for (int i = 0; i < entity_col->size; ++i) {
		if (entity_col->elements[i] == entity) {
			return i;
		}
	}
	return -1;
}


/* makes sure that the array is big enough. */
static bool ensure_capacity(EntityCollection* entity_col)
{
	if (entity_col->size <= entity_col->capacity) {
		return true;
	}

	int new_capacity = (int)(entity_col->size * ENLARGEMENT_FACTOR);
	Entity** new_array = calloc(new_capacity, sizeof(Entity*));
	if (!new_array) {
		Log_error("EntityCollection", "ensure_capacity: failed to allocate memory. new_capacity = %d", new_capacity);
		return false;
	}

	for (int i = 0; i < entity_col->size; ++i) {
		new_array[i] = entity_col->elements[i];
	}

	entity_col->capacity = new_capacity;
	free(entity_col->elements);
	entity_col->elements = new_array;
	return true;
}