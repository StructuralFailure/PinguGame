#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#define CELL_WIDTH 16
#define CELL_HEIGHT 16


typedef enum Direction {
	DIR_LEFT = 1,
	DIR_UP = 2,
	DIR_RIGHT = 4,
	DIR_DOWN = 8,
	__DIR_COUNT
} Direction;


typedef enum LineSegmentRelation {
	VI_NONE,
	VI_COLLINEAR_OVERLAPPING,
	VI_COLLINEAR_DISJOINT,
	VI_PARALLEL,
	VI_INTERSECT,
	VI_NO_INTERSECT
} LineSegmentRelation;


typedef struct Vector2D {
	float x;
	float y;
} Vector2D;


typedef struct Vector2DInt {
	int x;
	int y;
} Vector2DInt;


typedef struct LineSegment {
	Vector2D point_a;
	Vector2D point_b;
} LineSegment;


typedef struct Rectangle {
	Vector2D position;
	Vector2D size;
} Rectangle;


typedef struct Rectangle2DInt {
	Vector2DInt position;
	Vector2DInt size;
} RectangleInt;


bool Rectangle_overlap(Rectangle* rect_a, Rectangle* rect_b);

bool        LineSegment_intersect(LineSegment* line_a, LineSegment* line_b);
LineSegment LineSegment_add_vector(LineSegment ls, Vector2D vec);

Vector2D Vector2D_difference(Vector2D vec_min, Vector2D vec_sub);
Vector2D Vector2D_sum(Vector2D vec_sum_a, Vector2D vec_sum_b);
Vector2D Vector2D_sum_array(Vector2D* vecs, int count);
float    Vector2D_cross_product(Vector2D vec_a, Vector2D vec_b);


#endif
