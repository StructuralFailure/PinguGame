#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#define CELL_WIDTH 16
#define CELL_HEIGHT 16

typedef struct Vector2D {
	float x;
	float y;
} Vector2D;

typedef struct Vector2DInt {
	int x;
	int y;
} Vector2DInt;

typedef struct Rectangle {
	Vector2D position;
	Vector2D size;
} Rectangle;

typedef struct Rectangle2DInt {
	Vector2DInt position;
	Vector2DInt size;
} RectangleInt;


bool Rectangle_overlap(Rectangle* rect_a, Rectangle* rect_b);



#endif