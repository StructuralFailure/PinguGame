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


#define MAX_JUMP_CHARGE 7
#define JUMP_SPEED      -3.5

#define SPEED_CLIMBING_X  0.7
#define SPEED_CLIMBING_Y  0.7
/* TODO: add a bunch more defines to get rid of magic numbers. */
#define GRAVITY           0.15 
#define MAX_FALLING_SPEED 3

#define TEX_FRAME_WIDTH  16
#define TEX_FRAME_HEIGHT 20
#define TEX_WALKING_FRAME_COUNT 6


static char text_score[32];
static SDL_Texture* tex_player_left;
static SDL_Texture* tex_player_right;
static SDL_Texture* tex_player;


void handle_item_block_collision(Entity*);
void handle_enemy_collision(Entity*, Entity* entity_other);


Entity* EntityPlayer_create()
{
	if (!tex_player_left) {
		tex_player_left = SDLHelper_load_texture("assets/gfx/player_left.bmp");
	}
	if (!tex_player_right) {
		tex_player_right = SDLHelper_load_texture("assets/gfx/player_right.bmp");
	}
	if (!tex_player) {
		tex_player = SDLHelper_load_texture("assets/gfx/player.bmp");
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
			.y = 20
		}
	};

	player->add = EntityPlayer_add;
	player->update = EntityPlayer_update;
	player->draw = EntityPlayer_draw;
	player->destroy = EntityPlayer_destroy;
	player->collide = EntityPlayer_collide;
	player->get_direction = EntityPlayer_viewport_get_direction;
	player->message = EntityPlayer_message;


	data->state = EPS_DEFAULT;
	data->powerup_level = 123;
	data->health = 456;
	data->velocity = (Vector2D) {
		.x = 0,
		.y = 0
	};
	data->facing = EPF_RIGHT;
	data->jump_charge_counter = 0;
	data->starting_pos = (Vector2D) { 0 };

	return player;
}


void EntityPlayer_add(Entity* entity)
{
	printf("[EntityPlayer] adding.\n");
	/* steal viewport */
	entity->world->viewport->locked_onto = entity;
	/* create text entity */
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);
	data->entity_text = EntityText_create(data->entity_text_text);
	data->entity_text->rect.position = (Vector2D) {
		.x = 8,
		.y = 8
	};
	World_add_entity(entity->world, data->entity_text);

	data->starting_pos = entity->rect.position;
}


