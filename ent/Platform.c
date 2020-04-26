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

EntityType affected_entity_types[] = {
	ET_PLAYER,
	ET_ENEMY
};


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

	data->origin = platform->rect.position;
	data->destination = (Vector2D) { data->origin.x, data->origin.y - 256 };
	data->speed = 1;
	data->moving_towards_destination = true;
	data->stickiness = DIR_UP;
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

	/*Log("EntityPlatform", "update: platform.y = %f", entity->rect.position.y);*/

	Vector2D path = Vector2D_difference(data->destination, data->origin);
	Vector2D delta_pos = Vector2D_create_with_length(path, data->speed);
	Vector2D end_pos = Vector2D_sum(entity->rect.position, delta_pos);

	float dist_total = Vector2D_distance(data->origin, data->destination);
	if (data->moving_towards_destination) {
		float dist_origin_to_end_pos = Vector2D_distance(data->origin, end_pos);
		if (dist_origin_to_end_pos > dist_total) {
			data->moving_towards_destination = false;
		}
	} else {
		float dist_destination_to_end_pos = Vector2D_distance(data->destination, end_pos);
		if (dist_destination_to_end_pos > dist_total) {
			data->moving_towards_destination = true;
		}
	}

	if (!data->moving_towards_destination) {
		delta_pos = (Vector2D) { -delta_pos.x, -delta_pos.y };
		end_pos = Vector2D_sum(entity->rect.position, delta_pos);
	}

	Vector2D pos_original = entity->rect.position;

	World* world = entity->world;
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
		Entity* other_entity = world->entities[i];

		if (!other_entity) {
			continue;
		}

		/* check whether the type of the entity is in the array of types to check. if not, skip. */
		bool is_in_types_array = false;
		for (int i_type = 0; i_type < sizeof(affected_entity_types) / sizeof(EntityType); ++i_type) {
			if (affected_entity_types[i] == other_entity->type) {
				is_in_types_array = true;
				break;
			}
		}
		if (!is_in_types_array) {
			continue;
		}

		CollidedWith cw = World_move_until_collision_with_flags(world, &(entity->rect), &delta_pos, CC_RECTANGLE, &(other_entity->rect));
		entity->rect.position = pos_original;
		if (cw == CW_NOTHING) {
			continue;
		}

		/* TODO: kill other entity when crushed.
		 * right now, other entities get pushed into solid cells. this is no bueno.
		 * do this by moving it using the World_move function. if there is a collision, die.
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
			break;
		default:
			;
		}
	}

	/* stickiness */
	if (data->stickiness & DIR_UP) {
		for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
			Entity* other_entity = world->entities[i];
			if (!other_entity) {
				continue;
			}

			if (other_entity->rect.position.y + other_entity->rect.size.y == entity->rect.position.y &&
			    other_entity->rect.position.x + other_entity->rect.size.x > entity->rect.position.x &&
			    other_entity->rect.position.x < entity->rect.position.x + entity->rect.size.x) {
			    	/* TODO: use World_move instead of forcing movement. */
				other_entity->rect.position.x += delta_pos.x;
				other_entity->rect.position.y += delta_pos.y;
			}
		}
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


bool EntityPlatform_serialize(Entity* entity, char* output) 
{
	Log_error("EntityPlatform", "serialize: not implemented.");
	return false;
}


Entity* EntityPlatform_deserialize(char* input)
{
	Log("EntityPlatform", "deserializing.");

	int type;
	int origin_x;
	int origin_y;
	int destination_x;
	int destination_y;
	float speed;
	if (sscanf(input, "%d %d %d %d %d %f", &type, &origin_x, &origin_y, &destination_x, &destination_y, &speed) != 6) {
		Log_error("EntityPlatform", "deserialize: invalid argument count.");
		return NULL;
	}

	Entity* platform = EntityPlatform_create();
	if (!platform) {
		Log_error("EntityPlatform", "deserialize: failed to create entity.");
		return NULL;
	}
	EntityPlatformData* data = (EntityPlatformData*)(platform->data);
	data->origin = (Vector2D) { origin_x, origin_y };
	platform->rect.position = data->origin;
	data->destination = (Vector2D) { destination_x, destination_y };
	((EntityPlatformData*)(platform->data))->speed = speed;
	Log("EntityPlatform", "deserialized.");
	return platform;
}