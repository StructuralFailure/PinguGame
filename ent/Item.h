#ifndef ENTITY_ITEM_H
#define ENTITY_ITEM_H

#include <stdbool.h>
#include "../Entity.h"


typedef enum EntityItemType {
	EIT_FISH
} EntityItemType;


typedef enum EntityItemState {
	EIS_EMERGING,
	EIS_NORMAL,
	EIS_DISAPPEARING
} EntityItemState;


typedef struct EntityItemData {
	EntityItemType type;
	EntityItemState state;
	bool going_right;
} EntityItemData;


Entity* EntityItem_create();
void EntityItem_add(Entity* entity);
void EntityItem_update(Entity* entity);
void EntityItem_draw(Entity* entity, Viewport* viewport);
void EntityItem_collide(Entity* entity, Entity* entity_other);
void EntityItem_destroy(Entity* entity);


#endif