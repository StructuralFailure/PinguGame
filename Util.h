#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>


/* math functions */
float min(float a, float b);
float max(float a, float b);
float abs_float(float number);
int signum(float number);

/* array functions */
int position_in_array(int* array, int array_length, int value);
bool is_in_array(int* array, int array_length, int value);


#endif
