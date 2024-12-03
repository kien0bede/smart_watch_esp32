#ifndef __WALK_MPU_APP_H__
#define __WALK_MPU_APP_H__

#include <MPU6050.h>
#include <SimpleKalmanFilter.h>

extern MPU6050 mpu;

// Count times
void walkApp();
// Init Screen
void walkInitScreen();
// Result Screen
void walkResult();

#endif