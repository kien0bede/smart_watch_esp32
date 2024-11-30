#include "display.h"
#include "heartRate.h"
#include "heartRateApp.h"
#include "spo2_algorithm.h"
#include "heartrate_png.h"  

MAX30105 particleSensor;

uint32_t irBuffer[100];
uint32_t redBuffer[100];
byte sampleCount = 0; // Biến đếm số mẫu đã thu thập

// Biến lưu kết quả SPO2 và nhịp tim
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;
int16_t rc_heartapp;  

void heartRateApp() {
  // if (faceChange == true) {
  //       tft.fillScreen(TFT_BLACK);
  //       faceChange = false;
  // }

  // tft.setTextSize(3);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.setCursor(10, 10);

  // tft.printf("Please Wait...");
  // // Kiểm tra có dữ liệu mới từ cảm biến
  // if (particleSensor.check() != 0) {
  //   // Đọc dữ liệu và lưu vào bộ đệm
  //   redBuffer[sampleCount] = particleSensor.getRed();
  //   irBuffer[sampleCount] = particleSensor.getIR();
  //   sampleCount++;
  //   particleSensor.nextSample(); // Đi đến mẫu tiếp theo
  //   lastWake = millis();

  //   // Khi đã thu thập đủ 100 mẫu
  //   if (sampleCount == 100) {
  //     // Gọi thuật toán tính SPO2 và nhịp tim
  //     maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //     // In kết quả
  //     if (validHeartRate && validSPO2) {
  //       Screen = 5;
  //       subScreen = 1;
  //     } else {
  //       printf("Dữ liệu không hợp lệ.\n");
  //     }
  //     sampleCount = 0;
  //   }
  // }
}

void heartRateInitScreen() {
  if (faceChange == true) {
    tft.fillScreen(TFT_BLACK);
    rc_heartapp = png.openFLASH((uint8_t *)heartrate_png, sizeof(heartrate_png), pngDraw);
    if (rc_heartapp == PNG_SUCCESS) {
      tft.startWrite();
      rc_heartapp = png.decode(NULL, 0);
      tft.endWrite();
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, TFT_BLACK); 
    String stopwatch = "HEART RATE";
    int textWidth_stopwatch = tft.textWidth(stopwatch);  // Tính chiều rộng của chuỗi
    int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2; // Căn giữa trên toàn bộ màn hình
    tft.setCursor(x_stopwatch, 200);
    tft.printf("%s", stopwatch);
    faceChange = false;
  }
}

void heartRateResultScreen() {
  // if (faceChange == true) {
  //   tft.fillScreen(TFT_BLACK);
  //   faceChange = false;
  // }
  // tft.setTextSize(3);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.setCursor(10, 10);
  // tft.printf("BPM: %d, SPO2: %d\n", heartRate, spo2);
}