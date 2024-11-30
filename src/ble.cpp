#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "ble.h"

bool bt_disconnect = false;
bool newData = false;
String BT_IN = "";
String WT_IN = "";

BLECharacteristic *pCharacteristic;

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
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    bt_disconnect = false;
}