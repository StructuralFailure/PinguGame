//
// Created by fabian on 07.05.20.
//

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../Forward.h"


typedef struct Controller {
	World* world;

	void (*prepare_update)(struct Controller*);
	void (*finalize_update)(struct Controller*);
	void (*draw)(struct Controller*, Viewport*);
	void (*added_entity)(struct Controller*, Entity*);
	void (*removing_entity)(struct Controller*, Entity*);

	void* data;
} Controller;


Controller* Controller_create(void);
void Controller_destroy(Controller*);


#endif
