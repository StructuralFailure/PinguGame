#ifndef ENTITY_TEXT_H
#define ENTITY_TEXT_H


typedef struct EntityTextData {
	char* text;
} EntityTextData;


Entity* EntityText_create(const char* text);
void EntityText_draw(Entity* entity);
void EntityText_remove(Entity* entity);

void EntityText_set_text(Entity* entity, const char* text);


#endif