#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <SDL2/SDL.h>
#include "Graphics.h"
#include "Game.h"
#include "Entity.h"


/* TODO: add support for vertical viewports */

typedef struct Viewport {
	Game* game;
	Entity* locked_onto;
	Rectangle total;
	Rectangle visible;
	Vector2D camera_distance;
	Vector2D camera_speed;
} Viewport;

Viewport* Viewport_create();
void Viewport_destroy(Viewport* viewport);
void Viewport_draw_texture(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, SDL_Texture* texture);
void Viewport_draw(Viewport* viewport);
void Viewport_update(Viewport* viewport);

#endif
