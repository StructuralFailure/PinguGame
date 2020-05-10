//
// Created by fabian on 07.05.20.
//

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include "Controller.h"


typedef enum MenuControllerState {
	MCS_MAIN_MENU,
	MCS_OPTIONS_MENU
} MenuControllerState;


typedef struct MenuControllerData {
	MenuControllerState state;
} MenuControllerData;


Controller* MenuController_create(void);

void        MenuController_finalize_update(Controller*);
void        MenuController_draw(Controller*, Viewport*);
void        MenuController_add(Controller*);
void        MenuController_destroy(Controller*);


#endif
