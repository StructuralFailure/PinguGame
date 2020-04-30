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
	enemy->collide = EntityEnemy_collide;

	return enemy;
}

void EntityEnemy_add(Entity* entity)
{
	/* TODO: implement */
}


void EntityEnemy_update(Entity* entity) 
{
    ENTITY_DATA_ASSERT(Enemy);

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


void EntityEnemy_collide(Entity* entity, Entity* entity_other)
{
    if (entity_other->type != ET_PLAYER) {
        /* don't care about collisions with entities other than the player.
         * TODO: make enemies turn around when they run into one another.
         */
        return;
    }

    Rectangle eo_previous_rect = entity_other->previous_rect;

    Vector2D eo_delta_pos = Vector2D_difference(
            entity_other->rect.position,
            entity_other->previous_rect.position
    );

    CollidedWith collided_with = World_move_until_collision_with_flags(
            entity->world, &eo_previous_rect, &eo_delta_pos, CC_RECTANGLE, &(entity->rect)
    );

    if (collided_with != CW_TOP) {
        Log("EntityEnemy", "collide: damaged player.");
        if (entity_other->message) {
            entity_other->message(entity_other, entity, EMT_I_DAMAGED_YOU, NULL);
        }
    } else {
        Log("EntityEnemy", "collide: player damaged me.");
        if (entity_other->message) {
            entity_other->message(entity_other, entity, EMT_YOU_DAMAGED_ME, NULL);
        }
    }
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
	int position_x;
	int position_y;

	if (sscanf(input, "%d %d %d", &type, &position_x, &position_y) != 3) {
		Log_error("EntityEnemy", "deserialize: invalid argument count");
		return NULL;
	}

	enemy->rect.position = (Vector2D) { position_x, position_y };

	Log("EntityEnemy", "deserialized.");
	return enemy;
}