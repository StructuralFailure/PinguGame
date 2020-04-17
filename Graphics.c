#include "Graphics.h"


bool Rectangle_overlap(Rectangle* rect_a, Rectangle* rect_b) 
{
	return (
		rect_a->position.x + rect_a->size.x > rect_b->position.x &&
		rect_a->position.y + rect_a->size.y > rect_b->position.y &&
		rect_a->position.x < rect_b->position.x + rect_b->size.x &&
		rect_a->position.y < rect_b->position.y + rect_b->size.y
	);
}
