#ifndef __COMPASS_APP_H__
#define __COMPASS_APP_H__

#include <QMC5883LCompass.h>

extern QMC5883LCompass compass;

// Display to TFT
void compassApp();
// Init Screen
void compassInitScreen();

void compassInit();

#endif