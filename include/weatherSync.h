#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

#include <RTClib.h>

extern RTC_PCF8563 rtc;

extern int minTemp;
extern int maxTemp;
extern String weatherCondition;

// Count times
void weatherSyncApp();
// Init Screen
void weatherSyncInitScreen();

#endif