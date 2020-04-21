#include <stdlib.h>
#include <math.h>


#include <SDL2/SDL.h>
#include "SDLHelper.h"
#include "Viewport.h"
#include "Game.h"
#include "Entity.h"
#include "Level.h"
#include "Util.h"
#include "Log.h"
#include "World.h"


Viewport* Viewport_create() 
{
	Viewport* viewport = calloc(1, sizeof(Viewport));
	if (!viewport) {
		Log_error("Viewport", "failed to allocate memory.");
		return NULL;
	}
	viewport->total = (Rectangle) {
		.position = {
			.x = 0,
			.y = 0 
		},
		.size = {
			.x = VP_DEFAULT_WIDTH,
			.y = VP_DEFAULT_HEIGHT
		}
	};
	viewport->visible = (Rectangle) {
		.size = {
			.x = VP_DEFAULT_WIDTH,
			.y = VP_DEFAULT_HEIGHT
		}
	};
	viewport->camera_distance = (Vector2D) {
		.x = VP_DEFAULT_CAMDIST_X,
		.y = VP_DEFAULT_CAMDIST_Y
	};
	viewport->camera_speed = (Vector2D) {
		.x = VP_DEFAULT_CAMSPEED_X,
		.y = VP_DEFAULT_CAMSPEED_Y
	};
	Log("Viewport", "created.");
	return viewport;
}


void Viewport_draw_texture(Viewport* viewport, Rectangle* rect_source, Rectangle* rect_dest, SDL_Texture* texture) 
{
	SDL_Rect sdl_rect_source;
	SDL_Rect* sdl_rect_source_ptr = NULL;
	if (rect_source) {
		sdl_rect_source = SDLHelper_get_sdl_rect(rect_source);
		sdl_rect_source_ptr = &sdl_rect_source;
	}

	SDL_Rect sdl_rect_dest = SDLHelper_get_sdl_rect(rect_dest);

	sdl_rect_dest.x += viewport->total.position.x;
	sdl_rect_dest.y += viewport->total.position.y;
	sdl_rect_dest.x -= viewport->visible.position.x;
	sdl_rect_dest.y -= viewport->visible.position.y;

	SDL_RenderCopy(sdl_renderer, texture, sdl_rect_source_ptr, &sdl_rect_dest);
}


void Viewport_draw_line_segment(Viewport* viewport, LineSegment* line_segment)
{
	Vector2D point_a = 
		Vector2D_difference(
			Vector2D_sum(
				line_segment->point_a, 
				viewport->total.position
			),
			viewport->visible.position
		);

	Vector2D point_b =
		Vector2D_difference(
			Vector2D_sum(
				line_segment->point_b, 
				viewport->total.position
			),
			viewport->visible.position
		);

	SDL_RenderDrawLine(sdl_renderer, point_a.x, point_a.y, point_b.x, point_b.y);
}


void Viewport_draw(Viewport* viewport) 
{
	World* world = viewport->world;

	Rectangle rect_dest = {
		.size = {
			.x = CM_CELL_WIDTH,
			.y = CM_CELL_HEIGHT
		}
	};

	Rectangle rect_source = {
		.position = {
			.y = 0
		},
		.size = {
			.x = CM_CELL_WIDTH,
			.y = CM_CELL_HEIGHT
		}
	};

	/* TODO:
	 * do not attempt to draw tiles outside the visible part
	 * of the map. same goes for entities.
         */
	for (int y = 0; y < world->level->height; ++y) {
		for (int x = 0; x < world->level->width; ++x) {
			LevelCellTypeProperties* ct_properties = Level_get_cell_type_properties(world->level, x, y);
			SDL_Texture* cell_texture = ct_properties->texture;

			if (ct_properties->type == LCT_SOLID_BLOCK) {
				/* change texture of solid block depending on its neighbors */

				int tileset_index = 0;
				tileset_index |= 1 * Level_is_solid(world->level, x - 1, y);
				tileset_index |= 2 * Level_is_solid(world->level, x, y - 1);
				tileset_index |= 4 * Level_is_solid(world->level, x + 1, y);
				tileset_index |= 8 * Level_is_solid(world->level, x, y + 1);;

				rect_source.position.x = tileset_index * CM_CELL_WIDTH;
			} else {
				rect_source.position.x = 0;
			}

			rect_dest.position = (Vector2D) {
				.x = x * CM_CELL_WIDTH,
				.y = y * CM_CELL_HEIGHT
			};

			Viewport_draw_texture(viewport, &rect_source, &rect_dest, cell_texture);
		}
	}

	/* call Entity objects' render functions */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* ent = world->entities[i];
		if (ent && ent->draw) {
			ent->draw(ent, viewport);
		}		
	}
}


