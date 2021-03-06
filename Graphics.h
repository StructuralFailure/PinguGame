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


typedef struct RectangleInt {
	Vector2DInt position;
	Vector2DInt size;
} RectangleInt;


typedef struct Circle {
	Vector2D origin;
	float radius;
} Circle;


bool     Rectangle_overlap(Rectangle* rect_a, Rectangle* rect_b);
Vector2D Rectangle_center(Rectangle* rect);
void     Rectangle_print(Rectangle* rect);
//CollidedWith Rectangle_collision_face(Rectangle* rect_moving, Rectangle* rect_static, Vector2D* delta_pos);

bool        LineSegment_intersect(LineSegment* line_a, LineSegment* line_b);
LineSegment LineSegment_add_vector(LineSegment ls, Vector2D vec);
float       LineSegment_length(LineSegment* ls);

Vector2D Vector2D_difference(Vector2D vec_min, Vector2D vec_sub);
Vector2D Vector2D_sum(Vector2D vec_sum_a, Vector2D vec_sum_b);
Vector2D Vector2D_sum_array(Vector2D* vecs, int count);
Vector2D Vector2D_product(Vector2D vec, float mult);
float    Vector2D_dot_product(Vector2D vec_a, Vector2D vec_b);
float    Vector2D_cross_product(Vector2D vec_a, Vector2D vec_b);
Vector2D Vector2D_create_with_length(Vector2D vec, float length);
float    Vector2D_length(Vector2D vec);
float    Vector2D_distance(Vector2D vec_a, Vector2D vec_b);
Vector2D Vector2D_normal_clockwise(Vector2D vec);
Vector2D Vector2D_normal_counterclockwise(Vector2D vec);
float    Vector2D_cos(Vector2D vec_a, Vector2D vec_b);

bool     Circle_overlap(Circle* circle_a, Circle* circle_b);

Vector2D Direction_unit_vector(Direction);



#endif
