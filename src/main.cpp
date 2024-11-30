#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <MPU6050.h>
#include <MAX30105.h>
#include "heartRate.h"
#include <OneButton.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <time.h>
#include "esp_sleep.h"
#include "esp_system.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RTClib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "timeCount.h"
#include "display.h"
#include "heartRateApp.h"
#include "timeSync.h"
#include "ble.h"
#include <PNGdec.h>
#include "watchscreen.h" 
#include "bluetooth16.h"
#include "counttime.h"
#include "heartrate_png.h"

#define MAX_IMAGE_WIDTH 240

int16_t xpos;
int16_t ypos;

int16_t bg_xpos = 0;
int16_t bg_ypos = 0;

int16_t bt_xpos = 150;
int16_t bt_ypos = 0;

int16_t ax, ay, az;
int16_t gx, gy, gz;

#define SCL_PIN 40
#define SDA_PIN 41
#define BUTTON_PIN 12

RTC_DATA_ATTR int boot_count = 0;

RTC_PCF8563 rtc;
TFT_eSPI tft = TFT_eSPI();
PNG png; 
QMC5883LCompass compass;
MPU6050 mpu(0x69);

OneButton button(BUTTON_PIN, true);

int current_sensor = 1;
bool button_pressed = false;
bool heart_sensor_on = true;

unsigned long lastPressed = 0;
unsigned long lastWake = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long pressStartTime;
int pressState = 0;
int Screen = 0;
int subScreen = 0;
bool faceChange = true;

#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)
#define WAKEUP_GPIO_2              GPIO_NUM_2
#define WAKEUP_GPIO_12              GPIO_NUM_12

uint64_t bitmask = BUTTON_PIN_BITMASK(WAKEUP_GPIO_2) | BUTTON_PIN_BITMASK(WAKEUP_GPIO_12);

void IRAM_ATTR checkTicks() {
  button.tick();
}

boolean motionDetected = false;

void IRAM_ATTR doInt() {
  motionDetected = true;  
}

void enter_sleep()
{
  digitalWrite(TFT_BLK_PIN, LOW);
  delay(100);
  rtc_gpio_hold_en((gpio_num_t) TFT_BLK_PIN);
  rtc_gpio_hold_en(GPIO_NUM_12);
  esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
  esp_deep_sleep_start();
}

void ShortClick() {
  printf("singleClick() detected.\n");
  unsigned long currentMillis = millis();
  lastWake = currentMillis;
  lastDisplayUpdate = currentMillis;
  lastPressed = currentMillis;

  if (Screen == 0) {
    subScreen++;
    
    if (subScreen > 3) {
      subScreen = 0;
      faceChange = true;
    }

    if (subScreen == 1) {
      // particleSensor.shutDown();
      faceChange = true;
    } else if (subScreen == 2) {
      // particleSensor.shutDown();
      faceChange = true;
    } else if (subScreen == 3) {
      // particleSensor.shutDown();
      faceChange = true;
    }
  }
  if (Screen == 1) {
    subScreen++;
    if (subScreen > 2) {
      subScreen = 0;
      faceChange = true;
    }
  }
  if (Screen == 4) {
    startStop();
  }
  pressState = 1;
}

void LongPress() {
  printf("pressStart()\n");
  pressStartTime = millis() - 1000;
  lastWake = millis();
  lastDisplayUpdate = millis();
  if (Screen == 0) {
    if (subScreen == 0) {
      Screen = 1;
      subScreen = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 1) {
      Screen = 4;
      subScreen = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 2) {
      Screen = 5;
      subScreen = 0;
      particleSensor.wakeUp();
      faceChange = true;
      return;
    }
  }
  if (Screen == 1) {
    if (subScreen == 0) {
      Screen = 2;
      subScreen = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 1) {
      Screen = 3;
      subScreen = 0;
      return;
    }
    if (subScreen == 2) {
      Screen = 0;
      subScreen = 0;
      faceChange = true;
      return;
    }
  }
  if (Screen == 2) {
    Screen = 0;
    subScreen = 0;
    faceChange = true;
    return;
  }
  if (Screen == 4) {
    resetTime();
    faceChange = true;
    Screen = 0;
    subScreen = 1;
    return;
  }
  if (Screen == 5) {
    particleSensor.shutDown();
    Screen = 0;
    subScreen = 2;
    faceChange = true;
    return;
  }
  pressState = 1;
  lastPressed = millis();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  // Init BT
  initBLE();
  // Init cảm biến gia tốc
  mpu.initialize(ACCEL_FS::A16G, GYRO_FS::G2000DPS);

  mpu.setAccelerometerPowerOnDelay(3);

  mpu.setIntFreefallEnabled(false);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(true);

  mpu.setInterruptMode(1);

  mpu.setDHPFMode(1);

  mpu.setMotionDetectionThreshold(10);
  mpu.setMotionDetectionDuration(2);	

  attachInterrupt(GPIO_NUM_2, doInt, RISING);
  // Init la ban
  compass.init();
  // Init nhip tim
  // particleSensor.begin();
  // particleSensor.setup();
  // particleSensor.setPulseAmplitudeRed(0x0A);
  // particleSensor.setPulseAmplitudeIR(0x0A);
  // Init button
  button.attachClick(ShortClick);
  button.attachLongPressStart(LongPress);
  // Khởi tạo TFT
  tft.init();
  tft.setRotation(0);  // Điều chỉnh hướng màn hình
  tft.fillScreen(TFT_BLACK);  // Đặt màu nền là đen
  tft.setTextColor(TFT_WHITE);  // Đặt màu chữ là trắng
  tft.setTextSize(2);  // Kích thước chữ

  rtc_gpio_hold_dis((gpio_num_t) TFT_BLK_PIN);
  pinMode(TFT_BLK_PIN, OUTPUT);
  digitalWrite(TFT_BLK_PIN, HIGH);

  if(!rtc.begin()) {
    printf("Không thể kết nối với PCF8563!\n");
  }

  boot_count++;
  printf("Số lần khởi động: %d\n", boot_count);

  if (boot_count == 1) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    rtc.start();
  }
  else {
    DateTime rtcTime = rtc.now();
    rtc.start();
    printf(rtcTime.timestamp().c_str());
  }
  delay(100);
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

