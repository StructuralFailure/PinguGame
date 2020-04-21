#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include "../Game.h"
#include "../Util.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "../Graphics.h"
#include "../Log.h"
#include "Player.h"


#define TEXT_SCORE_SIZE 32
#define MAX_JUMP_CHARGE 7
#define JUMP_SPEED      -3.5

#define SPEED_CLIMBING_X  0.7
#define SPEED_CLIMBING_Y  0.7
/* TODO: add a bunch more defines to get rid of magic numbers. */
#define GRAVITY           0.15 
#define MAX_FALLING_SPEED 3


char text_score[32];
SDL_Texture* tex_player_left;
SDL_Texture* tex_player_right;


Entity* EntityPlayer_create()
{
	if (!tex_player_left) {
		tex_player_left = SDLHelper_load_texture("assets/gfx/player_left.bmp");
	}
	if (!tex_player_right) {
		tex_player_right = SDLHelper_load_texture("assets/gfx/player_right.bmp");
	}

	Entity* player = Entity_create();
	if (!player) {
		return NULL;
	}

	EntityPlayerData* data = calloc(1, sizeof(EntityPlayerData));
	if (!data) {
		free(player);
		return NULL;
	}
	player->data = data;

	player->type = ET_PLAYER;
	player->rect = (Rectangle) { 
		.position = {
			.x = 32,
			.y = 32
		},
		.size = {
			.x = 16,
			.y = 16
		}
	};

	player->add = EntityPlayer_add;
	player->update = EntityPlayer_update;
	player->draw = EntityPlayer_draw;
	player->destroy = EntityPlayer_destroy;
	player->collide = EntityPlayer_collide;
	player->get_direction = EntityPlayer_viewport_get_direction;


	data->state = EPS_DEFAULT;
	data->powerup_level = 123;
	data->health = 456;
	data->velocity = (Vector2D) {
		.x = 0,
		.y = 0
	};
	data->facing = EPF_RIGHT;
	data->jump_charge_counter = 0;

	return player;
}


void EntityPlayer_add(Entity* entity)
{
	printf("[EntityPlayer] added\n");
	/* steal viewport */
	entity->world->viewport->locked_onto = entity;
	/* create text entity */
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);
	data->entity_text = EntityText_create(text_score);
	data->entity_text_text = calloc(1, TEXT_SCORE_SIZE);
	EntityText_set_text(data->entity_text, data->entity_text_text);
	World_add_entity(entity->world, data->entity_text);
}


