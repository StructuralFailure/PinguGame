#ifndef ENTITY_TEXT_H
#define ENTITY_TEXT_H

#include "../Entity.h"
#include "../Viewport.h"


typedef struct EntityTextData {
	char* text;
} EntityTextData;


Entity* EntityText_create(char* text);
void EntityText_draw(Entity* entity, Viewport* viewport);
void EntityText_remove(Entity* entity);

void EntityText_set_text(Entity* entity, char* text);


#endif
