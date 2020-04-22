#ifndef ENTITY_ITEM_H
#define ENTITY_ITEM_H


typedef enum EntityItemType {
	EIT_FISH
} EntityItemType;


typedef enum EntityItemState {
	EIS_EMERGING,
	EIS_NORMAL,
	EIS_DISAPPEARING
} EntityItemState;


typedef struct EntityItemData {
	EntityItemType type;
	EntityItemState state;
	bool going_right;
} EntityItemData;


#endif