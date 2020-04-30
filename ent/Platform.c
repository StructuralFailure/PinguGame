#include <stdlib.h>
#include <math.h>

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


Vector2D get_coordinates_elliptical_movement(Entity* entity);
Vector2D get_coordinates_linear_movement(Entity* entity);


Entity* EntityPlatform_create() 
{
	Log("EntityPlatform", "creating.");
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

	data->stickiness = DIR_UP;
	data->origin = platform->rect.position;
	data->movement_type = EPMT_LINEAR;
	data->movement_duration = 300;
	data->ticks = 0;
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
    ENTITY_DATA_ASSERT(Platform);

	Vector2D pos_original = entity->rect.position;
	Vector2D end_pos = entity->rect.position;

	switch (data->movement_type) {
	case EPMT_LINEAR:
		end_pos = get_coordinates_linear_movement(entity);
		break;
	case EPMT_ELLIPTICAL:
		end_pos = get_coordinates_elliptical_movement(entity);
		break;
	default:
		; /* not implemented. do nothing. */
	}

	end_pos.x = round(end_pos.x);
	end_pos.y = round(end_pos.y);

	Vector2D delta_pos = Vector2D_difference(end_pos, entity->rect.position);
	World* world = entity->world;

	/* stickiness 
	 * putting glue on the platform is only implemented for its top.
	 */
	if (data->stickiness & DIR_UP) {
		for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {
			Entity* other_entity = world->entities[i];
			if (!other_entity || other_entity == entity || other_entity->type != ET_PLAYER) {
				continue;
			}

			if (other_entity->rect.position.y + other_entity->rect.size.y == entity->rect.position.y &&
			    other_entity->rect.position.x + other_entity->rect.size.x > entity->rect.position.x &&
			    other_entity->rect.position.x < entity->rect.position.x + entity->rect.size.x) {
			    	/* TODO: use World_move instead of forcing movement. */

				/* TODO: fix this ugly hack. */
				/* all solid entities except for the platform the player is sticking to should
				 * be considered. instead of being sensible and finally implementing a proper
				 * collision engine, i just move the platform out of the way and put it back
				 * afterwards as if nothing happened.
				 */ 
				Vector2D platform_position = entity->rect.position;
				entity->rect.position = (Vector2D) { -1000, -1000 };

				Vector2D other_entity_delta_pos = {
					.x = delta_pos.x,
					.y = (end_pos.y - other_entity->rect.size.y) - other_entity->rect.position.y
				};
				World_move(world, other_entity, &other_entity_delta_pos);

				entity->rect.position = platform_position;

			    	/*other_entity->rect.position.y = end_pos.y - other_entity->rect.size.y;
				other_entity->rect.position.x += delta_pos.x;*/
			}
		}
	}


	/* push other entities around if they are hit. */
	for (int i = 0; i < MAX_ENTITY_COUNT; ++i) {

		Entity* other_entity = world->entities[i];

		if (!other_entity) {
			continue;
		}

		/* check whether the type of the entity is in the array of types to check. if not, skip. */
		bool is_in_types_array = false;
		for (int i_type = 0; i_type < sizeof(affected_entity_types) / sizeof(EntityType); ++i_type) {
			if (affected_entity_types[i_type] == other_entity->type) {
				is_in_types_array = true;
				break;
			}
		}
		if (!is_in_types_array) {
			continue;
		}

		/* we have to move every entity around separately in order to know which side of it
		 * was hit and whether their displacement is possible. reset the position afterwards.
		 */
		CollidedWith cw = World_move_until_collision_with_flags(world, &(entity->rect), &delta_pos, CC_RECTANGLE, &(other_entity->rect));
		entity->rect.position = pos_original;

		if (cw == CW_NOTHING) {
			continue;
		}

		/* TODO: kill other entity when crushed.
		 * right now, other entities get pushed into solid cells. this is no bueno.
		 * do this by moving it using the World_move function. if there is a collision, die.
		 */

		/* careful: calling another entity's function means it's not guaranteed for it to still
		 *          be alive after execution. its reference cannot be used any longer.
		 */

		Vector2D other_entity_end_pos = other_entity->rect.position;

		switch (cw) {
		case CW_LEFT:
			other_entity_end_pos.x = end_pos.x + entity->rect.size.x;
			break;
		case CW_TOP:
			other_entity_end_pos.y = end_pos.y + entity->rect.size.y;
			break;
		case CW_RIGHT:
			other_entity_end_pos.x = end_pos.x - other_entity->rect.size.x;
			break;
		case CW_BOTTOM:
			other_entity_end_pos.y = end_pos.y - other_entity->rect.size.y;
			break;
		default:
			;
		}

		Vector2D other_entity_delta_pos = Vector2D_difference(
			other_entity_end_pos, other_entity->rect.position
		);

		if (World_move(world, other_entity, &other_entity_delta_pos) != CW_NOTHING) {
			/* crushed. */
			Log("Platform", "update: other entity crushed.");
			if (other_entity->message) {
				other_entity->message(other_entity, entity, EMT_I_DAMAGED_YOU, NULL);
			}
		}
	}

	++(data->ticks);
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
	int movement_type;
	int origin_x;
	int origin_y;
	int movement_duration;
	int ticks_displacement;

	int bytes_read;
	//Log("EntityPlatform", "deserialize: input: %s", input);
	if (sscanf(input, "%d %d %d %d %d %d %n",
		   &type, &movement_type, &movement_duration,
		   &origin_x, &origin_y, &ticks_displacement, &bytes_read) != 6) {
		Log_error("EntityPlatform", "deserialize: invalid argument count (expected 1+5+n).");
        return NULL;
    }
    input += bytes_read;

    //Log("EntityPlatform", "read the correct amount of arguments.");
    //Log("EntityPlatform", "x = %d | y = %d | movement_duration = %d | ticks_displacement = %d", origin_x, origin_y, movement_duration, ticks_displacement);

	Entity* platform = EntityPlatform_create();
	if (!platform) {
		Log_error("EntityPlatform", "deserialize: failed to create entity.");
		return NULL;
	}
	platform->type = ET_PLATFORM;

	EntityPlatformData* data = (EntityPlatformData*)(platform->data);
	data->movement_type = movement_type;
	data->origin = (Vector2D) {
		.x = origin_x,
		.y = origin_y
	};
	data->movement_duration = movement_duration;
	data->ticks_displacement = ticks_displacement;

	Log("EntityPlatform", "deserialize: ticks_displacement = %f", ticks_displacement);

	bool success = true;
	switch (movement_type) {
	case EPMT_LINEAR: {
		int destination_x;
		int destination_y;
		if (sscanf(input, "%d %d%n", 
		           &destination_x, &destination_y, &bytes_read) != 2) {
			Log_error("EntityPlatform", "deserialize: linear movement: invalid argument count (expected 2.)");
			success = false;
		} else {
			data->lm.destination = (Vector2D) {
				.x = destination_x,
				.y = destination_y
			};
		}
	}	break;
	case EPMT_ELLIPTICAL: {
	    Log("EntityPlatform", "deserialize: doing elliptical movement.");
		float radius_x;
		float radius_y;
		int reverse;
		if (sscanf(input, "%f %f %d%n", &radius_x, &radius_y, &reverse, &bytes_read) != 3) {
			Log_error("EntityPlatform", "deserialize: elliptical movement: invalid argument count (expected 3).");
			success = false;
		} else {
			data->em.radius = (Vector2D) { radius_x, radius_y };
			data->em.reverse = reverse;
		}
	}	break;
	default:
		success = false;
	}

	if (!success) {
		Log_error("EntityPlatform", "deserialize: failed. movement_type = %d", movement_type);
		EntityPlatform_destroy(platform);
		return NULL;
	}

	return platform;
}


