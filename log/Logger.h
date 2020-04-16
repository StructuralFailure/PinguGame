#ifndef LOGGER_H
#define LOGGER_H


typedef enum LogEventType {
	LE_GENERIC,
	LE_SDL,
	LE_GAME_COLLISION,
} LogEventType;


typedef struct LogEvent {
	long timestamp;
	LogEventType type;
	void* data;
} LogEvent;


typedef struct Logger {
	int capacity;
	int position;
	LogEvent* events;
} Logger;


#endif