//
// Created by fabian on 12.05.20.
//

#include <stdio.h>
#include <stdlib.h>
#include "../ent/Text.h"
#include "LevelEditorController.h"
#include "Controller.h"
#include "../Log.h"


typedef LevelEditorControllerData Data;


Controller* LevelEditorController_create(void)
{
	Controller* controller = Controller_create();
	if (!controller) {
		Log_error("LevelEditorController_create", "failed to create controller.");
		return NULL;
	}

	Data* data = calloc(1, sizeof(Data));
	if (!data) {
		Log_error("LevelEditorController_create", "failed to allocate memory for data.");
		Controller_destroy(controller);
		return NULL;
	}

	controller->data = data;
	strcpy(data->test_text, "Testtext");

	return controller;
}


void LevelEditorController_destroy(Controller* controller)
{
	Data* data = controller->data;
	EntityText_destroy(data->text_entity);
	free(data);
	free(controller);
}


void LevelEditorController_add(Controller* controller)
{
	Data* data = controller->data;
	if (!data) {
		Log_error("LevelEditorController_add", "data == NULL");
		return;
	}

	data->text_entity = EntityText_create(data->test_text);
	World_add_entity(controller->world, data->text_entity);
}


void LevelEditorController_update(Controller* controller)
{

}


void LevelEditorController_draw(Controller* controller)
{

}