#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <MAX30105.h>

#define SCL_PIN 40
#define SDA_PIN 41
#define BUTTON_PIN 21

Adafruit_MPU6050 mpu;
QMC5883LCompass compass;
MAX30105 particleSensor;

int current_sensor = 1;
bool button_pressed = false;
bool heart_sensor_on = true;

void IRAM_ATTR buttonISR() {
  button_pressed = true;
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
  // Init button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void readMPU() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  printf("MPU X: %.3f, Y: %.3f, Z: %.3f\n", a.acceleration.x, a.acceleration.y, a.acceleration.z);
  delay(500);
}

void readCompass() {
  compass.read();
  int x = compass.getX();
  int y = compass.getY();
  int z = compass.getZ();
  printf("Compass X: %d, Y: %d, Z: %d\n", x, y, z);
  delay(500);
}

void readHeartSensor() {
  u_int32_t red = particleSensor.getRed();
  u_int32_t ir = particleSensor.getIR();
  u_int32_t green = particleSensor.getGreen();
  printf("Red: %d, IR: %d, Green: %d\n", red, ir, green);
  delay(500);
}

void shutdownHeartSensor() {
  particleSensor.shutDown();
}

void wakeupHeartSensor() {
  particleSensor.wakeUp();
}

void loop() {
  if (button_pressed) {
    button_pressed = false;

    switch (current_sensor) {
      case 1:
        current_sensor = 2;
        break;
      case 2:
        current_sensor = 3;
        break;
      case 3:
        current_sensor = 1;
        break;
    }

    if (current_sensor != 1 && heart_sensor_on) {
      shutdownHeartSensor();
      heart_sensor_on = false;
    }
  }
  if (current_sensor == 1) {
    if (!heart_sensor_on) {
      wakeupHeartSensor();
      heart_sensor_on = true;
    }
    readHeartSensor();
  } else if (current_sensor == 2) {
    readCompass();
  } else {
    readMPU();
  }

  delay(100);
}
