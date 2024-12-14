#include "display.h"
#include "walkmpuApp.h"
#include "step_png.h"  
#include "foot_steps_result_png.h"
#include "ble.h"

MPU6050 mpu(0x69);

SimpleKalmanFilter kalmanFilterX(2, 2, 0.01);
SimpleKalmanFilter kalmanFilterY(2, 2, 0.01);
SimpleKalmanFilter kalmanFilterZ(2, 2, 0.01);

int16_t rc_walk;
int16_t rc_walk_result;
// Thông số phát hiện bước
float threshold = 1.2; // Ngưỡng để nhận diện bước (đơn vị: g)
unsigned long lastStepTime = 0; // Thời gian bước trước
unsigned long minStepInterval = 400; // Khoảng cách tối thiểu giữa hai bước (ms)
int stepCount = 0;
int distance = 0;

// Thêm các biến toàn cục mới
const int WINDOW_SIZE = 10; // Kích thước cửa sổ trượt
float accelValues[WINDOW_SIZE]; // Mảng lưu các giá trị gia tốc

// Điều chỉnh các thông số
const float THRESHOLD_LOW = 0.7;  // Ngưỡng dưới
const float THRESHOLD_HIGH = 1.4; // Ngưỡng trên
const unsigned long MIN_STEP_TIME = 200; // Thời gian tối thiểu giữa các bước (ms)

// Biến trạng thái
bool isStepUp = false;

void walkApp() {
  if (faceChange == true) {
    tft.fillScreen(TFT_BLACK);
    rc_walk_result = png.openFLASH((uint8_t *)foot_steps_result_png, sizeof(foot_steps_result_png), pngDraw);
    if (rc_walk_result == PNG_SUCCESS) {
        tft.startWrite();
        rc_walk_result = png.decode(NULL, 0);
        tft.endWrite();
    }
    faceChange = false;
  }
  lastWake = millis();
  float rawAx = mpu.getAccelerationX(); // Gia tốc thô theo trục X
  float rawAy = mpu.getAccelerationY(); // Gia tốc thô theo trục Y
  float rawAz = mpu.getAccelerationZ(); // Gia tốc thô theo trục Z

  printf("rawAx: %f\n", rawAx);
  printf("rawAy: %f\n", rawAy);
  printf("rawAz: %f\n", rawAz);

  float ax = rawAx / 16384.0;
  float ay = rawAy / 16384.0;
  float az = rawAz / 16384.0;

  // // Áp dụng bộ lọc Kalman cho từng trục
  // float filteredAx = kalmanFilterX.updateEstimate(ax);
  // float filteredAy = kalmanFilterY.updateEstimate(ay);
  // float filteredAz = kalmanFilterZ.updateEstimate(az);

  // printf("filteredAx: %f\n", filteredAx);
  // printf("filteredAy: %f\n", filteredAy);
  // printf("filteredAz: %f\n", filteredAz);

  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  // Tính giá trị trung bình của cửa sổ
  float sum = 0;
  for(int i = 0; i < WINDOW_SIZE; i++) {
      sum += accelValues[i];
  }
  float avgAccel = sum / WINDOW_SIZE;

  // Thuật toán phát hiện bước dựa trên đỉnh và đáy
  unsigned long currentTime = millis();

  if (!isStepUp && magnitude > THRESHOLD_HIGH) {
      isStepUp = true;
  } 
  else if (isStepUp && magnitude < THRESHOLD_LOW) {
      if (currentTime - lastStepTime > MIN_STEP_TIME) {
          stepCount++;
          lastStepTime = currentTime;
      }
      isStepUp = false;
  }

  // Debug - in ra các giá trị để kiểm tra
  printf("Magnitude: %f Steps: %d\n", magnitude, stepCount);

  distance = 0.4 * stepCount;

  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.setCursor(120, 30);
  tft.printf("%04d", stepCount);
  tft.setCursor(110, 130);
  tft.printf("%04dm", distance);
  tft.setTextSize(4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(60, 200);
  tft.printf("START");
}

void walkResult() {
  if (faceChange == true) {
    tft.fillScreen(TFT_BLACK);
    rc_walk_result = png.openFLASH((uint8_t *)foot_steps_result_png, sizeof(foot_steps_result_png), pngDraw);
    if (rc_walk_result == PNG_SUCCESS) {
        tft.startWrite();
        rc_walk_result = png.decode(NULL, 0);
        tft.endWrite();
    }
    tft.setTextSize(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.setCursor(120, 30);
    tft.printf("%04d", stepCount);
    tft.setCursor(110, 130);
    tft.printf("%04dm", distance);
    tft.setTextSize(4);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(80, 200);
    tft.printf("STOP");
    faceChange = false;
    dataSent = false;
  }

  if (!dataSent) {
    char bleData[10];
    snprintf(bleData, sizeof(bleData), "W%04d%04d", stepCount, distance);
    if (stepCount > 0) {
      writeBLEData(bleData);
    }
  }
  totalStep += stepCount;
  stepCount = 0;
  distance = 0;
}

void walkInitScreen() {
  if (faceChange == true) {
      tft.fillScreen(TFT_BLACK);
      rc_walk = png.openFLASH((uint8_t *)step_png, sizeof(step_png), pngDraw);
      if (rc_walk == PNG_SUCCESS) {
          tft.startWrite();
          rc_walk = png.decode(NULL, 0);
          tft.endWrite();
      }
      tft.setTextSize(3);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
      String stopwatch = "WALK APP";
      int textWidth_stopwatch = tft.textWidth(stopwatch);  // Tính chiều rộng của chuỗi
      int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2; // Căn giữa trên toàn bộ màn hình
      tft.setCursor(x_stopwatch, 200);
      tft.printf("%s", stopwatch);
      faceChange = false;
    }
}