void EntityPlayer_update(Entity* entity)
{
	EntityPlayerData* data = (EntityPlayerData*)entity->data;
	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	/* TODO: this is getting messy as fuck.
	 * i need to find a better solution to manage state.
	 */

	if (data->state != EPS_CLIMBING) {
		/* TODO: put this into a separate function to maintain readability. */

		/* player is not climbing.
		 * let him move around the map freely and perform collision detectio etc.
		 */

		/* gravity */
		data->velocity.y = min(data->velocity.y + 0.15, 4.5);

		bool moving_horizontally = false;
		if (keystate[SDL_SCANCODE_LEFT]) {
			data->velocity.x = max(data->velocity.x - 0.3, -2.5);
			if (data->velocity.x < 0) {
				data->facing = EPF_LEFT;
			}
			moving_horizontally = true;
		} 
		if (keystate[SDL_SCANCODE_RIGHT]) {
			data->velocity.x = min(data->velocity.x + 0.3, 2.5);
			if (data->velocity.x > 0) {
				data->facing = EPF_RIGHT;
			}
			moving_horizontally = true;
		}

		/* prevent player from holding down the up key to perform multiple jumps. */
		bool key_up_pressed_fresh = keystate[SDL_SCANCODE_UP] && !(data->key_up_pressed_prev);
		data->key_up_pressed_prev = keystate[SDL_SCANCODE_UP];

		switch (data->state) {
		case EPS_DEFAULT:
			if (keystate[SDL_SCANCODE_UP]) {
				if (World_rectangle_overlaps_cell_of_type(entity->world, &(entity->rect), LCT_LADDER)) {
					/* climb ladder */
					data->velocity.x = 0;
					data->velocity.y = 0;
					data->state = EPS_CLIMBING;

					/* TODO: ugly hack. separating logic to move vs. logic to climb into
					 *       different functions should clean this up somewhat.
					 */
					return;
				} else {
					/* see if we can jump */
					if (key_up_pressed_fresh) {
						data->jump_charge_counter = 0;
						data->velocity.y = JUMP_SPEED;
						data->state = EPS_JUMPING_CHARGING;
					}
				}
			}
			break;
		case EPS_JUMPING_CHARGING:
			if (keystate[SDL_SCANCODE_UP]) {
				++(data->jump_charge_counter);
				if (data->jump_charge_counter <= MAX_JUMP_CHARGE) {
					data->velocity.y = JUMP_SPEED;
				} else {
					data->jump_charge_counter = 0;
					data->state = EPS_JUMPING;
				}
			} else {
				data->state = EPS_JUMPING;
			}
			break;
		case EPS_JUMPING:
			break;
		case EPS_FALLING:
			break;
		default:
			Log_error("EntityPlayer", "player state not handled.");
		}

		if (!moving_horizontally) {
			if (data->velocity.x > 0) {
				data->velocity.x = max(0, data->velocity.x - 0.4);
			} else if (data->velocity.x < 0) {
				data->velocity.x = min(0, data->velocity.x + 0.4);
			}
			if (data->velocity.x == 0) {
				entity->rect.position.x = round(entity->rect.position.x);
			}
		}

		CollidedWith cw = World_move(entity->world, entity, &(data->velocity));
		if ((cw & CW_LEFT) || (cw & CW_RIGHT)) {
			data->velocity.x = 0;
		}

		
		if (data->state == EPS_FALLING || 
			data->state == EPS_JUMPING || 
			data->state == EPS_JUMPING_CHARGING) {
			if (cw & CW_TOP) {
				data->state = EPS_DEFAULT;
				data->velocity.y = 0;
			} else if (cw & CW_BOTTOM) {
				data->velocity.y = -data->velocity.y * 0.5;
				data->state = EPS_FALLING;
			}
		} else if (cw & CW_TOP) {
			data->state = EPS_DEFAULT;
			data->velocity.y = 0;
		} else {
			data->state = EPS_FALLING;
		}
	} else /* data->state == EPS_CLIMBING */ { 
		/* player is climbing ladder */

		/* check if player is actually still on ladder. */
		bool is_on_ladder = World_rectangle_overlaps_cell_of_type(entity->world, &(entity->rect), LCT_LADDER);

		if (is_on_ladder) {
			/* let player climb. */
			data->velocity.x = keystate[SDL_SCANCODE_RIGHT] * SPEED_CLIMBING_X -
			                   keystate[SDL_SCANCODE_LEFT]  * SPEED_CLIMBING_X;
			data->velocity.y = keystate[SDL_SCANCODE_DOWN]  * SPEED_CLIMBING_Y -
			                   keystate[SDL_SCANCODE_UP]    * SPEED_CLIMBING_Y;
			World_move(entity->world, entity, &(data->velocity));
		} else {
			/* no longer on ladder -> fall */
			data->state = EPS_FALLING;
		}

	}

}

void EntityPlayer_collide(Entity* entity, Entity* entity_other)
{
	/*printf("[ collision detected ] type: %d\n", entity_other->type);*/
}

void EntityPlayer_draw(Entity* entity, Viewport* viewport) 
{
	Rectangle rect = entity->rect;
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);
	SDL_Texture* tex;
	if (data->facing == EPF_RIGHT) {
		tex = tex_player_right;
	} else {
		tex = tex_player_left;
	}


	Viewport_draw_texture(viewport, NULL, &rect, tex);

	/* change score text to display current position. */
	snprintf(
		data->entity_text_text, TEXT_SCORE_SIZE, "%03d %03d %d",
		(int)rect.position.x, (int)rect.position.y, data->state
	);
}


Direction EntityPlayer_viewport_get_direction(Entity* entity) 
{
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);
	if (data->facing == EPF_RIGHT) {
		return DIR_RIGHT;
	} else {
		return DIR_LEFT;
	}
}


void EntityPlayer_destroy(Entity* entity)
{
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);

	if (entity->world) {
		if (entity->world->viewport) {
			entity->world->viewport->locked_onto = NULL;
		}
		World_remove_entity(entity->world, data->entity_text);
	}

	free(data->entity_text_text);
	free(data);
	free(entity);

	Log("EntityPlayer", "destroyed.");
}


bool EntityPlayer_serialize(Entity* entity, char* output)
{
	return false;
}


Entity* EntityPlayer_deserialize(char* input) 
{
	/* format:
	 * type posx posy
	 */
	Entity* player = EntityPlayer_create();
	if (!player) {
		Log_error("EntityPlayer", "failed to create Entity base class.");
		return NULL;
	}

	EntityType type;

	if (sscanf(input, "%d %f %f", &type, &(player->rect.position.x), &(player->rect.position.y)) != 3) {
		Log_error("EntityPlayer", "deserialize: invalid argument count");
		EntityPlayer_destroy(player);
		return NULL;
	}

	Log("EntityPlayer", "deserialized.");
	return player;
}