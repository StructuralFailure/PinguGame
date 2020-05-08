//
// Created by fabian on 07.05.20.
//

#include <stdlib.h>

#include "Controller.h"
#include "../Log.h"


Controller* Controller_create(void)
{
	Controller* controller = calloc(1, sizeof(Controller));
	if (!controller) {
		Log_error("Controller_create", "failed to allocate memory.");
		return NULL;
	}
	return controller;
}