#ifndef ENTITY_ENEMY_H
#define ENTITY_ENEMY_H


#include "../Graphics.h"


typedef enum EntityEnemyType {
	EET_ASSHOLE = 0
} EntityEnemyType;


typedef struct EntityEnemyData {
	EntityEnemyType species;
	Vector2D velocity;
} EntityEnemyData;


Entity* EntityEnemy_create(float x, float y);
void EntityEnemy_add(Entity* entity);
void EntityEnemy_update(Entity* entity);
void EntityEnemy_draw(Entity* entity);
void EntityEnemy_remove(Entity* entity);


#endif