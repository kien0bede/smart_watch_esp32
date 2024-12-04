#include <TFT_eSPI.h>
#include <OneButton.h>
#include "display.h"
#include "setting_png.h"

int16_t rc_setting;
int16_t int_duration_brightness;
RTC_DATA_ATTR int16_t duration_brightness = 10000;
RTC_DATA_ATTR int16_t brightness_level = 153;

void watchSettingInitScreen() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        rc_setting = png.openFLASH((uint8_t *)setting_png, sizeof(setting_png), pngDraw);
        if (rc_setting == PNG_SUCCESS) {
            tft.startWrite();
            rc_setting = png.decode(NULL, 0);
            tft.endWrite();
        }
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        String stopwatch = "SETTING";
        int textWidth_stopwatch = tft.textWidth(stopwatch);
        int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
        tft.setCursor(x_stopwatch, 200);
        tft.printf("%s", stopwatch);
        faceChange = false;
    }
}

void watchSettingApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setCursor(0, 20);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("WATCH SETTING");
    tft.setTextSize(2);
    tft.setCursor(10, 60);
    tft.printf("Duration");
    tft.setCursor(40, 90);
    if (duration == 1) {
        tft.printf("2s");
    } else if (duration == 2) {
        tft.printf("5s");
    } else if (duration == 3) {
        tft.printf("10s");
    } else if (duration == 4) {
        tft.printf("15s");
    } else if (duration == 5) {
        tft.printf("30s");
    }
    tft.setCursor(120, 60);
    tft.printf("Brightness");
    tft.setCursor(160, 90);
    if (brightness == 1) {
        tft.printf("20%%");
    } else if (brightness == 2) {
        tft.printf("40%%");
    } else if (brightness == 3) {
        tft.printf("60%%");
    } else if (brightness == 4) {
        tft.printf("80%%");
    } else if (brightness == 5) {
        tft.printf("100%%");
    }
    if (subScreen == 0) {
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 120);
        tft.printf("Duration");
        tft.setCursor(40, 150);
        switch (duration) {
            case 1:
                tft.printf("2s");
                int_duration_brightness = 2000;
                break;
            case 2:
                tft.printf("5s");
                int_duration_brightness = 5000;
                break;
            case 3:
                tft.printf("10s");
                int_duration_brightness = 10000;
                break;
            case 4:
                tft.printf("15s");
                int_duration_brightness = 15000;
                break;
            case 5:
                tft.printf("30s");
                int_duration_brightness = 30000;
                break;
        }
    } else if (subScreen == 1) {
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(120, 120);
        tft.printf("Brightness");
        tft.setCursor(160, 150);
        switch (brightness) {
            case 1:
                tft.printf("20%%");
                brightness_level = 51;
                break;
            case 2:
                tft.printf("40%%");
                brightness_level = 102;
                break;
            case 3:
                tft.printf("60%%");
                brightness_level = 153;
                break;
            case 4:
                tft.printf("80%%");
                brightness_level = 204;
                break;
            case 5:
                tft.printf("100%%");
                brightness_level = 255;
                break;
        }
    } else if (subScreen == 2) {
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(90, 180);
        tft.printf("DONE");
    }
}