int16_t rc_bluetooth;
int16_t rc_watchscreen;
void WatchFaceScreen() {
  xpos = bg_xpos;
  ypos = bg_ypos;
  /* Hiển thị background */
  rc_watchscreen = png.openFLASH((uint8_t *)watchscreen, sizeof(watchscreen), pngDraw);
  if (rc_watchscreen == PNG_SUCCESS) {
    tft.startWrite();
    rc_watchscreen = png.decode(NULL, 0);
    tft.endWrite();
  }
}

void showBluetoothIcon() {
  xpos = bt_xpos;
  ypos = bt_ypos;
  
  rc_bluetooth = png.openFLASH((uint8_t *)bluetooth16, sizeof(bluetooth16), pngDraw);
  if (rc_bluetooth == PNG_SUCCESS) {
    tft.startWrite();
    rc_bluetooth = png.decode(NULL, 0);
    tft.endWrite();
  }
  // Đặt điểm vẽ về 0:0
  xpos = 0;
  ypos = 0;
}

void hideBluetoothIcon() {
  tft.fillRect(bt_xpos, bt_ypos, 32, 32, TFT_BLACK); // Xóa icon bằng cách vẽ nền đen
}

void watchFace() {
  DateTime now = rtc.now();

  if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        WatchFaceScreen();
        faceChange = false;
  }
  if (bt_disconnect == false) {
        showBluetoothIcon();
  } else {
        hideBluetoothIcon();
  }
  // Hiển thị giờ phút giây
  tft.setCursor(35, 50);
  tft.setTextSize(6);
  tft.setTextColor(TFT_CYAN, TFT_BLACK); 
  tft.printf("%02d:%02d", 
    now.hour(),    // Giờ
    now.minute()  // Phút
  );

  // Hiển thị ngày tháng năm
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Tạo chuỗi ngày/tháng/năm
  String dateStr = String(now.day(), DEC) + "/" + 
                  String(now.month(), DEC) + "/" + 
                  String(now.year(), DEC);

  int16_t textWidth = tft.textWidth(dateStr.c_str()); // Lấy chiều rộng của văn bản
  // Tính toán vị trí căn lề cách đều
  int16_t xpos = (tft.width() - textWidth) / 2;  // Căn giữa
  tft.setCursor(xpos, 110); // Vị trí trên màn hình (y có thể thay đổi tùy nhu cầu)
  tft.print(dateStr);

  /* Hiển thị Walk */
  tft.setCursor(40, 180);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("WALK");
  /* Hiển thị BPM */
  tft.setCursor(160, 180);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("BPM");

  int walk = 195;
  /* Căn chỉnh tự động theo kích thước chuỗi */
  char walkStr[5];
  sprintf(walkStr, "%04d", walk);
  int textWidth_walk = tft.textWidth(walkStr); 
  int x_walk = 5 + (textWidth_walk / 2);
  tft.setCursor(x_walk, 210);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("%s", walkStr);

  int bpm = 98;
  /* Căn chỉnh tự động theo kích thước chuỗi */
  char bpmStr[4];
  sprintf(bpmStr, "%03d", bpm);
  int textWidth_Bpm = tft.textWidth(bpmStr); 
  int xBpm = 125 + (textWidth_Bpm / 2);
  tft.setCursor(xBpm, 210);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("%s", bpmStr); 
}

void compassApp() {
  compass.read();
  int x = compass.getX();
  int y = compass.getY();
  int z = compass.getZ();
  printf("Compass X: %d, Y: %d, Z: %d\n", x, y, z);
}

void watchtask() {
  if (pressState == 1 && digitalRead(0) == 1) {
    pressState = 0;
  }
  if (millis() - lastWake > 10000) {
    enter_sleep();
  }
  if (Screen == 0) {
    if (subScreen == 0) {
      watchFace();
    } else if (subScreen == 1) {
      timeCountInitScreen();
    } else if (subScreen == 2) {
      heartRateInitScreen();
    } else if (subScreen == 3) {
      compassApp();
    }
  }
  if (Screen == 1) {
    if (subScreen == 0) {
      timeSyncInitScreen();
    } else if (subScreen == 1) {
      printf("Screen 1 1\n");
    } else if (subScreen == 2) {
      printf("OUT\n");
    }
  }
  if (Screen == 2) {
    if (subScreen == 0) {
      timeSyncApp();
    }
  }
  if (Screen == 3) {
    if (subScreen == 0) {
      if (bt_disconnect == false) {
        BLEDevice::stopAdvertising();
        printf("BT đã ngắt kết nối!\n");
        bt_disconnect = true;
      } else {
        BLEDevice::startAdvertising();
        printf("BT đã kết nối!\n");
        bt_disconnect = false;
      }
      delay(2000);
      Screen = 1;
      subScreen = 1;
    }
  }
  if (Screen == 4) {
    timeCountApp();
  }
  if (Screen == 5) {
    if (subScreen == 0) {
      heartRateApp();
    }
    if (subScreen == 1) {
      heartRateResultScreen();
    }
  }
}

void loop() {
  button.tick();
  watchtask();
}
