//
// Created by fabian on 12.05.20.
//

#ifndef ENTITY_TEXT_H
#define ENTITY_TEXT_H

#include "../Entity.h"
#include "../Tileset.h"


typedef enum EntityTextFontType {
	ETFT_SMALL,
	ETFT_LARGE
} EntityTextFontType;


typedef struct EntityTextData {

	char* text;
	EntityTextFontType font_type;
	int character_padding;
	Vector2D position;

	/* do not modify. */
	Animation** anims;

} EntityTextData;


Entity* EntityText_create();
void    EntityText_destroy(Entity*);

void EntityText_update(Entity*);
void EntityText_draw(Entity*);


#endif
