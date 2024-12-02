#ifndef _SERVO_H_
#define _SERVO_H_


#include <Arduino.h>
#include <Wire.h>

#define UBYTE   unsigned char
#define UWORD   unsigned short
#define UDOUBLE unsigned int

#define I2C_Addr    0x51
#define INT_PIN     21

#define CTRL_BUF1   0x00
#define CTRL_BUF2   0x01

#define SECOND_DATA_BUF 0x02
#define MINUTE_DATA_BUF 0x03
#define HOUR_DATA_BUF   0x04

#define DAY_DATA_BUF    0x05
#define WEEK_DATA_BUF   0x06
#define MONTH_DATA_BUF  0x07
#define YEAR_DATA_BUF   0x08

#define MINUTE_AE_BUF   0x09
#define HOUR_AE_BUF     0x0A
#define DAY_AE_BUF      0x0B
#define WEEK_AE_BUF     0x0C

#define CLK_FRQ_BUF     0x0D
#define TIMER_CTRL_BUF  0x0E
#define COUNT_VAL_BUF   0x0F

#define changeIntToHex(dec)     ((((dec)/10) <<4) + ((dec)%10) )
#define converseIntToHex(dec)   ((((dec)>>4) *10) + ((dec)%16) )

#define changeHexToInt(hex)     ((((hex)>>4) *10 ) + ((hex)%16) )
#define converseHexToInt(hex)   ((((hex)/10) <<4 ) + ((hex)%10) )

#define TIMER_FREQUENCY_4096    0 // 4096HZ
#define TIMER_FREQUENCY_64      1 // 64HZ
#define TIMER_FREQUENCY_1       2 // 1HZ
#define TIMER_FREQUENCY_1_60    3 // 1/60Hz



void PCF8563_Init(void);

void PCF8563_Set_Time(int hour, int minute, int second);
void PCF8563_Get_Time(UBYTE *buf);

void PCF8563_Set_Days(int year, int months, int days);
void PCF8563_Get_Days(UBYTE *buf);

void PCF8563_Set_WeekData(int WeekData);

void PCF8563_Set_Alarm(int hour, int minute);
void PCF8563_Set_Timer(int Timer_frequency ,UBYTE value);

void PCF8563_Alarm_Enable(void);
void PCF8563_Alarm_Disable(void);
void PCF8563_Timer_Enable(void);
void PCF8563_Timer_Disable(void);

void PCF8563_Cleare_AF_Flag(void);//Alarm
void PCF8563_Cleare_TF_Flag(void);//Timer

UBYTE  PCF8563_Get_Flag(void);

void PCF8563_CLKOUT_FrequencyHZ(UWORD Frequency);

void PCF8563_CLKOUT_Disable(void);
void PCF8563_CLKOUT_Enable(void);
#endif
