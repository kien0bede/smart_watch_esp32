#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

#include <RTClib.h>

extern RTC_PCF8563 rtc;

// Count times
void weatherSyncApp();
// Init Screen
void weatherSyncInitScreen();

#endif