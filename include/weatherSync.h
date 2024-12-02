#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

#include <RTClib.h>
#include <PCF8563.h>

extern int minTemp;
extern int maxTemp;
extern String weatherCondition;

// Count times
void weatherSyncApp();
// Init Screen
void weatherSyncInitScreen();

#endif