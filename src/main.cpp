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

#define SCL_PIN 40
#define SDA_PIN 41
#define BUTTON_PIN 21
#define TFT_BLK_PIN 14

RTC_DATA_ATTR int boot_count = 0;

// Thông tin kết nối WiFi
const char* ssid = "SIX TRET";
const char* password = "chaokhub";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600);
RTC_DS1307 rtc;

TFT_eSPI tft = TFT_eSPI();

Adafruit_MPU6050 mpu;
QMC5883LCompass compass;
MAX30105 particleSensor;

OneButton button(BUTTON_PIN, true);

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

int current_sensor = 1;
bool button_pressed = false;
bool heart_sensor_on = true;

unsigned long lastPressed = 0;
unsigned long lastWake = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long millisHeartRate = 0;
unsigned long pressStartTime;
int pressState = 0;
int Screen = 0;
int subScreen = 0;
bool faceChange = false;
bool wifi_disconnect = true;
bool screenHeartRate = true;

#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)  // 2 ^ GPIO_NUMBER in hex
#define WAKEUP_GPIO_4              GPIO_NUM_4     // Only RTC IO are allowed - ESP32 Pin example
#define WAKEUP_GPIO_21              GPIO_NUM_21     // Only RTC IO are allowed - ESP32 Pin example

// Define bitmask for multiple GPIOs
uint64_t bitmask = BUTTON_PIN_BITMASK(WAKEUP_GPIO_4) | BUTTON_PIN_BITMASK(WAKEUP_GPIO_21);

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
  Wire.requestFrom(address, 2);

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
  rtc_gpio_hold_en(GPIO_NUM_21);
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
    if (subScreen == 1 && !screenHeartRate) {
      millisHeartRate = millis();
      digitalWrite(TFT_BLK_PIN, HIGH);
      screenHeartRate = true;
      return;
    }
    subScreen++;
    
    if (subScreen > 3) {
      subScreen = 0;
      faceChange = true;
    }

    if (subScreen == 1) {
      particleSensor.wakeUp();
      millisHeartRate = millis();
    } else {
      particleSensor.shutDown();
    }
  }
  pressState = 1;
}

void LongPress() {
  printf("pressStart()\n");
  pressStartTime = millis() - 1000;  // as set in setPressTicks()
  lastWake = millis();
  lastDisplayUpdate = millis();
  particleSensor.shutDown();
  pressState = 1;
  lastPressed = millis();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  // Init gia toc
  // mpu.begin(0x69);
  // mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  // mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  configureMPU(5);
  // Init la ban
  compass.init();
  // Init nhip tim
  particleSensor.begin();
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  // Init button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  button.attachClick(ShortClick);
  button.attachLongPressStart(LongPress);
  // Khởi tạo TFT
  tft.init();
  tft.setRotation(1);  // Điều chỉnh hướng màn hình
  tft.fillScreen(TFT_BLACK);  // Đặt màu nền là đen
  tft.setTextColor(TFT_WHITE);  // Đặt màu chữ là trắng
  tft.setTextSize(2);  // Kích thước chữ

  rtc_gpio_hold_dis((gpio_num_t) TFT_BLK_PIN);
  pinMode(TFT_BLK_PIN, OUTPUT);
  digitalWrite(TFT_BLK_PIN, HIGH);

  if(!rtc.begin()) {
    printf("Không thể kết nối với DS1307!\n");
  }

  boot_count++;
  printf("Số lần khởi động: %d\n", boot_count);

  if (boot_count == 1) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      printf("Đang kết nối WiFi...\n");
    }
    printf("WiFi đã kết nối!\n");
    wifi_disconnect = false;
    timeClient.begin();
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    DateTime ntpTime = DateTime(epochTime);
    rtc.adjust(ntpTime);
    WiFi.disconnect(true);
    wifi_disconnect = true;
  }
  else {
    DateTime rtcTime = rtc.now();
    printf("Thời gian từ DS1307 sau khi thức dậy: ");
    printf(rtcTime.timestamp().c_str());
  }
  delay(100);
}

void watchFace() {
  DateTime now = rtc.now();
  
  // Đặt cỡ chữ lớn
  tft.setTextSize(3); // Cỡ chữ lớn (thay đổi giá trị nếu cần)
  
  // Đặt màu chữ là trắng và nền là đen
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Màu chữ trắng và nền đen
  
  // In thời gian theo định dạng chiều dọc
  tft.setCursor(10, 10); // Đặt vị trí con trỏ để vẽ
  tft.printf("%02d/%02d/%04d\n", 
    now.day(),    // Ngày
    now.month(),  // Tháng
    now.year()    // Năm
  );

  tft.setCursor(10, 80); // Di chuyển con trỏ xuống để in giờ
  tft.printf("%02d:%02d:%02d", 
    now.hour(),    // Giờ
    now.minute(),  // Phút
    now.second()   // Giây
  );
}

void heartRateApp() {
  long irValue = particleSensor.getIR();
  if (millis() - millisHeartRate > 10000)
  {
    digitalWrite(TFT_BLK_PIN, LOW);
    screenHeartRate = false;
  }

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  printf("IR=");
  printf("%d", irValue);
  printf(", BPM=");
  printf("%d", beatsPerMinute);
  printf(", Avg BPM=");
  printf("%d\n", beatAvg);

  if (irValue < 50000)
    printf(" No finger?\n");

  printf("");
}

void compassApp() {
  compass.read();
  int x = compass.getX();
  int y = compass.getY();
  int z = compass.getZ();
  printf("Compass X: %d, Y: %d, Z: %d\n", x, y, z);
}

void watchtask() {
  fallDetect();
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
      heartRateApp();
    } else if (subScreen == 2) {
      compassApp();
    } else if (subScreen == 3) {
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
  }
}

void loop() {
  button.tick();
  watchtask();
}
