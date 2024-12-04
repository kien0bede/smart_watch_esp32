#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
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
#include "compassApp.h"
#include "timeSync.h"
#include "ble.h"
#include <PNGdec.h>
#include "watchscreen.h" 
#include "bluetooth16.h"
#include "counttime.h"
#include "heartrate_png.h"
#include "sync_png.h"
#include "exit_png.h"
#include "alarmApp.h"
#include "icon_alarm_png.h"
#include "PCF8563.h"
#include "walkmpuApp.h"
#include "watchSetting.h"

UBYTE buf[10];

int GPIO_reason;

// Định nghĩa tần số các nốt nhạc (Hz)
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659

// Giai điệu Jingle Bells (danh sách các nốt nhạc và độ dài nốt)
int melody[] = {
  NOTE_E4, NOTE_E4, NOTE_E4, // Jingle bells
  NOTE_E4, NOTE_E4, NOTE_E4, // Jingle bells
  NOTE_E4, NOTE_G4, NOTE_C4, NOTE_D4, NOTE_E4, // Jingle all the way
  NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, // Oh what fun
  NOTE_F4, NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4, // It is to ride
  NOTE_E4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_G4 // In a one-horse open sleigh
};

// Độ dài của mỗi nốt (1 = nguyên nốt, 2 = nửa nốt, 4 = một phần tư, ...)
int noteDurations[] = {
  4, 4, 2, // Jingle bells
  4, 4, 2, // Jingle bells
  4, 4, 4, 4, 2, // Jingle all the way
  4, 4, 4, 4, // Oh what fun
  4, 4, 4, 4, 4, // It is to ride
  4, 4, 4, 4, 2 // In a one-horse open sleigh
};

#define MAX_IMAGE_WIDTH 240
// Các toạ độ cho background và icon Bluetooth
int16_t xpos;
int16_t ypos;

int16_t bg_xpos = 0;
int16_t bg_ypos = 0;

int16_t bt_xpos = 170;
int16_t bt_ypos = 0;

int16_t alarm_xpos = 145;
int16_t alarm_ypos = 0;

int16_t rc_exit;

#define SCL_PIN 40
#define SDA_PIN 41
#define BUTTON_PIN 12

RTC_DATA_ATTR int boot_count = 0;
RTC_DATA_ATTR int totalStep = 0;
RTC_DATA_ATTR int preBPM = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

PNG png; 

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
bool wifi_disconnect = true;

RTC_DATA_ATTR int duration = 2;
RTC_DATA_ATTR int brightness = 2;

#define ANALOG_PIN 1
int sensorValue;
int bat_percentage;
float vRef = 3.3;
float R1 = 200000.0;
float R2 = 100000.0;

int buzzerPin = 45;

const int rtc_int = 21;

volatile bool alarm_ready = false;

void IRAM_ATTR onInterruptRTC() {
  alarm_ready = true;
}

#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)  // 2 ^ GPIO_NUMBER in hex
#define WAKEUP_GPIO_2              GPIO_NUM_2     // Only RTC IO are allowed - ESP32 Pin example
#define WAKEUP_GPIO_12             GPIO_NUM_12    // Only RTC IO are allowed - ESP32 Pin example
#define WAKEUP_GPIO_21             GPIO_NUM_21    // INT từ PCF8563

// Define bitmask for multiple GPIOs
uint64_t bitmask = BUTTON_PIN_BITMASK(WAKEUP_GPIO_2) | BUTTON_PIN_BITMASK(WAKEUP_GPIO_12) | BUTTON_PIN_BITMASK(WAKEUP_GPIO_21);

void IRAM_ATTR checkTicks() {
  button.tick();
}

void enter_sleep()
{
  analogWrite(TFT_BLK_PIN, 0);
  delay(100);
  rtc_gpio_hold_en((gpio_num_t) TFT_BLK_PIN);
  rtc_gpio_hold_en(GPIO_NUM_21);
  rtc_gpio_hold_en(GPIO_NUM_12);
  esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
  esp_deep_sleep_start();
}

