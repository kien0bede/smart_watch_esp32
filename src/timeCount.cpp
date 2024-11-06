#include <TFT_eSPI.h>
#include <OneButton.h>
#include "display.h"
#include "timeCount.h"

bool isRunning = false;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;

void displayTime(unsigned long timeInMillis) {
    unsigned long minutes = (timeInMillis / 60000) % 60;
    unsigned long seconds = (timeInMillis / 1000) % 60;
    unsigned long millisec = timeInMillis % 1000;

    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }

    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);

    tft.printf("%02lu:%02lu:%03lu", minutes, seconds, millisec);
}

void startStop() {
    if (isRunning) {
        isRunning = false;
        elapsedTime += millis() - startTime;
    } else {
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
        currentTime = millis() - startTime + elapsedTime;
        displayTime(currentTime);
    } else {
        displayTime(elapsedTime);
    }
}

void timeCountInitScreen() {
    if (faceChange == true) {
            tft.fillScreen(TFT_BLACK);
            faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("TIME COUNT\n");
}
