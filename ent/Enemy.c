#include <stdlib.h>

#include <SDL2/SDL.h>
#include "../Game.h"
#include "../Util.h"
#include "../Graphics.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "Enemy.h"


#define GRAVITY        0.15
#define MAX_FALL_SPEED 3
#define HSPEED         0.5

SDL_Texture* tex_enemy;


Entity* EntityEnemy_create(float x, float y) 
{
	if (!tex_enemy) {
		tex_enemy = SDLHelper_load_texture("assets/gfx/enemy.bmp");
	}

	Entity* enemy = Entity_create();
	if (!enemy) {
		return NULL;
	}

	EntityEnemyData* data = calloc(1, sizeof(EntityEnemyData));
	if (!data) {
		free(enemy);
		return NULL;
	}
	enemy->data = data;

	enemy->type = ET_ENEMY;
	enemy->rect = (Rectangle) {
		.position = {
			.x = x,
			.y = y
		},
		.size = {
			.x = 16,
			.y = 16
		}
	};

	data->species = EET_ASSHOLE;
	data->going_right = true;
	data->velocity = (Vector2D) {
		.x = HSPEED,
		.y = 0
	};

	enemy->add = EntityEnemy_add;
	enemy->update = EntityEnemy_update;
	enemy->draw = EntityEnemy_draw;
	enemy->remove = EntityEnemy_remove;
	enemy->collide = NULL;

	return enemy;
}

void EntityEnemy_destroy(Entity* entity) 
{
	free(entity->data);
	free(entity);
}


void EntityEnemy_add(Entity* entity)
{
	/* TODO: implement */
}


void EntityEnemy_update(Entity* entity) 
{
	EntityEnemyData* data = (EntityEnemyData*)(entity->data);

	Vector2D* v = &(data->velocity);
	v->y = min(v->y + GRAVITY, MAX_FALL_SPEED);

	CollidedWith cw = Game_move(entity->game, entity, v);
	if (cw & CW_TOP) {
		v->y = 0;
	}

	if ((cw & CW_LEFT) || (cw & CW_RIGHT)) {
		/* wall has been hit, turn around */
		v->x = -v->x;
	}

	/*printf("i am an enemy! v = { x = %f | y = %f }\n", v->x, v->y);*/
}


void EntityEnemy_draw(Entity* entity)
{
	SDL_Rect sdl_rect = SDLHelper_get_sdl_rect(&(entity->rect));
	SDL_RenderCopy(sdl_renderer, tex_enemy, NULL, &sdl_rect);
}


void EntityEnemy_remove(Entity* entity)
{


}
