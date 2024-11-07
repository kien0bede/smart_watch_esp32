#ifndef __HEART_RATE_APP_H__
#define __HEART_RATE_APP_H__

#include "MAX30105.h"

extern MAX30105 particleSensor;

// Display to TFT
void heartRateApp();
// Init Screen
void heartRateInitScreen();
// Result Screen
void heartRateResultScreen();

#endif