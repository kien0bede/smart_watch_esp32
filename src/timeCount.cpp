#include <TFT_eSPI.h>
#include <OneButton.h>
#include "display.h"
#include "timeCount.h"
#include "counttime.h"

bool isRunning = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;

int16_t rc_stopwatch;  

void displayTime(unsigned long timeInMillis) {
    unsigned long minutes = (timeInMillis / 60000) % 60;
    unsigned long seconds = (timeInMillis / 1000) % 60;
    unsigned long millisec = timeInMillis % 1000;

    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }

    char timeBuffer[9];  // Chứa chuỗi kết quả, độ dài 8 cho "MM:SS:SSS"
    sprintf(timeBuffer, "%02lu:%02lu:%03lu", minutes, seconds, millisec);
    String timeString = String(timeBuffer);
    // Căn giữa trên màn hình
    int timeWidth = tft.textWidth(timeString);  // Tính chiều rộng của chuỗi
    int x = (tft.width() - timeWidth) / 2;  // Căn giữa theo chiều ngang
    int y = (tft.height() - 24) / 2;  // Căn giữa theo chiều dọc

    // Hiển thị thời gian
    tft.setCursor(x, y);
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Màu chữ trắng, nền đen
    tft.print(timeString); 
}

void startStop() {
    if (isRunning) {
        String nextframe = "   STOP   ";
        int textWidth_nextframe = tft.textWidth(nextframe);  
        int x_nextframe = (tft.width() - textWidth_nextframe) / 2; 
        tft.setTextSize(3);
        tft.setTextColor(TFT_RED, TFT_BLACK); 
        tft.setCursor(x_nextframe, 200);
        tft.printf("%s", nextframe);
        isRunning = false;
        elapsedTime += millis() - startTime;
    } else {
        String nextframe = "START";
        int textWidth_nextframe = tft.textWidth(nextframe);  
        int x_nextframe = (tft.width() - textWidth_nextframe) / 2; 
        tft.setTextSize(3);
        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
        tft.setCursor(x_nextframe, 200);
        tft.printf("%s", nextframe);
        isRunning = true;
        startTime = millis();
    }
}

void resetTime() {
    isRunning = false;
    elapsedTime = 0;
    displayTime(0);
}

void timeCountApp() {
  unsigned long currentTime;
    if (isRunning) {
        lastWake = millis();
        currentTime = millis() - startTime + elapsedTime;
        displayTime(currentTime);
    } else {
        displayTime(elapsedTime);
    }
}

void timeCountInitScreen() {
    if (faceChange == true) {
            tft.fillScreen(TFT_BLACK);
            rc_stopwatch = png.openFLASH((uint8_t *)counttime, sizeof(counttime), pngDraw);
            if (rc_stopwatch == PNG_SUCCESS) {
                tft.startWrite();
                rc_stopwatch = png.decode(NULL, 0);
                tft.endWrite();
            }
            tft.setTextSize(3);
            tft.setTextColor(TFT_CYAN, TFT_BLACK); 
            String stopwatch = "STOPWATCH";
            int textWidth_stopwatch = tft.textWidth(stopwatch);  // Tính chiều rộng của chuỗi
            int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2; // Căn giữa trên toàn bộ màn hình
            tft.setCursor(x_stopwatch, 200);
            tft.printf("%s", stopwatch);
            faceChange = false;
    }
}