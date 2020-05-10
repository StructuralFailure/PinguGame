//
// Created by fabian on 10.05.20.
//

#ifndef ENTITY_COLLECTION_H
#define ENTITY_COLLECTION_H

#include <stdbool.h>
#include "Entity.h"


typedef struct EntityCollection {
	int size;
	int capacity;
	Entity** elements;
} EntityCollection;


bool    EntityCollection_init(EntityCollection*, int initial_capacity);
void    EntityCollection_deinit(EntityCollection*);

bool    EntityCollection_add(EntityCollection*, Entity*);
bool    EntityCollection_remove(EntityCollection*, Entity*);
bool    EntityCollection_remove_at(EntityCollection*, int index);
int     EntityCollection_find(EntityCollection*, Entity*);


#endif