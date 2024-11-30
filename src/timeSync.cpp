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
    if (faceChange) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Time Sync Begin");

    int counter = 1;
    lastWake = millis();

    // Kiểm tra và xử lý dữ liệu mới
    if (newData && BT_IN.length() == 13) {
        newData = false;
        
        try {
            int Year = (((BT_IN[0] - '0') * 10) + (BT_IN[1] - '0')) + 2000;
            int Month = ((BT_IN[2] - '0') * 10) + (BT_IN[3] - '0');
            int Date = ((BT_IN[4] - '0') * 10) + (BT_IN[5] - '0');
            int Hour = ((BT_IN[7] - '0') * 10) + (BT_IN[8] - '0');
            int Minute = ((BT_IN[9] - '0') * 10) + (BT_IN[10] - '0');
            int Second = ((BT_IN[11] - '0') * 10) + (BT_IN[12] - '0');
            
            // Debug thông tin thời gian
            printf("Year: %d, Month: %d, Date: %d, Hour: %d, Minute: %d, Second: %d\n",
                   Year, Month, Date, Hour, Minute, Second);
            
            // Thiết lập thời gian trên RTC
            rtc.adjust(DateTime(Year, Month, Date, Hour, Minute, Second));
            printf("The Time Has Been Set\n");
            counter = 0;

        } catch (...) {
            printf("Error: Exception in parsing time data.\n");  // Debug lỗi nếu có
        }
    } else if (newData) {
        printf("Error: Invalid data length.\n");  // Thông báo lỗi nếu dữ liệu không hợp lệ
        newData = false;
    }

    if (counter == 0) {
        delay(2000);
        Screen = 0;
        subScreen = 0;
        faceChange = true;
    }
}