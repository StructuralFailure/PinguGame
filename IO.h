#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "Graphics.h"


/* bitfield */
typedef enum IOMouseButton {
	IO_MOUSE_LEFT  = 1,
	IO_MOUSE_RIGHT = 2,
	__IO_MOUSE_BUTTON_COUNT
} IOMouseButton;


typedef enum IOKey {
	IO_KEY_LEFT  = 0,
	IO_KEY_UP    = 1,
	IO_KEY_RIGHT = 2,
	IO_KEY_DOWN  = 3,
	__IO_KEY_COUNT
} IOKey;


void IO_init(void);
void IO_update_keys(void);
void IO_update_mouse(void);
void IO_update(void);

bool IO_key_down(IOKey key);
bool IO_key_pressed(IOKey key);
bool IO_key_depressed(IOKey key);


void          IO_update_mouse(void);
Vector2DInt   IO_mouse_position(void);
bool          IO_mouse_down(IOMouseButton button);
bool          IO_mouse_pressed(IOMouseButton button);
bool          IO_mouse_depressed(IOMouseButton button);


#endif