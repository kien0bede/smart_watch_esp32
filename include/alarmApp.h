#ifndef __ALARM_APP_H__
#define __ALARM_APP_H__

#include <RTClib.h>

extern RTC_PCF8563 rtc;

extern RTC_DATA_ATTR int hour_alarm;
extern RTC_DATA_ATTR int minute_alarm;
extern RTC_DATA_ATTR bool alarm_on;

// Count times
void alarmApp();
// Init Screen
void alarmInitScreen();

#endif