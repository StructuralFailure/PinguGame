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


float abs_float(float number)
{
	if (number < 0) {
		return -number;
	}
	return number;
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


/* returns the position of the first occurrence of a value inside an array, -1 otherwise. */
int position_in_array(int* array, int array_length, int value) 
{
	for (int i = 0; i < array_length; ++i) {
		if (array[i] == value) {
			return i;
		}
	}
	return -1;
}


/* returns whether a specific value occurs in an array at least once. */
bool is_in_array(int* array, int array_length, int value)
{
	return position_in_array(array, array_length, value) != -1;
}