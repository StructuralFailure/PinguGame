#include "Util.h"


float min(float a, float b) 
{
	if (a < b) {
		return a;
	}
	return b;
}

float max(float a, float b) 
{
	if (a > b) {
		return a;
	}
	return b;
}

int signum(float number)
{
	if (number < 0) {
		return -1;
	} else if (number > 0) {
		return 1;
	}
	return 0;
}
