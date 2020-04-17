#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Graphics.h"
#include "Game.h"
#include "Entity.h"


/* TODO: add support for vertical viewports */

typedef struct Viewport {
	Game* game;
	Entity* locked_on;
	Rectangle total;
	Rectangle visible;
	Rectangle inner;
};

Viewport* Viewport_create();
void Viewport_destroy(Viewport* viewport);
void Viewport_draw_texture(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, Texture* texture);
void Viewport_draw(Viewport* viewport);

#endif