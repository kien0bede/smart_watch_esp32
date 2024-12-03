#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <PNGdec.h>

#define TFT_BLK_PIN 14

typedef unsigned char UBYTE;

extern UBYTE buf[10];
extern TFT_eSPI tft;
extern TFT_eSprite img;
extern PNG png;
extern bool faceChange;
extern int Screen;
extern int subScreen;
extern unsigned long lastWake;
extern RTC_DATA_ATTR int totalStep;
extern RTC_DATA_ATTR int preBPM;
void pngDraw(PNGDRAW *pDraw);

#endif
