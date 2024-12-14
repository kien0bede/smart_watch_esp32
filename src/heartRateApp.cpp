#include "display.h"
#include "heartRate.h"
#include "heartRateApp.h"
#include "spo2_algorithm.h"
#include "heartrate_png.h"  
#include "heart_rate_result_png.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include "ble.h"

MAX30105 particleSensor;

// uint32_t irBuffer[100];
// uint32_t redBuffer[100];
// byte sampleCount = 0; // Biến đếm số mẫu đã thu thập

// // Biến lưu kết quả SPO2 và nhịp tim
// int32_t spo2;
// int8_t validSPO2;
// int32_t heartRate;
// int8_t validHeartRate;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;

double SpO2 = 0;
double ESpO2 = 60.0;
double FSpO2 = 0.7;
double frate = 0.95;
int i = 0;
int Num = 30;
#define FINGER_ON 7000
#define MINIMUM_SPO2 60.0
int16_t rc_heartapp;  
int16_t rc_heartresult;

#define BUFFER_SIZE     100    // Kích thước buffer mẫu
#define FILTER_THRESHOLD 50000 // Ngưỡng lọc nhiễu
#define MIN_VALID_IR    20000  // Ngưỡng IR tối thiểu

// Thêm ở phần đầu file
#define TOP_VALUES_COUNT 5
int topBeatValues[TOP_VALUES_COUNT] = {0}; // Mảng lưu 5 giá trị cao nhất

// Hàm thêm giá trị mới và sắp xếp lại mảng top values
void updateTopValues(int newValue) {
    // Chèn giá trị mới nếu nó lớn hơn giá trị nhỏ nhất trong top
    if (newValue > topBeatValues[TOP_VALUES_COUNT - 1]) {
        topBeatValues[TOP_VALUES_COUNT - 1] = newValue;
        
        // Sắp xếp lại mảng (bubble sort)
        for (int i = TOP_VALUES_COUNT - 1; i > 0; i--) {
            if (topBeatValues[i] > topBeatValues[i-1]) {
                int temp = topBeatValues[i];
                topBeatValues[i] = topBeatValues[i-1];
                topBeatValues[i-1] = temp;
            }
        }
    }
}

// Hàm tính trung bình 5 giá trị cao nhất
int calculateTopAverage() {
    int sum = 0;
    for (int i = 0; i < TOP_VALUES_COUNT; i++) {
        sum += topBeatValues[i];
    }
    return sum / TOP_VALUES_COUNT;
}

void heartRateApp() {
  static unsigned long startTime = 0; // Thời gian bắt đầu đo
  static bool measuring = false; // Cờ để kiểm tra xem có đang đo không
  static bool validMeasurement = false; 

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
  lastWake = millis();
  long irValue = particleSensor.getIR();
  if (irValue > FINGER_ON ) {
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
        if (beatAvg > 30) {
          updateTopValues(beatAvg);
        }
        if (beatAvg > 30 && !measuring) {
          startTime = millis();
          measuring = true;
          validMeasurement = true;
        }
      }
    }
    uint32_t ir, red ;
    double fred, fir;
    particleSensor.check();
    if (particleSensor.available()) {
      i++;
      ir = particleSensor.getFIFOIR();
      red = particleSensor.getFIFORed();
      fir = (double)ir;
      fred = (double)red;
      aveir = aveir * frate + (double)ir * (1.0 - frate);
      avered = avered * frate + (double)red * (1.0 - frate);
      sumirrms += (fir - aveir) * (fir - aveir);
      sumredrms += (fred - avered) * (fred - avered);

      if ((i % Num) == 0) {
        double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
        SpO2 = -45.060 * R * R + 30.354 * R + 94.845;
        ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
        if (ESpO2 <= MINIMUM_SPO2) ESpO2 = MINIMUM_SPO2;
        if (ESpO2 > 100) ESpO2 = 99.9;
        sumredrms = 0.0; sumirrms = 0.0; SpO2 = 0;
        i = 0;
      }
      particleSensor.nextSample();
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 155);
    tft.printf("%03d", int(beatAvg));
    tft.setCursor(170, 155);
    if (beatAvg > 30) {
      tft.printf("%03d", int(ESpO2));
    } else {
      tft.printf("000");
    }
    tft.setTextSize(4);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(40, 200);
    tft.printf("WAIT...");
  }

  if (measuring && validMeasurement && (millis() - startTime >= 10000)) {
    measuring = false;
    validMeasurement = false;
    Screen = 5;
    subScreen = 1;
    faceChange = true;
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
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(20, 155);
    int avgTopBeats = calculateTopAverage();
    tft.printf("%03d", int(avgTopBeats));
    tft.setCursor(170, 155);
    tft.printf("%03d", int(ESpO2));
    tft.setTextSize(4);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(80, 200);
    tft.printf("STOP");
    preBPM = int(avgTopBeats);
    faceChange = false;
    dataSent = false;  // Reset flag khi vào màn hình mới
  }

  if (!dataSent) {  // Chỉ gửi nếu chưa gửi
    char bleData[11];
    snprintf(bleData, sizeof(bleData), "HR%03d%03d", calculateTopAverage(), int(ESpO2));
    writeBLEData(bleData);
  }

  for (byte rx = 0 ; rx < RATE_SIZE ; rx++) {
    rates[rx] = 0;
  }
  // Reset các giá trị
  for (int i = 0; i < TOP_VALUES_COUNT; i++) {
      topBeatValues[i] = 0;
  }
  beatAvg = 0; rateSpot = 0; lastBeat = 0;
  avered = 0; aveir = 0; sumirrms = 0; sumredrms = 0;
  SpO2 = 0; ESpO2 = 90.0;
}