void Viewport_update(Viewport* viewport)
{
	Entity* entity = viewport->locked_onto;
	if (!entity) {
		return;
	}

	Direction entity_direction = 0xFF; /* check all directions by default */
	if (entity->get_direction) {
		entity_direction = entity->get_direction();
	}

	Rectangle* rect_entity = &(entity->rect);

	Vector2D pos_entity_center = {
		.x = round(rect_entity->position.x + (rect_entity->size.x / 2)),
		.y = round(rect_entity->position.y + (rect_entity->size.y / 2))
	};

	/* TODO:
	 * there's gotta be a less verbose way to achieve this.
	 */
	if (entity_direction & DIR_RIGHT) {
		float cam_endpoint_x = pos_entity_center.x + viewport->camera_distance.x;
		if (cam_endpoint_x > viewport->visible.position.x + viewport->visible.size.x) {
			float viewport_x_new = min(
				pos_entity_center.x + viewport->camera_distance.x - viewport->visible.size.x,
				viewport->total.position.x + viewport->total.size.x - viewport->visible.size.x
			);
			float viewport_x_diff = viewport_x_new - viewport->visible.position.x;
			viewport->visible.position.x += min(viewport->camera_speed.x, viewport_x_diff);

		}
	} else if (entity_direction & DIR_LEFT) {
		float cam_endpoint_x = pos_entity_center.x - viewport->camera_distance.x;
		if (cam_endpoint_x < viewport->visible.position.x) {
			float viewport_x_new = max(
				pos_entity_center.x - viewport->camera_distance.x,
				viewport->total.position.x
			);
			float viewport_x_diff = viewport->visible.position.x - viewport_x_new;
			viewport->visible.position.x -= min(viewport->camera_speed.x, viewport_x_diff);
		}
	} else if (entity_direction & DIR_UP) {
		float cam_endpoint_y = pos_entity_center.y + viewport->camera_distance.y;
		if (cam_endpoint_y > viewport->visible.position.y + viewport->visible.size.y) {
			float viewport_y_new = min(
				pos_entity_center.y + viewport->camera_distance.y - viewport->visible.size.y,
				viewport->total.position.y + viewport->total.size.y - viewport->visible.size.y
			);
			float viewport_y_diff = viewport_y_new - viewport->visible.position.y;
			viewport->visible.position.y += min(viewport->camera_speed.y, viewport_y_diff);
		}
	} else if (entity_direction & DIR_DOWN) {
		float cam_endpoint_y = pos_entity_center.y - viewport->camera_distance.y;
		if (cam_endpoint_y < viewport->visible.position.y) {
			float viewport_y_new = max(
				pos_entity_center.y - viewport->camera_distance.y,
				viewport->total.position.y
			);
			float viewport_y_diff = viewport->visible.position.y - viewport_y_new;
			viewport->visible.position.y -= min(viewport->camera_speed.y, viewport_y_diff);
		}
	}



}


void Viewport_destroy(Viewport* viewport)
{
	free(viewport);
	Log("Viewport", "destroyed.");
}
