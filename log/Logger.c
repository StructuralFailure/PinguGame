#include <stdlib.h>

#include "Logger.h"


Logger* Logger_create(int capacity) 
{
	Logger* logger = malloc(sizeof(Logger));
	if (!logger) {
		return NULL;
	}
	logger->capacity = capacity;
	logger->position = 0;
	logger->events = malloc(sizeof(LogEvent) * capacity);
	if (!logger->events) {
		free(logger);
		return NULL;
	}
	
	return logger;
}





void Logger_destroy(Logger* logger) 
{
	free(logger->events);
	free(logger);
}
