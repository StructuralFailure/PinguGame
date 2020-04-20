#include <stdlib.h>

#include <SDL2/SDL.h>
#include "../Game.h"
#include "../Log.h"
#include "../Util.h"
#include "../Graphics.h"
#include "../Entity.h"
#include "../SDLHelper.h"
#include "../Viewport.h"
#include "../World.h"
#include "Enemy.h"


#define GRAVITY        0.15
#define MAX_FALL_SPEED 3
#define HSPEED         0.5

SDL_Texture* tex_enemy;


Entity* EntityEnemy_create() 
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
			.x = 0,
			.y = 0
		},
		.size = {
			.x = 16,
			.y = 16
		}
	};

	data->species = EET_ASSHOLE;
	data->velocity = (Vector2D) {
		.x = HSPEED,
		.y = 0
	};

	enemy->add = EntityEnemy_add;
	enemy->update = EntityEnemy_update;
	enemy->draw = EntityEnemy_draw;
	enemy->destroy = EntityEnemy_destroy;
	enemy->collide = NULL;

	return enemy;
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

	CollidedWith cw = World_move(entity->world, entity, v);
	if (cw & CW_TOP) {
		v->y = 0;
	}

	if ((cw & CW_LEFT) || (cw & CW_RIGHT)) {
		/* wall has been hit, turn around */
		v->x = -v->x;
	}

	/*printf("i am an enemy! v = { x = %f | y = %f }\n", v->x, v->y);*/
}


void EntityEnemy_draw(Entity* entity, Viewport* viewport)
{
	Viewport_draw_texture(viewport, NULL, &(entity->rect), tex_enemy);

	/*SDL_Rect sdl_rect = SDLHelper_get_sdl_rect(&(entity->rect));
	SDL_RenderCopy(sdl_renderer, tex_enemy, NULL, &sdl_rect);*/
}


void EntityEnemy_destroy(Entity* entity)
{
	Log("EntityEnemy", "destroyed.");

	free(entity->data);
	free(entity);
}


bool EntityEnemy_serialize(Entity* entity, char* output)
{
	return false;
}


Entity* EntityEnemy_deserialize(char* input) 
{
	/* format:
	 * type posx posy
	 */
	Entity* enemy = EntityEnemy_create();
	if (!enemy) {
		return NULL;
	}

	EntityType type;

	if (sscanf(input, "%d %f %f", &type, &(enemy->rect.position.x), &(enemy->rect.position.y)) != 3) {
		Log_error("EntityEnemy", "deserialize: invalid argument count");
		return NULL;
	}

	Log("EntityEnemy", "deserialized.");
	return enemy;
}