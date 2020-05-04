#ifndef ENTITY_SNAIL_H
#define ENTITY_SNAIL_H

#include "Graphics.h"
#include "Entity.h"


typedef enum EntitySnailStickingDirection {
	ESSD_RIGHTWARDS,
	ESSD_DOWNWARDS,
	ESSD_LEFTWARDS,
	ESSD_UPWARDS
 }EntitySnailStickingDirection;


typedef enum EntitySnailState {
    ESS_ROLLING,
    ESS_FALLING,
    ESS_KICKABLE,
    ESS_CRAWLING,
    __ESS_COUNT
} EntitySnailState;

typedef struct EntitySnailData {
    EntitySnailStickingDirection sticking; /* which side OF THE SNAIL is sticking to the block, NOT which face of the block it's sticking to */
    bool crawling_clockwise; /* refers to inner turns, will be reversed when there is an outer turn. */
    Vector2D shell_velocity;
    float falling_speed;
    int kicking_cooldown;
    EntitySnailState state;
    EntitySnailState previous_state;

    Entity* locked_onto;
} EntitySnailData;


Entity* EntitySnail_create();
void    EntitySnail_add(Entity*);
void    EntitySnail_update(Entity*);
void    EntitySnail_draw(Entity*, Viewport*);
void    EntitySnail_destroy(Entity*);
void    EntitySnail_collide(Entity*, Entity* entity_other);
void*   EntitySnail_message(Entity* receiver, Entity* sender, EntityMessageType, void* payload);

void EntitySnail_added_other_entity(Entity* entity, Entity* entity_other);
void EntitySnail_removing_other_entity(Entity* entity, Entity* entity_other);

bool EntitySnail_serialize(Entity*, char* output);
Entity* EntitySnail_deserialize(char* input);


#endif
