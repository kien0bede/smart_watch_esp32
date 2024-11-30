#include <TFT_eSPI.h>
#include <OneButton.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "display.h"
#include "timeSync.h"
#include <ble.h>

void timeSyncInitScreen(){
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Time Sync");
}

void timeSyncApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Time Sync Begin");

    int counter = 1;
    if (newData && BT_IN.length() == 15) {
        newData = false;
        pCharacteristic->setValue("");
        
        int Year = (((BT_IN[0] - '0') * 10) + (BT_IN[1] - '0')) + 2000;
        int Month = ((BT_IN[2] - '0') * 10) + (BT_IN[3] - '0');
        int Date = ((BT_IN[4] - '0') * 10) + (BT_IN[5] - '0');
        int Dow = BT_IN[6] - '0';
        int Hour = ((BT_IN[7] - '0') * 10) + (BT_IN[8] - '0');
        int Minute = ((BT_IN[9] - '0') * 10) + (BT_IN[10] - '0');
        int Second = ((BT_IN[11] - '0') * 10) + (BT_IN[12] - '0');
        
        // Thiết lập thời gian trên RTC
        rtc.adjust(DateTime(Year, Month, Date, Hour, Minute, Second));
        printf("The Time Has Been Set\n");
        counter = 0;
    }

    if (counter == 0) {
        tft.setCursor(10, 50);
        tft.printf("Time Sync Complete");
        delay(2000);
        Screen = 0;
        subScreen = 0;
    }
}