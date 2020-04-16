#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <SDL2/SDL.h>
#include "../Game.h"
#include "../Util.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "../Graphics.h"
#include "Player.h"


const int MAX_JUMP_CHARGE = 7;
const float JUMP_SPEED = -3.5;

bool try_jump(Entity* entity);


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
	player->remove = EntityPlayer_remove;
	player->collide = EntityPlayer_collide;


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

}


void EntityPlayer_update(Entity* entity)
{
	EntityPlayerData* data = (EntityPlayerData*)entity->data;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	/* gravity */
	data->velocity.y = min(data->velocity.y + 0.15, 3);

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

	switch (data->state) {
	case EPS_DEFAULT:
		if (keystate[SDL_SCANCODE_UP]) {
			data->jump_charge_counter = 0;
			data->velocity.y = JUMP_SPEED;
			data->state = EPS_JUMPING_CHARGING;
		}
		break;
	case EPS_JUMPING_CHARGING:
		if (keystate[SDL_SCANCODE_UP]) {
			++(data->jump_charge_counter);
			printf("%d ", data->jump_charge_counter);
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

	CollidedWith cw = Game_move(entity->game, entity, &(data->velocity));
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

	/*printf("player state: %d\n", data->state);*/

	RectangleInt overlapping_cells = Game_get_overlapping_cells(entity->game, &(entity->rect));
	/*printf(
		"[ Player ] [ pos .x = %f | .y = %f ] [ x = %d | y = %d | w = %d | h = %d ]\n",
		entity->rect.position.x, entity->rect.position.y,
		overlapping_cells.position.x, overlapping_cells.position.y,
		overlapping_cells.size.x, overlapping_cells.size.y
	);*/

	/* print overlapping cells */
	/*Vector2DInt overlapping_cells[4];
	int amount = Game_get_overlapping_cells(entity->game, &(entity->rect), overlapping_cells, 4);
	printf("overlapping cells: ");
	for (int i = 0; i < amount; ++i) {
		printf("[ %d | %d ] ", overlapping_cells[i].x, overlapping_cells[i].y);
	}
	printf("\n");*/

}

void EntityPlayer_collide(Entity* entity, Entity* entity_other)
{
	/*printf("[ collision detected ] type: %d\n", entity_other->type);*/
}

void EntityPlayer_draw(Entity* entity) 
{
	SDL_Rect sdl_rect = SDLHelper_get_sdl_rect(&(entity->rect));
	EntityPlayerData* data = (EntityPlayerData*)(entity->data);
	SDL_Texture* tex;
	if (data->facing == EPF_RIGHT) {
		tex = tex_player_right;
	} else {
		tex = tex_player_left;
	}
	SDL_RenderCopy(sdl_renderer, tex, NULL, &sdl_rect);
}


void EntityPlayer_remove(Entity* entity)
{
	
}