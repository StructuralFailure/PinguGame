#ifndef WORLD_CONTROLLER_H
#define WORLD_CONTROLLER_H


#include <stdbool.h>

#include "Controller.h"
#include "../Entity.h"
#include "../Forward.h"


typedef struct World World;


typedef struct WorldControllerData {
	World* world;
	int current_score;
	int total_score;
	int current_world_id;
	int previous_player_health;
	int healthbar_flash_counter;

	Entity* player;
} WorldControllerData;


Controller* WorldController_create(void);
void WorldController_destroy(Controller*);

void WorldController_prepare_update(Controller*);
void WorldController_finalize_update(Controller*);
void WorldController_draw(Controller*, Viewport*);
void WorldController_added_entity(Controller*, Entity*);
void WorldController_removing_entity(Controller*, Entity*);


#endif
