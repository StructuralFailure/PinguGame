#include <SDL2/SDL.h>
#include "IO.h"
#include "Graphics.h"
#include "SDLHelper.h"
#include "Log.h"


#define ASSERT_KEY_IS_VALID(KEY) {         \
	if (KEY < 0 || KEY > __IO_KEY_COUNT) { \
        Log_error("IO", "invalid key.");   \
        return false;                      \
    }                                      \
}


static const Uint8 io_key_to_sdl_scancode_map[__IO_KEY_COUNT] = {
		[IO_KEY_LEFT]  = SDL_SCANCODE_LEFT,
		[IO_KEY_UP]    = SDL_SCANCODE_UP,
		[IO_KEY_RIGHT] = SDL_SCANCODE_RIGHT,
		[IO_KEY_DOWN]  = SDL_SCANCODE_DOWN
};
static bool current_key_status[__IO_KEY_COUNT];
static bool previous_key_status[__IO_KEY_COUNT];
static const Uint8* sdl_keyboard_state;


static const Uint8 io_mouse_button_to_sdl_button_map[__IO_MOUSE_BUTTON_COUNT] = {
		[IO_MOUSE_LEFT]  = SDL_BUTTON_LEFT,
		[IO_MOUSE_RIGHT] = SDL_BUTTON_RIGHT
};
static bool current_mouse_status[__IO_MOUSE_BUTTON_COUNT];
static bool previous_mouse_status[__IO_MOUSE_BUTTON_COUNT];


void IO_init(void)
{
	if (sdl_keyboard_state) {
		return;
	}

	sdl_keyboard_state = SDL_GetKeyboardState(NULL);
	Log("IO", "initialized.");
}


void IO_update(void)
{
	IO_update_keys();
	IO_update_mouse();
}


void IO_update_keys(void)
{
	for (int i = 0; i < __IO_KEY_COUNT; ++i) {
		previous_key_status[i] = current_key_status[i];
		current_key_status[i] = sdl_keyboard_state[
			io_key_to_sdl_scancode_map[i]
		];
	}
}


void IO_update_mouse(void)
{
	Uint8 mouse_state = SDL_GetMouseState(NULL, NULL);
	for (int i = 0; i < __IO_MOUSE_BUTTON_COUNT; ++i) {
		previous_mouse_status[i] = current_mouse_status[i];
		current_mouse_status[i] = mouse_state & SDL_BUTTON(io_mouse_button_to_sdl_button_map[i]);
	}
}


inline bool IO_key_down(IOKey key)
{
	//ASSERT_KEY_IS_VALID(key);
	return current_key_status[key];
}


inline bool IO_key_pressed(IOKey key)
{
	//ASSERT_KEY_IS_VALID(key);
	return current_key_status[key] && !previous_key_status[key];
}


inline bool IO_key_depressed(IOKey key)
{
	//ASSERT_KEY_IS_VALID(key);
	return !current_key_status[key] && previous_key_status[key];
}


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


bool IO_mouse_down(IOMouseButton button)
{
	return current_mouse_status[button];
}


bool IO_mouse_pressed(IOMouseButton button)
{
	/* TODO: add support for multiple buttons at once. */
	return current_mouse_status[button] && !previous_mouse_status[button];
}


bool IO_mouse_depressed(IOMouseButton button)
{
	return !current_mouse_status[button] && previous_mouse_status[button];
}


Vector2DInt IO_mouse_position(void)
{
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	return	(Vector2DInt) {
		.x = (int)(x / RENDER_SCALE_FACTOR),
		.y = (int)(y / RENDER_SCALE_FACTOR)
	};
}