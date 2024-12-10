#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "ble.h"
#include "display.h"
#include "bluetooth_available_png.h"
#include "bluetooth_not_available_png.h"

int16_t rc_bt;  
RTC_DATA_ATTR bool bt_disconnect = false;
bool dataSent = false;
bool newData = false;
String stopwatch = "";
String BT_IN = "";
String WT_IN = "";

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string value = pCharacteristic->getValue();  // Lưu giá trị Bluetooth nhận được
        if (value.length() == 13) {  // Kiểm tra độ dài chuỗi
            BT_IN = String(pCharacteristic->getValue().c_str());
            WT_IN = String(pCharacteristic->getValue().c_str());
            newData = true;
            printf("Data received: %s\n", BT_IN.c_str());  // Debug thông tin nhận
        } else {
            printf("Error: Invalid data length.\n");
        }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        printf("Bluetooth connected!\n");
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        printf("Bluetooth disconnected!\n");
        // Khởi động lại advertising khi bị ngắt kết nối
        pServer->startAdvertising();
    }
};

void writeBLEData(const char* data) {
    if (dataSent) return;  // Nếu đã gửi thành công rồi thì return luôn
    
    printf("Connection status: %d, Characteristic: %p\n", deviceConnected, pCharacteristic);
    if (deviceConnected && pCharacteristic != NULL) {
        try {
            pCharacteristic->setValue((uint8_t*)data, strlen(data));
            pCharacteristic->notify();
            printf("Sent BLE data: %s\n", data);
            delay(10);
            dataSent = true;  // Đánh dấu là đã gửi thành công
        } catch (...) {
            printf("Error sending BLE data\n");
        }
    } else {
        printf("Device not connected or characteristic not initialized\n");
    }
}

void initBLE() {
    BLEDevice::init("SmartWatch");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    // Bỏ từ khóa BLECharacteristic* để sử dụng biến global
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE | 
                        BLECharacteristic::PROPERTY_NOTIFY
                    );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pServer->setCallbacks(new MyServerCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    if (bt_disconnect == true) {
        BLEDevice::stopAdvertising();
    } else {
        BLEDevice::startAdvertising();
    }
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