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

UBYTE buf[10];

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

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);

PNG png; 

// Adafruit_MPU6050 mpu;
// QMC5883LCompass compass;

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
// #define WAKEUP_GPIO_INT_RTC        GPIO_NUM_21    // INT từ PCF8563

// Define bitmask for multiple GPIOs
uint64_t bitmask = BUTTON_PIN_BITMASK(WAKEUP_GPIO_2) | BUTTON_PIN_BITMASK(WAKEUP_GPIO_12) /*| BUTTON_PIN_BITMASK(WAKEUP_GPIO_INT_RTC)*/;

// MPU registers
#define SIGNAL_PATH_RESET  0x68
#define ACCEL_CONFIG       0x1C
#define MOT_THR            0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR            0x20  // Duration counter threshold for motion interrupt
#define MOT_DETECT_CTRL    0x69
#define INT_ENABLE         0x38
#define PWR_MGMT           0x6B //SLEEPY TIME
#define MPU6050_ADDRESS    0x69 //AD0 is 0

#define ACCEL_XOUT_H 0x3B     // Register for Accelerometer X-axis high byte
#define GYRO_XOUT_H 0x43      // Register for Gyroscope X-axis high byte

 int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
 float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
 boolean fall = false; //stores if a fall has occurred
 boolean trigger1 = false; //stores if first trigger (lower threshold) has occurred
 boolean trigger2 = false; //stores if second trigger (upper threshold) has occurred
 boolean trigger3 = false; //stores if third trigger (orientation change) has occurred
 byte trigger1count = 0; //stores the counts past since trigger 1 was set true
 byte trigger2count = 0; //stores the counts past since trigger 2 was set true
 byte trigger3count = 0; //stores the counts past since trigger 3 was set true
 int angleChange = 0;

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.write(data);
  Wire.endTransmission();
}

uint8_t readByte(uint8_t address, uint8_t subAddress)
{
  uint8_t data;
  Wire.beginTransmission(address);
  Wire.write(subAddress);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t) 1);
  data = Wire.read();
  return data;
}

int16_t readWord(uint8_t address, uint8_t reg) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t) 2);

  int16_t value = Wire.read() << 8 | Wire.read();
  return value;
}

void mpu_read() {
   Wire.beginTransmission(MPU6050_ADDRESS);
   Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
   Wire.endTransmission(false);
   Wire.requestFrom(MPU6050_ADDRESS, 14, true); // request a total of 14 registers
   AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
   AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
   AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
   Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
   GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
   GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
   GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 }

void fallDetect() {
  mpu_read();
  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ - 1947) / 16384.00;
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;
  // calculating Amplitute vactor for 3 axis
  float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
  int Amp = Raw_Amp * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
  Serial.println(Amp);
  if (Amp <= 2 && trigger2 == false) { //if AM breaks lower threshold (0.4g)     
    trigger1 = true;     
    printf("TRIGGER 1 ACTIVATED\n");
  }   
  if (trigger1 == true) {     
    trigger1count++;     
    if (Amp >= 12) { //if AM breaks upper threshold (3g)
      trigger2 = true;
      printf("TRIGGER 2 ACTIVATED\n");
      trigger1 = false; trigger1count = 0;
    }
  }
  if (trigger2 == true) {
    trigger2count++;
    angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5); Serial.println(angleChange);
    if (angleChange >= 30 && angleChange <= 400) { //if orientation changes by between 80-100 degrees       
      trigger3 = true; trigger2 = false; trigger2count = 0;       
      printf("%d", angleChange);       
      printf("TRIGGER 3 ACTIVATED\n");     
    }   
  }   
  if (trigger3 == true) {     
    trigger3count++;     
    if (trigger3count >= 10) {
      angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
      //delay(10);
      printf("%d", angleChange); 
      if ((angleChange >= 0) && (angleChange <= 10)) { //if orientation changes remains between 0-10 degrees         
        fall = true; trigger3 = false; trigger3count = 0;         
        printf("%d", angleChange);      }       
      else { //user regained normal orientation         
        trigger3 = false; trigger3count = 0;         
        printf("TRIGGER 3 DEACTIVATED\n");       
      }     
    }   
  }   
  if (fall == true) { //in event of a fall detection     
    printf("FALL DETECTED\n");        
    fall = false;
  }   
  if (trigger2count >= 6) { //allow 0.5s for orientation change
    trigger2 = false; trigger2count = 0;
    printf("TRIGGER 2 DECACTIVATED\n");
  }
  if (trigger1count >= 6) { //allow 0.5s for AM to break upper threshold
    trigger1 = false; trigger1count = 0;
    printf("TRIGGER 1 DECACTIVATED\n");
  }
}

