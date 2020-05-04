#include <math.h>
#include <stdio.h>

#include "Graphics.h"
#include "Log.h"


#define UNITS_PER_MOVEMENT


Direction Rectangle_find_collision_sides(Rectangle* rect_moving, Rectangle* rect_static, Vector2D* delta, bool forwards)
{
	Vector2D unit_delta = Vector2D_create_with_length(*delta, 1);
	bool has_collided = false;

	if (forwards) {

	} else {
		Vector2D* rm_pos = &(rect_moving->position);
		Vector2D* rm_size = &(rect_moving->size);
		(void)rm_size;

		rm_pos->x += delta->x;
		rm_pos->y += delta->y;

		bool has_collided = false;
		bool is_colliding = false;

		do {
			if (Rectangle_overlap(rect_moving, rect_static)) {

			}

			if (is_colliding) {
				has_collided = true;

				rm_pos->x -= unit_delta.x;
				rm_pos->y -= unit_delta.y;
			}
		} while (is_colliding);
	}

	(void)has_collided;
	return 0;
}


bool Rectangle_overlap(Rectangle* rect_a, Rectangle* rect_b) 
{
	if (!rect_a || !rect_b) {
		Log_error("Graphics", "Rectangle_overlap: at least one argument is NULL.");
		return false;
	}

	return (
		rect_a->position.x + rect_a->size.x > rect_b->position.x &&
		rect_a->position.y + rect_a->size.y > rect_b->position.y &&
		rect_a->position.x < rect_b->position.x + rect_b->size.x &&
		rect_a->position.y < rect_b->position.y + rect_b->size.y
	);
}


void Rectangle_print(Rectangle* rect) 
{
	printf(
		"Rectangle\n{ x1 = %f | y1 = %f\n   || x2 = %f | y2 = %f }\n",
		rect->position.x, rect->position.y,
		rect->position.x + rect->size.x, rect->position.y + rect->size.y
	);
}


Vector2D Rectangle_center(Rectangle* rect)
{
	return (Vector2D) {
		.x = rect->position.x + (rect->size.x / 2),
		.y = rect->position.y + (rect->size.y / 2)
	};
}


#if 0
CollidedWith Rectangle_collision_face(Rectangle* rect_moving, Rectangle* rect_static, Vector2D* delta_pos)
{
	Vector2D rect_moving_original_pos = rect_moving->position;
	float delta_pos_length = Vector2D_length(*delta_pos);
	Vector2D delta_pos_unit = Vector2D_create_with_length(delta_pos, 1);
	rect_moving->position = Vector2D_sum(rect_moving->position, *delta_pos);

	bool has_collided = false;
	float distance_traveled_back = 0;
	while (Rectangle_overlap(rect_moving, rect_static)) {
		has_collided = true;
		rect_moving->position = Vector2D_difference(rect_moving->position, delta_pos_unit);
		dist_traveled_back += 1;

		if (distance_traveled_back >= delta_pos_length) {
			rect_moving->position = rect_moving_original_pos;
			break; /* we can't assume that there was no collision before the function
			        * was called, so we break out of the loop to make sure it terminates.
			        */
		}
	}


}
#endif


bool LineSegment_intersect(LineSegment* ls_a, LineSegment* ls_b)
{
	if (!ls_a || !ls_b) {
		Log_error("Graphics", "LineSegment_intersect: at least one argument is NULL.");
		return false;
	}

	Vector2D p = ls_a->point_a;
	Vector2D q = ls_b->point_a;

	Vector2D q_minus_p = Vector2D_difference(q, p);

	Vector2D r = Vector2D_difference(ls_a->point_b, ls_a->point_a);
	Vector2D s = Vector2D_difference(ls_b->point_b, ls_b->point_a);

	float r_cross_s = Vector2D_cross_product(r, s);
	if (r_cross_s == 0) {
		return false;
	}

	float t = Vector2D_cross_product(q_minus_p, s) / r_cross_s;
	if (t < 0 || t > 1) {
		return false;
	}

	float u = Vector2D_cross_product(q_minus_p, r) / r_cross_s;
	if (u < 0 || u > 1) {
		return false;
	}

	return true;
}


/* adds a vector a to a line segment.
 * i.e. moves a line segment without changing its length or direction.
 */
LineSegment LineSegment_add_vector(LineSegment ls, Vector2D vec)
{
	return (LineSegment) {
		.point_a = Vector2D_sum(ls.point_a, vec),
		.point_b = Vector2D_sum(ls.point_b, vec)
	};
}


float LineSegment_length(LineSegment* ls)
{
	return Vector2D_distance(ls->point_a, ls->point_b);
}


Vector2D Vector2D_difference(Vector2D vec_min, Vector2D vec_sub) 
{
	return (Vector2D) {
		.x = vec_min.x - vec_sub.x,
		.y = vec_min.y - vec_sub.y
	};
}


Vector2D Vector2D_sum(Vector2D vec_sum_a, Vector2D vec_sum_b) 
{
	return (Vector2D) {
		.x = vec_sum_a.x + vec_sum_b.x,
		.y = vec_sum_a.y + vec_sum_b.y
	};
}


Vector2D Vector2D_sum_array(Vector2D* vecs, int count)
{
	Vector2D result = { 0 };
	for (int i = 0; i < count; ++i) {
		result.x += vecs[i].x;
		result.y += vecs[i].y;
	}
	return result;
}


Vector2D Vector2D_product(Vector2D vec, float mult)
{
	return (Vector2D) {
		vec.x * mult,
		vec.y * mult
	};
}


float Vector2D_dot_product(Vector2D vec_a, Vector2D vec_b)
{
	return (vec_a.x * vec_b.x) + (vec_a.y * vec_b.y);
}


float Vector2D_cross_product(Vector2D vec_a, Vector2D vec_b)
{
	return (vec_a.x * vec_b.y) - (vec_a.y * vec_b.x);
}


Vector2D Vector2D_create_with_length(Vector2D vec, float length)
{
	float current_length = sqrt(vec.x * vec.x + vec.y * vec.y);
	if (current_length == 0) {
		return (Vector2D) { 0, 0 };
	}

	return (Vector2D) {
		.x = vec.x / current_length * length,
		.y = vec.y / current_length * length
	};
}


float Vector2D_length(Vector2D vec)
{
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}


float Vector2D_distance(Vector2D vec_a, Vector2D vec_b)
{
	return Vector2D_length(Vector2D_difference(vec_b, vec_a));
}


Vector2D Direction_vector(Direction dir, float length)
{
    Vector2D vector;
    if (dir & DIR_LEFT) {
        --vector.x;
    }
    if (dir & DIR_UP) {
        --vector.y;
    }
    if (dir & DIR_RIGHT) {
        ++vector.x;
    }
    if (dir & DIR_DOWN) {
        ++vector.y;
    }
    vector.x *= length;
    vector.y *= length;
    return vector;
}


Vector2D Vector_normal_clockwise(Vector2D vec)
{
	return (Vector2D) { -vec.y, vec.x };
}


Vector2D Vector_normal_counterclockwise(Vector2D vec)
{
	return (Vector2D) { vec.y, -vec.x };
}


float Vector2D_cos(Vector2D vec_a, Vector2D vec_b)
{
	float length_a = Vector2D_length(vec_a);
	float length_b = Vector2D_length(vec_b);
	return (Vector2D_dot_product(vec_a, vec_b) / (length_a * length_b));
}


bool Circle_overlap(Circle* circle_a, Circle* circle_b)
{
	return Vector2D_distance(circle_a->origin, circle_b->origin) < circle_a->radius + circle_b->radius;
}