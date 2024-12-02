#include <TFT_eSPI.h>
#include <OneButton.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "display.h"
#include "timeSync.h"
#include <ble.h>
#include "bao_thuc_png.h"

int16_t rc_alarm;
RTC_DATA_ATTR int hour_alarm = 0;
RTC_DATA_ATTR int minute_alarm = 0;
RTC_DATA_ATTR bool alarm_on = false;

void alarmInitScreen() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        rc_alarm = png.openFLASH((uint8_t *)bao_thuc_png, sizeof(bao_thuc_png), pngDraw);
        if (rc_alarm == PNG_SUCCESS) {
            tft.startWrite();
            rc_alarm = png.decode(NULL, 0);
            tft.endWrite();
        }
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        String stopwatch = "ALARMS";
        int textWidth_stopwatch = tft.textWidth(stopwatch);
        int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
        tft.setCursor(x_stopwatch, 200);
        tft.printf("%s", stopwatch);
        faceChange = false;
    }
}

void alarmApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    PCF8563_Get_Time(buf);
    tft.setCursor(30, 20);
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK); 
    tft.printf("%02d:%02d", 
        buf[2],    // Giờ
        buf[1]  // Phút
    );
    tft.setCursor(30, 80);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf(alarm_on ? " ON" : " OFF");
    if (subScreen == 0) {
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 50);
        tft.printf("%02d", hour_alarm);
    } else if (subScreen == 1) {
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 50);
        tft.printf("%02d", minute_alarm);
    } else if (subScreen == 2) {
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 50);
        tft.printf("Set Alarm");
    } else if (subScreen == 3) {
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 50);
        tft.printf("Unset Alarm");
    }
}