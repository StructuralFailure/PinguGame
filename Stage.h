//
// Created by fabian on 08.05.20.
//

#ifndef STAGE_H
#define STAGE_H


typedef enum StageRealm {
	SR_OVERWORLD,
	SR_ARCTIC,
	SR_CLOUDS
} StageRealm;


typedef enum StageType {
	ST_DEFAULT,
	ST_UNDERWATER
} StageType;


typedef struct Stage {

	StageRealm realm;
	StageType type;

	const char* name;
	const char* path;

} Stage;


#endif