void configureMPU(int sens){
  writeByte(MPU6050_ADDRESS, 0x6B, 0x00);               // Wake up the MPU6050 by setting the PWR_MGMT_1 register (0x6B) to 0x00.
  writeByte(MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07);  // Reset all internal signal paths by writing 0x07 to the SIGNAL_PATH_RESET register (0x68).
  writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0x01);       // Set the accelerometer Digital High Pass Filter to 5Hz by writing 0x01 to ACCEL_CONFIG (0x1C).
  writeByte(MPU6050_ADDRESS, MOT_THR, sens);            // Set the motion threshold to the desired sensitivity (e.g., 20) in the MOT_THR register (0x1F).
  writeByte(MPU6050_ADDRESS, MOT_DUR, 40);              // Set motion detect duration to 40 ms in the MOT_DUR register (0x20).
  writeByte(MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15);    // Configure motion detection settings by writing 0x15 to MOT_DETECT_CTRL (0x69).
  writeByte(MPU6050_ADDRESS, 0x37, 0x8C);               // Configure INT pin as active low, and latch until interrupt cleared.
  writeByte(MPU6050_ADDRESS, INT_ENABLE, 0x40);         // Enable motion detection interrupt by writing 0x40 to INT_ENABLE (0x38).
  writeByte(MPU6050_ADDRESS, 0x6C, 0x00);         // Enable both accelerometer and gyroscope (PWR_MGMT_2 register 0x6C set to 0x00).
}
void IRAM_ATTR checkTicks() {
  button.tick();
}

void enter_sleep()
{
  analogWrite(TFT_BLK_PIN, 0);
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
      faceChange = true;
    } else if (subScreen == 2) {
      faceChange = true;
    } else if (subScreen == 3) {
      faceChange = true;
    }
  }
  if (Screen == 1) {
    subScreen++;
    faceChange = true;
    if (subScreen > 3) {
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
        alarm_on = true;
      }
      else {
        alarm_on = false;
      }
      Screen = 1;
      subScreen = 2;
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
  configureMPU(5);
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
  digitalWrite(TFT_BLK_PIN, HIGH);

  pinMode(buzzerPin, OUTPUT);

  boot_count++;
  printf("Số lần khởi động: %d\n", boot_count);

  if (boot_count == 1) {
    PCF8563_Init();
    PCF8563_Set_Time(10, 10, 10);
    PCF8563_Set_Days(2025, 10, 10);
    PCF8563_Get_Time(buf);
    PCF8563_Get_Days(&buf[3]);
  }
  else {
    PCF8563_Init();
    PCF8563_Get_Time(buf);
    PCF8563_Get_Days(&buf[3]);
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
  // Đặt điểm vẽ về 0:0
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
  // Đặt điểm vẽ về 0:0
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

void walkApp() {
  int16_t accelX = readWord(MPU6050_ADDRESS, ACCEL_XOUT_H);
  int16_t accelY = readWord(MPU6050_ADDRESS, ACCEL_XOUT_H + 2);
  int16_t accelZ = readWord(MPU6050_ADDRESS, ACCEL_XOUT_H + 4);

  // Convert raw values to 'g' for accelerometer and '°/s' for gyroscope
  float ax = accelX / 16384.0;
  float ay = accelY / 16384.0;
  float az = accelZ / 16384.0;

  // Print the accelerometer and gyroscope values
  printf("Accel (g): X=");
  printf("%f", ax);
  printf(" Y=");
  printf("%f", ay);
  printf(" Z=");
  printf("%f\n", az);
}

void watchtask() {
  // fallDetect();
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
      compassInitScreen();
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
}

void loop() {
  button.tick();
  watchtask();
}
