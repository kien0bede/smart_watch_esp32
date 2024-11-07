#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>

#define TFT_BLK_PIN 14

extern TFT_eSPI tft;
extern bool faceChange;
extern int Screen;
extern int subScreen;
extern unsigned long lastWake;

#endif