Vector2D get_coordinates_elliptical_movement(Entity* entity) {
	EntityPlatformData* data = (EntityPlatformData*)(entity->data);

	float radians_per_tick = 2 * M_PI / data->movement_duration;

	float angle = radians_per_tick * (data->ticks + data->ticks_displacement);
	if (data->em.reverse) {
	    angle *= -1;
	}

	Vector2D path_from_origin = {
		.x = data->em.radius.x * cos(angle),
		.y = data->em.radius.y * sin(angle)
	};

	//Log("Platform", "path_from_origin = { %f | %f }", path_from_origin.x, path_from_origin.y);

	//Log("EntityPlatform", "get_coordinates_circular_movement: radians_per_tick = %f", radians_per_tick);
	//Log("EntityPlatform", "path_from_origin = { %f | %f }", path_from_origin.x, path_from_origin.y);
	//Log("EntityPlatform", "origin = { %f | %f )", data->origin.x, data->origin.y);

	Vector2D coords = {
		.x = data->origin.x + path_from_origin.x - entity->rect.size.x / 2,
		.y = data->origin.y + path_from_origin.y - entity->rect.size.y / 2
	};

	//Log("Platform", "coordinates = { %f | %f }", coords.x, coords.y);
	return coords;
}


/* moves back and forth */
Vector2D get_coordinates_linear_movement(Entity* entity) {
	EntityPlatformData* data = (EntityPlatformData*)(entity->data);

	bool moving_towards_destination = (((data->ticks + data->ticks_displacement) / data->movement_duration) % 2 == 0);

	Vector2D current_origin;
	Vector2D current_destination;
	if (moving_towards_destination) {
		current_origin = data->origin;
		current_destination = data->lm.destination;
	} else {
		current_origin = data->lm.destination;
		current_destination = data->origin;
	}

	Vector2D origin_to_destination = Vector2D_difference(current_destination, current_origin);
	float part_movement_completed = (data->ticks % data->movement_duration) / (float)(data->movement_duration);
	float distance = Vector2D_length(origin_to_destination) * part_movement_completed;

	Vector2D relative_movement = Vector2D_create_with_length(origin_to_destination, distance);

	return (Vector2D) {
		.x = current_origin.x + relative_movement.x,
		.y = current_origin.y + relative_movement.y
	};
}