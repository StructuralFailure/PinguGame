#ifndef ENTITY_CONTROLLER_H
#define ENTITY_CONTROLLER_H


#include <stdbool.h>

#include "../Entity.h"
#include "../Forward.h"


typedef struct World World;


typedef struct WorldController {
	World* world;
	int current_score;
	int total_score;
	int current_world_id;

	Entity* player;
} WorldController;


WorldController* WorldController_create(void);
void WorldController_destroy(WorldController*);

void WorldController_prepare_update(WorldController*);
void WorldController_finalize_update(WorldController*);
void WorldController_draw(WorldController*, Viewport*);
void WorldController_added_entity(WorldController*, Entity*);
void WorldController_removing_entity(WorldController*, Entity*);


#endif
