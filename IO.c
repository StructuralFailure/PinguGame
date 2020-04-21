#include <SDL2/SDL.h>
#include "IO.h"
#include "Graphics.h"
#include "SDLHelper.h"


IOMouseButton IO_get_mouse_status(Vector2DInt* position)
{
	int mouse_x;
	int mouse_y;
	Uint32 sdl_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	*position = (Vector2DInt) {
		.x = mouse_x / RENDER_SCALE_FACTOR,
		.y = mouse_y / RENDER_SCALE_FACTOR
	};

	IOMouseButton io_buttons = 0;

	if (sdl_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		io_buttons |= IO_MOUSE_LEFT;
	} else if (sdl_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		io_buttons |= IO_MOUSE_RIGHT;
	}

	return io_buttons;
}