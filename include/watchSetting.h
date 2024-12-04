#ifndef __WATCH_SETTING_H__
#define __WATCH_SETTING_H__

#include <RTClib.h>
#include <PCF8563.h>

extern int16_t int_duration_brightness;
extern RTC_DATA_ATTR int16_t duration_brightness;
extern RTC_DATA_ATTR int16_t brightness_level;

// Count times
void watchSettingApp();
// Init Screen
void watchSettingInitScreen();

#endif