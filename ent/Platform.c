#include <stdlib.h>

#include <SDL2/SDL.h>
#include "Platform.h"
#include "../Entity.h"
#include "../Log.h"
#include "../Game.h"
#include "../World.h"
#include "../Graphics.h"
#include "../Viewport.h"
#include "../SDLHelper.h"
#include "../Util.h"


SDL_Texture* tex_platform;


Entity* EntityPlatform_create() 
{
	Log("EntityPlatform", "creating");
	if (!tex_platform) {
		tex_platform = SDLHelper_load_texture("assets/gfx/platform.bmp");
	}

	Entity* platform = Entity_create();
	if (!platform) {
		Log_error("EntityPlatform", "create: failed to create base entity.");
		return NULL;
	}

	platform->type = ET_PLATFORM;
	platform->is_solid = true;
	platform->rect = (Rectangle) {
		.position = {
			.x = 16,
			.y = 16
		},
		.size = {
			.x = 96,
			.y = 16
		}
	};

	EntityPlatformData* data = calloc(1, sizeof(EntityPlatformData));
	if (!data) {
		Log_error("EntityPlatform", "create: failed to allocate memory for data.");
		free(platform);
		return NULL;
	}

	data->moving_towards_destination = true;
	data->origin = platform->rect.position;
	data->destination = (Vector2D) { data->origin.x + 256, data->origin.y };
	data->speed = 3;
	platform->data = data;

	platform->add = EntityPlatform_add;
	platform->update = EntityPlatform_update;
	platform->draw = EntityPlatform_draw;
	platform->destroy = EntityPlatform_destroy;
	/* platform->collide performs inexact collision checking without information
	 * about the side that was collided with. this entity uses a different
	 * implementation that works similarly.
	 */


	Log("EntityPlatform", "created.");
	return platform;
}


void EntityPlatform_add(Entity* entity)
{

}


void EntityPlatform_update(Entity* entity)
{
	EntityPlatformData* data = (EntityPlatformData*)(entity->data);
	if (!data) {
		Log_error("EntityPlatform", "update: data == NULL, update not performed.");
		return;
	}


	Vector2D path = Vector2D_difference(data->destination, data->origin);
	Vector2D delta_pos = Vector2D_create_with_length(path, data->speed);
	if (!data->moving_towards_destination) {
		delta_pos.x = -delta_pos.x;
		delta_pos.y = -delta_pos.y;
	}
	Vector2D end_pos = Vector2D_sum(entity->rect.position, delta_pos);

	/* TODO: entity types to check are hard-coded. do this better in the future. */
	EntityType types_to_check[] = {
		ET_PLAYER,
		ET_ENEMY
	};

	Vector2D pos_original = entity->rect.position;

	World* world = entity->world;
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* other_entity = world->entities[i];

		if (!other_entity) {
			continue;
		}

		/* check whether the type of the entity is in the array of types to check. if not, skip. */
		bool is_in_types_array = false;
		for (int i_type = 0; i_type < sizeof(types_to_check) / sizeof(EntityType); ++i_type) {
			if (types_to_check[i] == other_entity->type) {
				is_in_types_array = true;
				break;
			}
		}
		if (!is_in_types_array) {
			continue;
		}

		CollidedWith cw = World_move_until_collision_with_flags(world, &(entity->rect), &delta_pos, CC_RECTANGLE, &(entity->rect));
		if (cw == CW_NOTHING) {
			continue;
		}


		/* TODO: kill other entity when crushed.
		 * right now, other entities get pushed into solid cells. this is no bueno.
		 * do this by moving it using the World_move function. if there is a collision, die.
		 *
		 * TODO: make entities stick to platform
		 */
		switch (cw) {
		case CW_LEFT:
			other_entity->rect.position.x = end_pos.x + entity->rect.size.x;
			break;
		case CW_TOP:
			other_entity->rect.position.y = end_pos.y + entity->rect.size.y;
			break;
		case CW_RIGHT:
			other_entity->rect.position.x = end_pos.x - other_entity->rect.size.x;
			break;
		case CW_BOTTOM:
			other_entity->rect.position.y = end_pos.y - other_entity->rect.size.y;
		default:
			;
		}



		entity->rect.position = pos_original;
	}
	entity->rect.position = end_pos; /* force movement, nothing can stop a platform. */

}


void EntityPlatform_draw(Entity* entity, Viewport* viewport)
{
	Viewport_draw_texture(viewport, NULL, &(entity->rect), tex_platform);
}


void EntityPlatform_destroy(Entity* entity) 
{
	Log("EntityPlatform", "destroying.");

	EntityPlatformData* data = (EntityPlatformData*)(entity->data);
	if (!data) {
		Log_error("EntityPlatform", "destroy: data is not supposed to be NULL.");
	}
	free(data);
	free(entity);

	Log("EntityPlatform", "destroyed.");
}