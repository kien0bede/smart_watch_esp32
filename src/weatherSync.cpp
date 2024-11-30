#include <TFT_eSPI.h>
#include <OneButton.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "display.h"
#include "timeSync.h"
#include <ble.h>

int minTemp = 0;
int maxTemp = 0;
String weatherCondition;

void weatherSyncInitScreen(){
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Weather Sync");
}

void weatherSyncApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Weather Sync Begin");

    int counter = 1;
    if (newData && WT_IN.length() == 6) {
        newData = false;
        pCharacteristic->setValue("");

        String weatherData = WT_IN;

        int minTemp = weatherData.substring(0, 2).toInt();  // "23" => 23
        int maxTemp = weatherData.substring(2, 4).toInt();  // "34" => 34
        String weatherCondition = weatherData.substring(4);  // "Na" => "Na"
        counter = 0;
    }

    if (counter == 0) {
        tft.setCursor(10, 50);
        tft.printf("Weather Sync Complete");
        delay(2000);
        Screen = 0;
        subScreen = 0;
    }
}

void weatherApp() {
    if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Min Temp: %d C", minTemp);
    tft.setCursor(10, 50);
    tft.printf("Max Temp: %d C", maxTemp);

    if (weatherCondition == "Na") {
        tft.setCursor(10, 90);
        tft.printf("Condition: Sunny");
    } else if (weatherCondition == "Mu") {
        tft.setCursor(10, 90);
        tft.printf("Condition: Rainy");
    } else if (weatherCondition == "Ma") {
        tft.setCursor(10, 90);
        tft.printf("Condition: Cloudy");
    } else {
        tft.setCursor(10, 90);
        tft.printf("Condition: Unknown");
    }
}