#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

#include "Log.h"

#define ENTITY_DATA(TYPE)                                      \
            Entity ## TYPE ## Data * data = (Entity ## TYPE ## Data *)(entity->data)
#define ENTITY_DATA_ASSERT(TYPE)                               \
            ENTITY_DATA(TYPE);                                 \
            if (!data) {                                       \
                Log_error(__func__, "data == NULL");     \
                return;                                        \
            }
#define ENTITY_EVENT(ENTITY_TYPE, EVENT_NAME)                             \
			entity->EVENT_NAME = Entity ## ENTITY_TYPE ## _ ## EVENT_NAME
#define ENTITY_SETUP_CALLBACKS(TYPE, OBJ_POINTER) {            \
            OBJ_POINTER->add = Entity ## TYPE ## _add;         \
            OBJ_POINTER->update = Entity ## TYPE ## _update;   \
            OBJ_POINTER->draw = Entity ## TYPE ## _draw;       \
            OBJ_POINTER->collide = Entity ## TYPE ## _collide; \
            OBJ_POINTER->destroy = Entity ## TYPE ## _destroy; \
            OBJ_POINTER->message = Entity ## TYPE ## _message; \
        }

/* math functions */
float min(float a, float b);
float max(float a, float b);
float abs_float(float number);
int signum(float number);

/* array functions */
int position_in_array(int* array, int array_length, int value);
bool is_in_array(int* array, int array_length, int value);


#endif