void print_GPIO_wake_up(){
  GPIO_reason = (int)(log(esp_sleep_get_ext1_wakeup_status()) / log(2));
  printf("GPIO that triggered the wake up: GPIO %d\n", GPIO_reason);
}

void ShortClick() {
  printf("singleClick() detected.\n");
  unsigned long currentMillis = millis();
  lastWake = currentMillis;
  lastDisplayUpdate = currentMillis;
  lastPressed = currentMillis;

  if (Screen == 0) {
    subScreen++;
    
    if (subScreen > 4) {
      subScreen = 0;
      faceChange = true;
    }

    if (subScreen == 1) {
      faceChange = true;
    } else if (subScreen == 2) {
      faceChange = true;
    } else if (subScreen == 3) {
      faceChange = true;
    } else if (subScreen == 4) {
      faceChange = true;
    }
  }
  if (Screen == 1) {
    subScreen++;
    faceChange = true;
    if (subScreen > 4) {
      subScreen = 0;
      faceChange = true;
    }
  }
  if (Screen == 4) {
    startStop();
  }
  if (Screen == 7) {
    subScreen++;
    faceChange = true;
    if (subScreen > 3) {
      subScreen = 0;
      faceChange = true;
    }
  }
    if (Screen == 8) {
    Screen = 0;
    subScreen = 0;
    faceChange = true;
    return;
  }
  if (Screen == 10) {
    subScreen++;
    faceChange = true;
    if (subScreen > 2) {
      subScreen = 0;
      faceChange = true;
    }
  }
  pressState = 1;
}

void LongPress() {
  printf("pressStart()\n");
  pressStartTime = millis() - 1000;  // as set in setPressTicks()
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
    if (subScreen == 3)
    {
      Screen = 6;
      subScreen = 0;
      tft.setFreeFont(0);
      xpos = 0;
      ypos = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 4) {
      Screen = 9;
      subScreen = 0;
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
      faceChange = true;
      return;
    }
    if (subScreen == 2) {
      Screen = 7;
      subScreen = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 3) {
      Screen = 10;
      subScreen = 0;
      faceChange = true;
      return;
    }
    if (subScreen == 4) {
      Screen = 0;
      subScreen = 0;
      faceChange = true;
      return;
    }
  }
  if (Screen == 3) {
    Screen = 1;
    subScreen = 1;
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
    Screen = 0;
    subScreen = 2;
    faceChange = true;
    return;
  }
  if (Screen == 6) {
    Screen = 0;
    subScreen = 3;
    faceChange = true;
    return;
  }
  if (Screen == 7) {
    if (subScreen == 0) {
      hour_alarm++;
      if (hour_alarm > 23) {
        hour_alarm = 0;
      }
    } else if (subScreen == 1) {
      minute_alarm++;
      if (minute_alarm > 59) {
        minute_alarm = 0;
      }
    } else {
      if (subScreen == 2) {
        PCF8563_Set_Alarm(hour_alarm, minute_alarm);
        PCF8563_Alarm_Enable();
        alarm_on = true;
      }
      else {
        PCF8563_Alarm_Disable();
        alarm_on = false;
      }
      Screen = 1;
      subScreen = 2;
      faceChange = true;
      return;
    }
  }
  if (Screen == 9) {
    if (subScreen == 0) {
      Screen = 9;
      subScreen = 1;
      faceChange = true;
      return;
    } else if (subScreen == 1) {
      Screen = 0;
      subScreen = 4;
      faceChange = true;
      return;
    }
  }
  if (Screen == 10) {
    if (subScreen == 0) {
      duration++;
      faceChange = true;
      if (duration > 5) {
        duration = 1;
        faceChange = true;
      }
    } else if (subScreen == 1) {
      brightness++;
      faceChange = true;
      if (brightness > 5) {
        brightness = 1;
        faceChange = true;
      }
    } else if (subScreen == 2) {
      Screen = 1;
      subScreen = 3;
      duration_brightness = int_duration_brightness;
      analogWrite(TFT_BLK_PIN, brightness_level);
      faceChange = true;
      return;
    }
  }
  pressState = 1;
  lastPressed = millis();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  // Init gia toc
  mpu.initialize();
  mpu.setIntMotionEnabled(true);
  mpu.setMotionDetectionThreshold(30);
  mpu.setMotionDetectionDuration(50);
  // Init BT
  initBLE();
  // Init la ban
  compassInit();
  // Init nhip tim
  particleSensor.begin();
  particleSensor.setup(); // Thiết lập với cấu hình mặc định
  particleSensor.setPulseAmplitudeRed(0x0A); // Độ sáng LED đỏ thấp
  particleSensor.setPulseAmplitudeIR(0x0A);  // Độ sáng LED hồng ngoại thấp
  particleSensor.shutDown();
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
  analogWrite(TFT_BLK_PIN, brightness_level);

  attachInterrupt(INT_PIN, onInterruptRTC, FALLING);

  pinMode(buzzerPin, OUTPUT);

  boot_count++;
  printf("Count times: %d\n", boot_count);

  print_GPIO_wake_up();

  if (GPIO_reason == 21) {
    Screen = 8;
    subScreen = 0;
  }

  if (boot_count == 1) {
    PCF8563_Init();
    PCF8563_Set_Time(0, 0, 0);
    PCF8563_Set_Days(2025, 10, 10);
    PCF8563_Get_Time(buf);
    PCF8563_Get_Days(&buf[3]);
  } else {
    PCF8563_Init();
    PCF8563_Get_Time(buf);
    PCF8563_Get_Days(&buf[3]);
    if (alarm_on == true) {
      PCF8563_Set_Alarm(hour_alarm, minute_alarm);
      PCF8563_Alarm_Enable();
    }
  }

  if (boot_count == 1) {
    hour_alarm = 0;
    minute_alarm = 0;

    PCF8563_Set_Alarm(hour_alarm, minute_alarm);
    
    PCF8563_Alarm_Disable();

    alarm_on = false;
  }
  delay(100);
}

