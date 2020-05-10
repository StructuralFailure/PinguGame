//
// Created by fabian on 07.05.20.
//

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include "../Forward.h"


typedef enum ControllerType {
	CT_MENU_CONTROLLER,
	CT_STAGE_CONTROLLER,
	CT_LEVEL_EDITOR_CONTROLLER
} ControllerType;


typedef enum RequestedWorld {
	RW_NONE = -1
} RequestedWorld;


typedef struct Controller {
	ControllerType type;
	World* world;
	int requested_world; /* -1 = ignored */


	void (*prepare_update)(struct Controller*);
	void (*finalize_update)(struct Controller*);
	void (*draw)(struct Controller*, Viewport*);
	void (*added_entity)(struct Controller*, Entity*);
	void (*removing_entity)(struct Controller*, Entity*);
	void (*destroy)(struct Controller*);

	void* data;
} Controller;


Controller* Controller_create(void);
void Controller_destroy(Controller*);


#endif
