#include <Arduino.h>
#include "display.h"
#include "compassApp.h"
#include "dial240.h"
#include "fonts.h"
#include "compass_png.h"

#define color1 TFT_WHITE
#define color2  0x8410
#define color3 TFT_ORANGE
#define color4 0x15B3
#define color5 0x00A3

QMC5883LCompass compass;

int16_t rc_compass;  
float lastValue=0;
double rad=0.01745;
float x[360];
float y[360];
float px[360];
float py[360];
float lx[360];
float ly[360];
int r=85;
int sx=120;
int sy=98;
int angle=0;
int lastAngle=0;
String lastHeading = "";
int start[12];
int startP[60];
String cc[12] = {"90", "120", "150", "180", "210", "240", "270", "300", "330", "0", "30", "60"};

void compassInit() {
    compass.init();
    compass.setSmoothing(25,true);
    int b = 0;
    int b2 = 0;
    for(int i=0;i<360;i++) {
        x[i]=(r*cos(rad*i))+sx;
        y[i]=(r*sin(rad*i))+sy;
        px[i]=((r-16)*cos(rad*i))+sx;
        py[i]=((r-16)*sin(rad*i))+sy;
        lx[i]=((r-24)*cos(rad*i))+sx;
        ly[i]=((r-24)*sin(rad*i))+sy;
        if (i % 30 == 0) {
            start[b] = i;
            b++;
        }
    }
}

void compassApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(4);  
        img.createSprite(240, 240);
        img.setTextDatum(4);
        faceChange = false;    
    }
    lastWake = millis();
    compass.read();
    int value = compass.getAzimuth();
    if (value < 0) {
        value += 360;
    }

    value = (value + 35) % 360;

    value = (value + 60) % 360;

    angle = value;

    String heading;
    if (((angle >= 338) && (angle <= 359)) || ((angle >= 0) && (angle <= 22))) { heading = " N"; }
    else if ((angle >= 23) && (angle <= 67)) { heading = "NE"; }
    else if ((angle >= 68) && (angle <= 113)) { heading = " E"; }
    else if ((angle >= 114) && (angle <= 157)) { heading = "SE"; }
    else if ((angle >= 158) && (angle <= 202)) { heading = " S"; }
    else if ((angle >= 203) && (angle <= 248)) { heading = "SW"; }
    else if ((angle >= 249) && (angle <= 292)) { heading = " W"; }
    else if ((angle >= 293) && (angle <= 337)) { heading = "NW"; }

    if (heading != lastHeading) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK);
        tft.drawString(lastHeading, 120, 15);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(heading, 120, 15);
        lastHeading = heading;
    }

    if (angle != lastAngle) {
        lastAngle = angle;

        img.fillSprite(TFT_BLACK);
        img.fillCircle(sx, sy, 100, color5);
        img.setTextColor(TFT_WHITE, color5);
        img.setFreeFont(&FreeSans9pt7b);

        // Đảo chiều xoay bằng cách sử dụng (360 - angle)
        int reverseAngle = (360 - angle) % 360;

        for (int i = 0; i < 12; i++) {
            int adjustedIndex = (start[i] + reverseAngle) % 360;

            img.drawString(cc[i], x[adjustedIndex], y[adjustedIndex]);
            img.drawLine(px[adjustedIndex], py[adjustedIndex], lx[adjustedIndex], ly[adjustedIndex], color1);
        }

        img.setFreeFont(&DSEG7_Modern_Bold_20);
        img.drawString(String(value), sx - 2, sy - 2);
        img.setTextFont(0);
        img.drawString("AZMUTH", sx, sy - 22);

        for (int i = 0; i < 360; i += 6) {
            int adjustedIndex = (i + reverseAngle) % 360;
            img.fillCircle(px[adjustedIndex], py[adjustedIndex], 1, color2);
        }

        img.fillTriangle(sx - 1, sy - 50, sx - 5, sy - 36, sx + 4, sy - 36, TFT_ORANGE);
        img.pushSprite(0, 31);
    }
}

void compassInitScreen() {
  if (faceChange == true) {
      tft.fillScreen(TFT_BLACK);
      rc_compass = png.openFLASH((uint8_t *)compass_png, sizeof(compass_png), pngDraw);
      if (rc_compass == PNG_SUCCESS) {
          tft.startWrite();
          rc_compass = png.decode(NULL, 0);
          tft.endWrite();
      }
      tft.setTextSize(3);
      tft.setTextColor(TFT_WHITE, TFT_BLACK); 
      String stopwatch = "COMPASS";
      int textWidth_stopwatch = tft.textWidth(stopwatch);
      int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
      tft.setCursor(x_stopwatch, 200);
      tft.printf("%s", stopwatch);
      faceChange = false;
    }
}
