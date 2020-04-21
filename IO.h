#ifndef IO_H
#define IO_H

#include "Graphics.h"


/* bitfield */
typedef enum IOMouseButton {
	IO_MOUSE_LEFT  = 1,
	IO_MOUSE_RIGHT = 2
} IOMouseButton;


IOMouseButton IO_get_mouse_status(Vector2DInt* position);


#endif