#include <stdlib.h>

#include <SDL2/SDL.h>
#include "../Viewport.h"
#include "../Entity.h"
#include "../Log.h"
#include "../Graphics.h"
#include "LineDrawer.h"

Entity* EntityLineDrawer_create()
{
	Entity* line_drawer = Entity_create();
	if (!line_drawer) {
		Log_error("EntityLineDrawer", "failed to create.");
		return NULL;
	}
	line_drawer->type = ET_LINE_DRAWER;

	EntityLineDrawerData* data = calloc(1, sizeof(EntityLineDrawerData));
	if (!data) {
		Log_error("EntityLineDrawer", "failed to allocate memory for data.");
		free(line_drawer);
		return NULL;
	}
	data->ls_a_is_finished = false;
	data->ls_b_is_finished = false;
	data->ls_a_is_current = true;
	data->mouse_held_down = false;
	line_drawer->data = data;

	line_drawer->update = EntityLineDrawer_update;
	line_drawer->draw = EntityLineDrawer_draw;
	line_drawer->destroy = EntityLineDrawer_destroy;

	Log("EntityLineDrawer", "created.");
	return line_drawer;
}


void EntityLineDrawer_update(Entity* entity) 
{
	EntityLineDrawerData* data = (EntityLineDrawerData*)(entity->data);

	/* TODO: replace with built-in IO functions
	 */
	int mouse_x;
	int mouse_y;
	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	LineSegment* ls_current;
	if (data->ls_a_is_current) {
		ls_current = &(data->ls_a);
	} else {
		ls_current = &(data->ls_b);
	}

	if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		if (!data->mouse_held_down) {
			data->mouse_held_down = true;

			if (data->ls_a_is_current) {
				data->ls_a_is_finished = false;
			} else {
				data->ls_b_is_finished = false;
			}
			
			ls_current->point_a = (Vector2D) {
				.x = mouse_x / RENDER_SCALE_FACTOR,
				.y = mouse_y / RENDER_SCALE_FACTOR
			};
		}
	} else if (data->mouse_held_down) {
		data->mouse_held_down = false;
		
		if (data->ls_a_is_current) {
			data->ls_a_is_finished = true;
		} else {
			data->ls_b_is_finished = true;
		}

		ls_current->point_b = (Vector2D) {
			.x = mouse_x / RENDER_SCALE_FACTOR,
			.y = mouse_y / RENDER_SCALE_FACTOR
		};

		data->ls_a_is_current = !(data->ls_a_is_current);

		Log(
			"EntityLineDrawer", 
			"line segment updated.\nls_a = [ [ x1 = %f | y1 = %f ] | [ x2 = %f | y2 = %f ] ]\nls_b = [ [ x1 = %f | y1 = %f ] | [ x2 = %f | y2 = %f ] ]",
			data->ls_a.point_a.x, data->ls_a.point_a.y, data->ls_a.point_b.x, data->ls_a.point_b.y,
			data->ls_b.point_a.x, data->ls_b.point_a.y, data->ls_b.point_b.x, data->ls_b.point_b.y
		);

		bool ls_intersect = LineSegment_intersect(&(data->ls_a), &(data->ls_b));
		if (ls_intersect) {
			Log("EntityLineDrawer", "line segments intersect.");
		} else {
			Log("EntityLineDrawer", "line segments do not intersect.");
		}
	}
}



void EntityLineDrawer_draw(Entity* entity, Viewport* viewport) 
{
	EntityLineDrawerData* data = (EntityLineDrawerData*)(entity->data);

	/* save render draw color */
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(sdl_renderer, &r, &g, &b, &a);

	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);

	if (data->ls_a_is_finished) {
		Viewport_draw_line_segment(viewport, &(data->ls_a));
	}
	if (data->ls_b_is_finished) {
		Viewport_draw_line_segment(viewport, &(data->ls_b));
	}

	/* restore render draw color */
	SDL_SetRenderDrawColor(sdl_renderer, r, g, b, a);
}


void EntityLineDrawer_destroy(Entity* entity)
{
	if (!entity) {
		Log("EntityLineDrawer", "NULL passed to destroy.");
		return;
	}

	EntityLineDrawerData* data = (EntityLineDrawerData*)(entity->data);
	if (data) {
		free(data);
	}

	free(entity);
	Log("EntityLineDrawer", "destroyed.");
}


Entity* EntityLineDrawer_deserialize(char* data)
{
	return EntityLineDrawer_create();
}