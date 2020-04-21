#include "Graphics.h"
#include "Log.h"


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


float Vector2D_cross_product(Vector2D vec_a, Vector2D vec_b)
{
	return (vec_a.x * vec_b.y) - (vec_a.y * vec_b.x);
}