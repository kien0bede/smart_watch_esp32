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

#define SCL_PIN 40
#define SDA_PIN 41
#define BUTTON_PIN 21

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
unsigned long pressStartTime;
int pressState = 0;
int Screen = 0;
int subScreen = 0;
bool faceChange = false;

void IRAM_ATTR checkTicks() {
  button.tick();
}

void enter_sleep()
{
  rtc_gpio_hold_en(GPIO_NUM_21);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_21, LOW);
  esp_deep_sleep_start();
}

void ShortClick() {
  printf("singleClick() detected.\n");
  lastWake = millis();
  if (Screen == 0) {
    subScreen++;
    if (subScreen > 2) {
      subScreen = 0;
      faceChange = true;
    }
    if (subScreen == 1) {
      particleSensor.wakeUp();
    } else {
      particleSensor.shutDown();
    }
  }
  pressState = 1;
  lastDisplayUpdate = millis();
  lastPressed = millis();
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
  printf("Begin Test!!!\n");
  // Init gia toc
  mpu.begin();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
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
}

void watchFace() {
  printf("Watch Face\n");
}

void heartRateApp() {
  long irValue = particleSensor.getIR();

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
  printf("Compass\n");
}

void watchtask() {
  if (pressState == 1 && digitalRead(0) == 1) {
    pressState = 0;
  }
  if (millis() - lastWake > 30000) {
    enter_sleep();
  }
  if (Screen == 0) {
    if (subScreen == 0) {
      watchFace();
    } else if (subScreen == 1) {
      heartRateApp();
    } else {
      compassApp();
    }
  }
}

void loop() {
  button.tick();
  watchtask();
}
