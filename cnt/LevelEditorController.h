//
// Created by fabian on 12.05.20.
//

#ifndef LEVEL_EDITOR_CONTROLLER_H
#define LEVEL_EDITOR_CONTROLLER_H


#include "Controller.h"


typedef struct LevelEditorControllerData {

	char test_text[256];
	Entity* text_entity;

} LevelEditorControllerData;


Controller* LevelEditorController_create(void);
void LevelEditorController_destroy(Controller*);

void LevelEditorController_add(Controller*);
void LevelEditorController_update(Controller*);
void LevelEditorController_draw(Controller*);


#endif
