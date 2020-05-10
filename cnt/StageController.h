#ifndef WORLD_CONTROLLER_H
#define WORLD_CONTROLLER_H


#include <stdbool.h>

#include "Controller.h"
#include "../Entity.h"
#include "../Forward.h"


typedef struct World World;


typedef struct StageControllerData {
	World* world;
	int current_score;
	int total_score;
	int current_world_id;
	int previous_player_health;
	int healthbar_flash_counter;

	Entity* player;
} StageControllerData;


Controller* StageController_create(void);


void StageController_add(Controller*);
void StageController_prepare_update(Controller*);
void StageController_finalize_update(Controller*);
void StageController_draw(Controller*, Viewport*);
void StageController_added_entity(Controller*, Entity*);
void StageController_removing_entity(Controller*, Entity*);
void StageController_destroy(Controller*);


#endif
