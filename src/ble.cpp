#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "ble.h"
#include "display.h"
#include "bluetooth_available_png.h"
#include "bluetooth_not_available_png.h"

int16_t rc_bt;  
bool bt_disconnect = false;
bool newData = false;
String stopwatch = "";
String BT_IN = "";
String WT_IN = "";

BLECharacteristic *pCharacteristic;

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        BT_IN = pCharacteristic->getValue().c_str();
        WT_IN = pCharacteristic->getValue().c_str();
        newData = true;
    }
};

void initBLE() {
    BLEDevice::init("SmartWatch");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_READ |
                                            BLECharacteristic::PROPERTY_WRITE
                                        );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Tương thích với iPhone
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    bt_disconnect = false;
}

void bluetoothInitScreen() {
  if (faceChange == true) {
      tft.fillScreen(TFT_BLACK);
      if (bt_disconnect == false) {
            rc_bt = png.openFLASH((uint8_t *)bluetooth_available_png, sizeof(bluetooth_available_png), pngDraw);
            if (rc_bt == PNG_SUCCESS) {
                tft.startWrite();
                rc_bt = png.decode(NULL, 0);
                tft.endWrite();
            }
      } else {
          rc_bt = png.openFLASH((uint8_t *)bluetooth_not_available_png, sizeof(bluetooth_not_available_png), pngDraw);
            if (rc_bt == PNG_SUCCESS) {
                tft.startWrite();
                rc_bt = png.decode(NULL, 0);
                tft.endWrite();
            }
      }
      tft.setTextSize(3);
      tft.setTextColor(TFT_BLUE, TFT_BLACK); 
      if (bt_disconnect == false) {
        stopwatch = "BT ON";
      } else {
        stopwatch = "BT OFF";
      }
      int textWidth_stopwatch = tft.textWidth(stopwatch);
      int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
      tft.setCursor(x_stopwatch, 200);
      tft.printf("%s", stopwatch);
      faceChange = false;
    }
}