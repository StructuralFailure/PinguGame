#include <stdlib.h>

#include <SDL2/SDL.h>
#include "Item.h"
#include "../Entity.h"
#include "../Log.h"


Entity* EntityItem_create()
{
	Entity* item = Entity_create();
	if (!item) {
		Log_error("EntityItem", "failed to create entity.");
		return NULL;
	}
    item->type = ET_ITEM;

    EntityItemData* data = calloc(1, sizeof(EntityItemData));
    if (!data) {
        Log_error("EntityItem", "failed to allocate memory for entity data.");
        free(item);
        return NULL;
    }
	data->state = EIS_EMERGING;
	data->going_right = true;
	item->data = data;

	item->rect = (Rectangle) {
		.size = {
			.x = 16,
			.y = 16
		}
	};

	item->add = EntityItem_add;
	item->update = EntityItem_update;
	item->draw = EntityItem_draw;
	item->collide = EntityItem_collide;
	item->destroy = EntityItem_destroy;

	return item;
}


void EntityItem_add(Entity* entity) 
{

}


void EntityItem_update(Entity* entity)
{

}


void EntityItem_draw(Entity* entity, Viewport* viewport)
{

}


void EntityItem_collide(Entity* entity, Entity* entity_other)
{

}


void EntityItem_destroy(Entity* entity)
{
	EntityItemData* data = (EntityItemData*)(entity->data);
	free(data);
	free(entity);
}