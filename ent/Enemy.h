#ifndef ENTITY_ENEMY_H
#define ENTITY_ENEMY_H


#include "../Graphics.h"
#include "../Entity.h"
#include "../Viewport.h"


typedef enum EntityEnemyType {
	EET_ASSHOLE = 0
} EntityEnemyType;


typedef struct EntityEnemyData {
	EntityEnemyType species;
	Vector2D velocity;
} EntityEnemyData;


Entity* EntityEnemy_create();
void EntityEnemy_add(Entity* entity);
void EntityEnemy_update(Entity* entity);
void EntityEnemy_draw(Entity* entity, Viewport* viewport);
void EntityEnemy_remove(Entity* entity);

/* saving and loading */
bool EntityEnemy_serialize(Entity* entity, char* output);
Entity* EntityEnemy_deserialize(char* input);


#endif
