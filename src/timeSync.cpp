#include <TFT_eSPI.h>
#include <OneButton.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "display.h"
#include "timeSync.h"
#include <ble.h>
#include "sync_png.h"
#include <PCF8563.h>

int16_t rc_sync;

void timeSyncInitScreen() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        rc_sync = png.openFLASH((uint8_t *)sync_png, sizeof(sync_png), pngDraw);
        if (rc_sync == PNG_SUCCESS) {
            tft.startWrite();
            rc_sync = png.decode(NULL, 0);
            tft.endWrite();
        }
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        String stopwatch = "SYNC TIME";
        int textWidth_stopwatch = tft.textWidth(stopwatch);
        int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
        tft.setCursor(x_stopwatch, 200);
        tft.printf("%s", stopwatch);
        faceChange = false;
    }
}

void timeSyncApp() {
    if (faceChange) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Time Sync...");

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

            PCF8563_Set_Time(Hour, Minute, Second);
            PCF8563_Set_Days(Year, Month, Date);
            PCF8563_Get_Time(buf);
            PCF8563_Get_Days(&buf[3]);

            printf("The Time Has Been Set\n");
            faceChange = true;
            counter = 0;

        } catch (...) {
            printf("Error: Exception in parsing time data.\n");  // Debug lỗi nếu có
        }
    } else if (newData) {
        printf("Error: Invalid data length.\n");  // Thông báo lỗi nếu dữ liệu không hợp lệ
        newData = false;
    }

    if (counter == 0) {
        if (faceChange) {
            tft.fillScreen(TFT_BLACK);
            faceChange = false;
        }
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(45, 10);
        tft.printf("Finished!");
        delay(2000);
        Screen = 0;
        subScreen = 0;
        faceChange = true;
    }
}