void EntityPlayer_update(Entity* entity)
{
	ENTITY_DATA_ASSERT(Player);

	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	//Log("EntityPlayer", "update start: %f | %f", entity->rect.position.x, entity->rect.position.y);

	/* TODO: this is getting messy as fuck.
	 * i need to find a better solution to manage state.
	 */

	if (data->state != EPS_CLIMBING) {
		/* TODO: put this into a separate function to maintain readability. */



		/* player is not climbing.
		 * let him move around the map freely and perform collision detectio etc.
		 */

		/* gravity */
		data->velocity.y = min(data->velocity.y + 0.15f, 4.5f);

		bool moving_horizontally = false;
		if (keystate[SDL_SCANCODE_LEFT]) {
			data->velocity.x = max(data->velocity.x - 0.3f, -2.5f);
			if (data->velocity.x < 0) {
				data->facing = EPF_LEFT;
			}
			moving_horizontally = true;
		} 
		if (keystate[SDL_SCANCODE_RIGHT]) {
			data->velocity.x = min(data->velocity.x + 0.3f, 2.5f);
			if (data->velocity.x > 0) {
				data->facing = EPF_RIGHT;
			}
			moving_horizontally = true;
		}

		/* hackkkk. */
		data->animation_frame_satisfied += abs_float(data->velocity.x) / 15.0f;

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
			float previous_velocity_x = data->velocity.x;
			if (data->velocity.x > 0) {
				data->velocity.x = max(0, data->velocity.x - 0.4f);
			} else if (data->velocity.x < 0) {
				data->velocity.x = min(0, data->velocity.x + 0.4f);
			}
			if (data->velocity.x == 0 && data->velocity.x != previous_velocity_x) {
				float delta_x_rounded = round(entity->rect.position.x) - entity->rect.position.x;
				Vector2D delta = {
					.x = delta_x_rounded,
					.y = 0
				};
				World_move(entity->world, entity, &delta);
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

		//Log("EntityPlayer", "player.y = %f", entity->rect.position.y);

		/*if (cw & CW_TOP) {
			Log("EntityPlayer", "top collision at player.y = %f", entity->rect.position.y);
		}*/


		if (cw & CW_BOTTOM) {
			handle_item_block_collision(entity);
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

	//Log("EntityPlayer", "update start: %f | %f", entity->rect.position.x, entity->rect.position.y);
}


void EntityPlayer_collide(Entity* entity, Entity* entity_other) {
    ENTITY_DATA_ASSERT(Player);

}


void EntityPlayer_draw(Entity* entity, Viewport* viewport) 
{
    ENTITY_DATA_ASSERT(Player);

	/*Rectangle rect = entity->rect;
	SDL_Texture* tex;
	if (data->facing == EPF_RIGHT) {
		tex = tex_player_right;
	} else {
		tex = tex_player_left;
	}*/

	int frame_row;
	int frame_col;

	if (data->state == EPS_JUMPING || data->state == EPS_JUMPING_CHARGING) {
		frame_row = 2;
		frame_col = (data->facing == EPF_LEFT);
	} else {
		if (data->animation_frame_satisfied >= 1.0) {
			data->animation_frame_satisfied = 0;
			data->animation_frame = (data->animation_frame + 1) % TEX_WALKING_FRAME_COUNT;
			//Log("EntityPlayer_draw", "frame satisfied, new frame: %d", data->animation_frame);
		}
		frame_row = (data->facing == EPF_LEFT);
		frame_col = data->animation_frame;
	}

	Rectangle tex_rect = {
			.position = {
					.x = frame_col * TEX_FRAME_WIDTH,
					.y = frame_row * TEX_FRAME_HEIGHT
			},
			.size = entity->rect.size
	};
	Viewport_draw_texture(viewport, &tex_rect, &(entity->rect), tex_player);

	/* change score text to display current position. */
	snprintf(
		data->entity_text_text, TEXT_SCORE_SIZE, "%03d %03d %d",
		(int)(entity->rect.position.x), (int)(entity->rect.position.y), data->state
	);
}


Direction EntityPlayer_viewport_get_direction(Entity* entity) 
{
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);

	Direction direction = DIR_UP | DIR_DOWN; /* force vertical viewport */
	if (data->facing == EPF_RIGHT) {
		return direction |= DIR_RIGHT;
	} else {
		return direction |= DIR_LEFT;
	}

	return direction;
}


void EntityPlayer_destroy(Entity* entity)
{
    ENTITY_DATA(Player);
	//EntityPlayerData* data = (EntityPlayerData*)(entity->data);

	if (entity->world) {
		if (entity->world->viewport) {
			entity->world->viewport->locked_onto = NULL;
		}
		World_remove_entity(entity->world, data->entity_text);
	}

	free(data);
	free(entity);

	Log("EntityPlayer", "destroyed.");
}


void* EntityPlayer_message(Entity* entity, Entity* sender, EntityMessageType message_type, void* payload)
{
    ENTITY_DATA(Player);
    //EntityPlayerData* data = (EntityPlayerData*)(entity->data);

    if (message_type == EMT_I_DAMAGED_YOU) {
        entity->rect.position = data->starting_pos;
    } else if (sender->type == ET_ENEMY && message_type == EMT_YOU_DAMAGED_ME) {
        data->velocity.y *= -1;
    }
	return NULL;
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


void handle_item_block_collision(Entity* entity)
{
	World* world = entity->world;
	Level* level = world->level;

	RectangleInt cells_above = World_get_overlapping_cells(world, &(entity->rect));
	cells_above.position.y -= 1;
	for (int dx = 0; dx < cells_above.size.x; ++dx) {
		Vector2D current_cell = {
			.x = cells_above.position.x + dx,
			.y = cells_above.position.y
		};

		LevelCellTypeProperties* cell_properties = Level_get_cell_type_properties(level, current_cell.x, current_cell.y);
		if (!cell_properties || cell_properties->type != LCT_ITEM_BLOCK) {
			continue;
		}
		/* handle item block */
		Log("EntityPlayer", "item block was hit from below.");
		Level_set_cell_type(level, current_cell.x, current_cell.y, LCT_EMPTY_ITEM_BLOCK);
	}
}


void handle_enemy_collision(Entity* entity, Entity* entity_other)
{

}