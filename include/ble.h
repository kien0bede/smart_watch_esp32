#ifndef __BLE_H__
#define __BLE_H__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

extern RTC_DATA_ATTR bool bt_disconnect;
extern bool newData;
extern String BT_IN;
extern String WT_IN;
extern BLECharacteristic *pCharacteristic;
extern bool deviceConnected;
extern bool dataSent;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void initBLE();
void bluetoothInitScreen();
void writeBLEData(const char* data);

#endif