//
// Created by fabian on 13.05.20.
//

#include <stdlib.h>
#include "FlameSpirit.h"
#include "../Tileset.h"
#include "../Entity.h"
#include "../Graphics.h"
#include "../Log.h"
#include "../World.h"
#include "../Util.h"
#include "Player.h"


#define TEX_PATH  "assets/gfx/flame_spirit.bmp"

/* anchor point:
 * .....
 * .TTT.   T = texture
 * .TET.   E = entity's rectangle
 * .TET.
 * .....
 */
#define WIDTH              10
#define HEIGHT             14
#define HSPEED             0.35f
#define GRAVITY            0.3f
#define GRAVITY_MAX_VSPEED 3.0f

static int DAMAGE_TO_PLAYER = 2;


typedef EntityFlameSpiritState State;
typedef EntityFlameSpiritData  Data;


static Tileset ts_flame_spirit = {
		.tile_width = 12,
		.tile_height = 16
};

static Animation anim_right = {
		.tileset = &ts_flame_spirit,
		.position = { 0, 1 },
		.length = 6,
		.ticks_per_frame = 5
};

static Animation anim_left = {
		.tileset = &ts_flame_spirit,
		.position = { 0, 0 },
		.length = 6,
		.ticks_per_frame = 5
};


Entity* EntityFlameSpirit_create()
{
	Log("EntityFlameSpirit_create", "creating.");

	if (!ts_flame_spirit.texture) {
		ts_flame_spirit.texture = SDLHelper_load_texture(TEX_PATH);
	}

	Entity* flame_spirit = Entity_create();
	if (!flame_spirit) {
		Log_error("EntityFlameSpirit_create", "failed to create entity.");
		return NULL;
	}

	Data* data = calloc(1, sizeof(Data));
	if (!data) {
		Log_error("EntityFlameSpirit_create", "failed to allocate memory for data.");
		free(flame_spirit);
		return NULL;
	}

	flame_spirit->update = EntityFlameSpirit_update;
	flame_spirit->draw = EntityFlameSpirit_draw;
	flame_spirit->collide = EntityFlameSpirit_collide;


	flame_spirit->type = ET_FLAME_SPIRIT;
	flame_spirit->data = data;
	flame_spirit->rect = (Rectangle) {
			.position = { 0 ,0 },
			.size = { WIDTH, HEIGHT }
	};

	data->velocity = (Vector2D) { HSPEED, 0 };

	Log("EntityFlameSpirit_create", "created.");
	return flame_spirit;
}


void EntityFlameSpirit_destroy(Entity* entity)
{
	if (!entity) {
		return;
	}

	Log("EntityFlameSpirit_destroy", "destroying.");

	free(entity->data);
	free(entity);

	Log("EntityFlameSpirit_destroy", "destroyed.");
}


bool EntityFlameSpirit_serialize(char* output)
{
	return false;
}


Entity* EntityFlameSpirit_deserialize(char* input)
{
	Log("EntityFlameSpirit_deserialize", "deserializing.");

	Entity* flame_spirit = EntityFlameSpirit_create();
	if (!flame_spirit) {
		Log_error("EntityFlameSpirit_deserialize", "failed to create entity.");
	}

	int type;
	int x;
	int y;

	int num_args;
	if ((num_args = sscanf(input, "%d %d %d", &type, &x, &y)) != 3) {
		Log_error(
				"EntityFlameSpirit_deserialize", "failed to deserialize. invalid argument count: 3 expected, %d supplied.",
				num_args
		);
		EntityFlameSpirit_destroy(flame_spirit);
		return NULL;
	}

	flame_spirit->rect.position = (Vector2D) { x, y };
	Log("EntityFlameSpirit_deserialize", "deserialized.");
	return flame_spirit;
}


void EntityFlameSpirit_draw(Entity* entity, Viewport* viewport)
{
	ENTITY_DATA_ASSERT(FlameSpirit);

	Animation* anim = (data->velocity.x >= 0 ? &anim_right : &anim_left);
	RectangleInt frame_rect = Animation_get_frame(anim, (int)entity->world->ticks);
	/* TODO: use ints for drawing. */
	Viewport_draw_texture(
			viewport,
			&(Rectangle) {
				.position = { frame_rect.position.x, frame_rect.position.y },
				.size =     { frame_rect.size.x,     frame_rect.size.y }
			},
			&(entity->rect),
			ts_flame_spirit.texture
	);
}


void EntityFlameSpirit_update(Entity* entity)
{
	ENTITY_DATA_ASSERT(FlameSpirit);

	/* apply gravity. */
	if (data->velocity.y + GRAVITY < GRAVITY_MAX_VSPEED) {
		data->velocity.y += GRAVITY;
	}

	/* move and turn around if necessary. */
	CollidedWith collided_with = World_move(entity->world, entity, &(data->velocity));
	if (collided_with & (CW_LEFT | CW_RIGHT)) {
		data->velocity.x *= -1;
	}
	if (collided_with & (CW_TOP | CW_BOTTOM)) {
		data->velocity.y = 0;
	}
}


void EntityFlameSpirit_collide(Entity* entity, Entity* other_entity)
{
	ENTITY_DATA_ASSERT(FlameSpirit);

	if (other_entity->type != ET_PLAYER) {
		return;
	}

	EntityPlayerData* player_data = entity->data;
	/*Vector2D player_delta_pos = Vector2D_difference(other_entity->rect.position, other_entity->previous_rect.position);
	Vector2D player_delta_pos_neg = (Vector2D) { player_delta_pos.x * -0.2f, player_delta_pos.y * -0.2f };
	World_move(other_entity->world, other_entity, &player_delta_pos_neg);*/

	int damage = DAMAGE_TO_PLAYER;
	other_entity->message(other_entity, entity, EMT_I_DAMAGED_YOU, &DAMAGE_TO_PLAYER);
}


static void turn_around(Entity* entity)
{

}