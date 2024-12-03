#include "display.h"
#include "heartRate.h"
#include "heartRateApp.h"
#include "spo2_algorithm.h"
#include "heartrate_png.h"  
#include "heart_rate_result_png.h"

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
int16_t rc_heartresult;

void heartRateApp() {
  if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        rc_heartresult = png.openFLASH((uint8_t *)heart_rate_result_png, sizeof(heart_rate_result_png), pngDraw);
        if (rc_heartresult == PNG_SUCCESS) {
            tft.startWrite();
            rc_heartresult = png.decode(NULL, 0);
            tft.endWrite();
        }
        tft.setTextSize(4);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor(60, 200);
        tft.printf("START");
        faceChange = false;
  }
  // Kiểm tra có dữ liệu mới từ cảm biến
  if (particleSensor.check() != 0) {
    // Đọc dữ liệu và lưu vào bộ đệm
    redBuffer[sampleCount] = particleSensor.getRed();
    irBuffer[sampleCount] = particleSensor.getIR();
    sampleCount++;
    particleSensor.nextSample(); // Đi đến mẫu tiếp theo
    lastWake = millis();

    // Khi đã thu thập đủ 100 mẫu
    if (sampleCount == 100) {
      // Gọi thuật toán tính SPO2 và nhịp tim
      maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

      // In kết quả
      if (heartRate < 180 && spo2 > 90) {
        Screen = 5;
        subScreen = 1;
        faceChange = true;
      } else {
        printf("Dữ liệu không hợp lệ.\n");
      }
      sampleCount = 0;
    }
  }
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
  if (faceChange == true) {
    particleSensor.shutDown();
    tft.fillScreen(TFT_BLACK);
    rc_heartresult = png.openFLASH((uint8_t *)heart_rate_result_png, sizeof(heart_rate_result_png), pngDraw);
    if (rc_heartresult == PNG_SUCCESS) {
        tft.startWrite();
        rc_heartresult = png.decode(NULL, 0);
        tft.endWrite();
    }
    faceChange = false;
  }
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(20, 155);
  tft.printf("%d", heartRate);
  tft.setCursor(170, 155);
  tft.printf("%d", spo2);
  tft.setTextSize(4);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(80, 200);
  tft.printf("STOP");
  preBPM = heartRate;
}