#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <SDL2/SDL.h>
#include "Graphics.h"
#include "Game.h"
#include "Entity.h"
#include "Forward.h"


#define VP_DEFAULT_WIDTH  384
#define VP_DEFAULT_HEIGHT 240
#define VP_DEFAULT_CAMDIST_X 224
#define VP_DEFAULT_CAMDIST_Y 120
#define VP_DEFAULT_CAMSPEED_X 3.5
#define VP_DEFAULT_CAMSPEED_Y 3.5


typedef struct Viewport {
	World* world;
	Entity* locked_onto;
	Rectangle total;
	Rectangle visible;
	Vector2D camera_distance;
	Vector2D camera_speed;
} Viewport;


Viewport* Viewport_create();
void Viewport_destroy(Viewport* viewport);
void Viewport_draw_texture(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, SDL_Texture* texture);
void Viewport_draw_texture_extended(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, SDL_Texture* texture, SDL_RendererFlip flip);
void Viewport_draw_line_segment(Viewport* viewport, LineSegment* line_segment);
void Viewport_draw(Viewport* viewport);
void Viewport_update(Viewport* viewport);

#endif