void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

int16_t rc_bluetooth;
int16_t rc_alrm;
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
  xpos = 0;
  ypos = 0;
}

void hideBluetoothIcon() {
  tft.fillRect(bt_xpos, bt_ypos, 25, 25, TFT_BLACK); // Xóa icon bằng cách vẽ nền đen
}

void showAlarmIcon() {
  xpos = alarm_xpos;
  ypos = alarm_ypos;
  
  rc_alrm = png.openFLASH((uint8_t *)icon_alarm_png, sizeof(icon_alarm_png), pngDraw);
  if (rc_alrm == PNG_SUCCESS) {
    tft.startWrite();
    rc_alrm = png.decode(NULL, 0);
    tft.endWrite();
  }
  xpos = 0;
  ypos = 0;
}

void hideAlarmIcon() {
  tft.fillRect(alarm_xpos, alarm_ypos, 25, 25, TFT_BLACK); // Xóa icon bằng cách vẽ nền đen
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void watchFace() {
  PCF8563_Get_Time(buf);
  PCF8563_Get_Days(&buf[3]);
  sensorValue = analogRead(ANALOG_PIN);

  float voltage = (float)sensorValue * (vRef / 4095.0);

  float actualVoltage = voltage * ((R1 + R2) / R2);
 
  bat_percentage = mapfloat(actualVoltage, 2.8, 4.2, 0, 100);
 
  if (bat_percentage >= 100)
  {
    bat_percentage = 100;
  }
  if (bat_percentage <= 0)
  {
    bat_percentage = 1;
  }

  tft.setCursor(200, 4);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.printf(" %02d", bat_percentage);

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

  if (alarm_on == true) {
    showAlarmIcon();
  } else {
    hideAlarmIcon();
  }
  
  // Hiển thị giờ phút giây
  tft.setCursor(35, 50);
  tft.setTextSize(6);
  tft.setTextColor(TFT_CYAN, TFT_BLACK); 
  tft.printf("%02d:%02d", 
    buf[2],    // Giờ
    buf[1]  // Phút
  );

  // Hiển thị ngày tháng năm
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Tạo chuỗi ngày/tháng/năm
  String year = String(buf[6]) + String(buf[5] < 10 ? "0" + String(buf[5]) : String(buf[5]));
  String month = (buf[4] < 10 ? "0" : "") + String(buf[4]);
  String day = (buf[3] < 10 ? "0" : "") + String(buf[3]);

  String dateStr = day + "/" + month + "/" + year;

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

  /* Căn chỉnh tự động theo kích thước chuỗi */
  char walkStr[5];
  sprintf(walkStr, "%04d", totalStep);
  int textWidth_walk = tft.textWidth(walkStr); 
  int x_walk = 5 + (textWidth_walk / 2);
  tft.setCursor(x_walk, 210);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("%s", walkStr);

  /* Căn chỉnh tự động theo kích thước chuỗi */
  char bpmStr[4];
  sprintf(bpmStr, "%03d", preBPM);
  int textWidth_Bpm = tft.textWidth(bpmStr); 
  int xBpm = 125 + (textWidth_Bpm / 2);
  tft.setCursor(xBpm, 210);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.printf("%s", bpmStr); 
}

void watchtask() {
  // fallDetect();
  if(alarm_ready) {
    alarm_ready = false;
    Screen = 8;
    subScreen = 0;
    faceChange = true;
  }
  if (pressState == 1 && digitalRead(0) == 1) {
    pressState = 0;
  }
  if (millis() - lastWake > duration_brightness) {
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
      compassInitScreen();
    } else if (subScreen == 4) {
      walkInitScreen();
    }
  }
  if (Screen == 1) {
    if (subScreen == 0) {
      bluetoothInitScreen();
    } else if (subScreen == 1) {
      timeSyncInitScreen();
    }
    else if (subScreen == 2) {
      alarmInitScreen();
    }
    else if (subScreen == 3) {
      watchSettingInitScreen();
    }
    else if (subScreen == 4) {
      if (faceChange == true) {
        tft.fillScreen(TFT_BLACK);
        rc_exit = png.openFLASH((uint8_t *)exit_png, sizeof(exit_png), pngDraw);
        if (rc_exit == PNG_SUCCESS) {
            tft.startWrite();
            rc_exit = png.decode(NULL, 0);
            tft.endWrite();
        }
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        String stopwatch = "EXIT";
        int textWidth_stopwatch = tft.textWidth(stopwatch);
        int x_stopwatch = (tft.width() - textWidth_stopwatch) / 2;
        tft.setCursor(x_stopwatch, 200);
        tft.printf("%s", stopwatch);
        faceChange = false;
      }
    }
  }
  if (Screen == 2) {
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
      subScreen = 0;
      faceChange = true;
    }
  }
  if (Screen == 3) {
    if (subScreen == 0) {
      timeSyncApp();
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
  if (Screen == 6) {
    compassApp();
  }
  if (Screen == 7) {
    alarmApp();
  }
  if (Screen == 8) {
    if (faceChange == true) {
      PCF8563_Init();
      PCF8563_Set_Alarm(hour_alarm, minute_alarm);
      PCF8563_Alarm_Enable();
      tft.fillScreen(TFT_BLACK);
      faceChange = false;
    }
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 40);
    tft.printf("Hello World!");
    tft.setCursor(45, 80);
    tft.printf("WAKE UP!");
    lastWake = millis();

    // Phát lần lượt các nốt trong giai điệu
    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
      int noteDuration = 1000 / noteDurations[i]; // Tính độ dài của từng nốt
      tone(buzzerPin, melody[i], noteDuration);   // Phát nốt nhạc

      // Kiểm tra button trong khi chờ kết thúc nốt nhạc
      unsigned long startTime = millis();
      while (millis() - startTime < noteDuration) {
        button.tick(); // Cập nhật trạng thái button
        if (Screen == 0) { // Nếu button được nhấn
          noTone(buzzerPin);      // Tắt âm thanh
        }
      }
      noTone(buzzerPin); // Tắt buzzer
    }
  }
  if (Screen == 9) {
    if (subScreen == 0) {
      walkApp();
    } else if (subScreen == 1) {
      walkResult();
    }
  }
  if (Screen == 10) {
    watchSettingApp();
  }
}

void loop() {
  button.tick();
  watchtask();
}
