#include "PCF8563.h"

/******************************************************************************
i2C underlying read and write
******************************************************************************/
void PCF8563_IIC_Write(UBYTE reg, UBYTE data)
{
    Wire.beginTransmission(I2C_Addr);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

UBYTE PCF8563_IIC_Read(UBYTE reg)
{
    Wire.beginTransmission(I2C_Addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(I2C_Addr, 1);
    return Wire.read();
}

/******************************************************************************
function: Init
parameter:
Info:
******************************************************************************/
void PCF8563_Init(void)
{
    pinMode(INT_PIN, INPUT_PULLUP);     //INPT
    Wire.begin(41, 40);
    Serial.begin(115200);
    PCF8563_IIC_Write(0x00, 0x00); //basic setting
    PCF8563_IIC_Write(0x01, 0x00); //Disable INT
    PCF8563_Timer_Disable();
    PCF8563_Alarm_Disable();
}

/******************************************************************************
function: Set  Time
parameter:
    hour:0~23
    minute:0~60
    second:0~60
Info:
******************************************************************************/
void PCF8563_Set_Time(int hour, int minute, int second)
{
    if (hour >= 0)
    {
        hour = changeIntToHex(hour % 60);
        PCF8563_IIC_Write(HOUR_DATA_BUF, hour);
    }
    if (minute >= 0)
    {
        minute = changeIntToHex(minute % 60);
        PCF8563_IIC_Write(MINUTE_DATA_BUF, minute);
    }
    if (second >= 0)
    {
        second = changeIntToHex(second % 60);
        PCF8563_IIC_Write(SECOND_DATA_BUF, second);
    }
}

/******************************************************************************
function: Set  date
parameter:
    year:1900~2099
    months:1~12
    days:0~31
Info:
******************************************************************************/
void PCF8563_Set_Days(int year, int months, int days)
{

    if (days >= 0 && days <= 31)
    {
        days = changeIntToHex(days);
        PCF8563_IIC_Write(DAY_DATA_BUF, days);
    }
    if (months >= 0 && months <= 12)
    {
        months = changeIntToHex(months);
        PCF8563_IIC_Write(MONTH_DATA_BUF, (PCF8563_IIC_Read(MONTH_DATA_BUF) & 0x80) | months);
    }

    if (year >= 1900 && year < 2000)
    {

        PCF8563_IIC_Write(MONTH_DATA_BUF, PCF8563_IIC_Read(MONTH_DATA_BUF) | 0x80);
        /*7  century; this bit is toggled when the years register
            overflows from 99 to 00
            0 indicates the century is 20xx
            1 indicates the century is 19xx
        */
        PCF8563_IIC_Write(YEAR_DATA_BUF, year % 100);
    }
    else if (year >= 2000 && year < 3000)
    {
        PCF8563_IIC_Write(MONTH_DATA_BUF, PCF8563_IIC_Read(MONTH_DATA_BUF) & 0x7F);
        PCF8563_IIC_Write(YEAR_DATA_BUF, year % 100);
    }
}

/******************************************************************************
function: Get date
parameter:
    buf: Data buffer
Info:
******************************************************************************/
void PCF8563_Get_Days(UBYTE *buf)
{
    buf[0] = PCF8563_IIC_Read(DAY_DATA_BUF) & 0x3f;
    buf[1] = PCF8563_IIC_Read(MONTH_DATA_BUF) & 0x1f;
    buf[2] = PCF8563_IIC_Read(YEAR_DATA_BUF) & 0xff;

    buf[0] = changeHexToInt(buf[0]);
    buf[1] = changeHexToInt(buf[1]);
    //buf[2] = changeHexToInt(buf[2]);

    if (PCF8563_IIC_Read(MONTH_DATA_BUF) & 0x80)
    {
        buf[3] = 19;
    }
    else
    {
        buf[3] = 20;
    }
}

/******************************************************************************
function: Set Week Data
parameter:
    WeekData:0~7
Info:
******************************************************************************/
void PCF8563_Set_WeekData(int WeekData)
{
    if (WeekData <= 7)
    {
        PCF8563_IIC_Write(WEEK_DATA_BUF, WeekData);
    }
}

/******************************************************************************
function: Get Time
parameter:
    buf: Data buffer
Info:
******************************************************************************/
void PCF8563_Get_Time(UBYTE *buf)
{
    buf[0] = PCF8563_IIC_Read(SECOND_DATA_BUF) & 0x7f; //get second data
    buf[1] = PCF8563_IIC_Read(MINUTE_DATA_BUF) & 0x7f; //get minute data
    buf[2] = PCF8563_IIC_Read(HOUR_DATA_BUF) & 0x3f;   //get hour data

    buf[0] = changeHexToInt(buf[0]);
    buf[1] = changeHexToInt(buf[1]);
    buf[2] = changeHexToInt(buf[2]);
}

/******************************************************************************
function: Set alarm parameters
parameter:
    hour: 0~23
    minute: 0~59
Info:
******************************************************************************/
void PCF8563_Set_Alarm(int hour, int minute)
{
    if (minute >= 0)
    {
        minute = changeIntToHex(minute);
        PCF8563_IIC_Write(MINUTE_AE_BUF, minute);
    }

    if (hour >= 0)
    {
        hour = changeIntToHex(hour);
        PCF8563_IIC_Write(HOUR_AE_BUF, hour);
    }
}
/******************************************************************************
function: Enable Alarm
parameter:
Info:
******************************************************************************/
void PCF8563_Alarm_Enable(void)
{

    PCF8563_IIC_Write(0x01, (PCF8563_IIC_Read(0x01) | 0x02));
    PCF8563_IIC_Write(MINUTE_AE_BUF, PCF8563_IIC_Read(MINUTE_AE_BUF) & 0x7f);
    PCF8563_IIC_Write(HOUR_AE_BUF, PCF8563_IIC_Read(HOUR_AE_BUF) & 0x7f);
    PCF8563_IIC_Write(DAY_AE_BUF, 0x80);  //关闭
    PCF8563_IIC_Write(WEEK_AE_BUF, 0x80); //关闭
}

/******************************************************************************
function: Disable Alarm
parameter:
Info:
******************************************************************************/
void PCF8563_Alarm_Disable(void)
{
    PCF8563_IIC_Write(0x01, (PCF8563_IIC_Read(0x01) & 0xfd));
    PCF8563_IIC_Write(MINUTE_AE_BUF, PCF8563_IIC_Read(MINUTE_AE_BUF) | 0x80);
    PCF8563_IIC_Write(HOUR_AE_BUF, PCF8563_IIC_Read(HOUR_AE_BUF) | 0x80);
    PCF8563_IIC_Write(DAY_AE_BUF, 0x80);  //关闭
    PCF8563_IIC_Write(WEEK_AE_BUF, 0x80); //关闭
}

/******************************************************************************
function: Set timer register
parameter:
    Timer_Frequency : Choose the corresponding frequency
                    4096    :TIMER_FREQUENCY_4096
                    64      :TIMER_FREQUENCY_64
                    1       :TIMER_FREQUENCY_1
                    0       :TIMER_FREQUENCY_1_60
    Value           : Value
                    Total cycle = Value/TIMER_FREQUENCY
Info:
    TIMER_CTRL_BUF//0x0E
    TIMER_FREQUENCY_4096    0 // 4096HZ      MAX  0.062 second
    TIMER_FREQUENCY_64      1 // 64HZ        MAX  3.98 second
    TIMER_FREQUENCY_1       2 // 1HZ         MAX  255 second
    TIMER_FREQUENCY_1_60    3 // 1/60Hz      MAX  255 minute
******************************************************************************/
void PCF8563_Set_Timer(int Timer_Frequency, UBYTE Value)
{
    // PCF8563_IIC_Write(TIMER_CTRL_BUF, PCF8563_IIC_Read(TIMER_CTRL_BUF)&0x7f);
    PCF8563_IIC_Write(COUNT_VAL_BUF, Value);
    if (Timer_Frequency == 4096)
    {
        PCF8563_IIC_Write(TIMER_CTRL_BUF, ((PCF8563_IIC_Read(TIMER_CTRL_BUF)) & 0xfc) | TIMER_FREQUENCY_4096);
    }
    else if (Timer_Frequency == 64)
    {
        PCF8563_IIC_Write(TIMER_CTRL_BUF, ((PCF8563_IIC_Read(TIMER_CTRL_BUF)) & 0xfc) | TIMER_FREQUENCY_64);
    }
    else if (Timer_Frequency == 1)
    {
        PCF8563_IIC_Write(TIMER_CTRL_BUF, ((PCF8563_IIC_Read(TIMER_CTRL_BUF)) & 0xfc) | TIMER_FREQUENCY_1);
    }
    else if (Timer_Frequency == 0)
    { // 1/60
        PCF8563_IIC_Write(TIMER_CTRL_BUF, ((PCF8563_IIC_Read(TIMER_CTRL_BUF)) & 0xfc) | TIMER_FREQUENCY_1_60);
    }
    else
    {
        printf("Set Timer Error\r\n");
    }
}
/******************************************************************************
function: Enable timer
parameter:
Info:
******************************************************************************/
void PCF8563_Timer_Enable(void)
{
    PCF8563_IIC_Write(0x01, (PCF8563_IIC_Read(0x01) | 0x01));
    PCF8563_IIC_Write(TIMER_CTRL_BUF, PCF8563_IIC_Read(TIMER_CTRL_BUF) | 0x80);
}

/******************************************************************************
function: Disable timer
parameter:
Info:
******************************************************************************/
void PCF8563_Timer_Disable(void)
{
    PCF8563_IIC_Write(0x01, (PCF8563_IIC_Read(0x01) & 0xfe));
    PCF8563_IIC_Write(TIMER_CTRL_BUF, PCF8563_IIC_Read(TIMER_CTRL_BUF) & 0x7f);
}

/******************************************************************************
function: Clear alarm interrupt flag
parameter:
Info:
******************************************************************************/
void PCF8563_Cleare_AF_Flag(void)
{
    PCF8563_IIC_Write(CTRL_BUF2, PCF8563_IIC_Read(CTRL_BUF2) & 0xf7);
}

/******************************************************************************
function: Clear timer interrupt flag
parameter:
Info:
******************************************************************************/
void PCF8563_Cleare_TF_Flag(void)
{
    PCF8563_IIC_Write(CTRL_BUF2, PCF8563_IIC_Read(CTRL_BUF2) & 0xfB);
}

/******************************************************************************
function: Get Flag
parameter:
Info:
    return: 1:AF alarm
            2:TF timer
            3:AF and TF
******************************************************************************/
UBYTE PCF8563_Get_Flag(void)
{
    UBYTE temp = 0;
    if (PCF8563_IIC_Read(CTRL_BUF2) & 0x08)
    {
        temp = temp | 0x01;
    }
    if (PCF8563_IIC_Read(CTRL_BUF2) & 0x04)
    {
        temp = temp | 0x02;
    }
    return temp;
}
/******************************************************************************
function: Set timer register
parameter:
    Timer_Frequency : Choose the corresponding frequency
                    32768   :327.68KHz
                    1024    :1024Hz
                    32      :32Hz
                    1       :1Hz
    Value           : Value
                    Total cycle = Value/TIMER_FREQUENCY
Info:

******************************************************************************/
void PCF8563_CLKOUT_FrequencyHZ(UWORD Frequency)
{
    if (Frequency == 32768)
    {
        PCF8563_IIC_Write(CLK_FRQ_BUF, (PCF8563_IIC_Read(CLK_FRQ_BUF) & 0xfC) | 0x00);
    }
    else if (Frequency == 1024)
    {
        PCF8563_IIC_Write(CLK_FRQ_BUF, (PCF8563_IIC_Read(CLK_FRQ_BUF) & 0xfC) | 0x01);
    }
    else if (Frequency == 32)
    {
        PCF8563_IIC_Write(CLK_FRQ_BUF, (PCF8563_IIC_Read(CLK_FRQ_BUF) & 0xfC) | 0x02);
    }
    else if (Frequency == 1)
    {
        PCF8563_IIC_Write(CLK_FRQ_BUF, (PCF8563_IIC_Read(CLK_FRQ_BUF) & 0xfC) | 0x03);
    }
    else
    {
        printf("Set CLKOUT requency Selection Error\r\n");
    }
}
/******************************************************************************
function: Enable CLKOUT
parameter:
Info:
******************************************************************************/
void PCF8563_CLKOUT_Enable(void)
{
    PCF8563_IIC_Write(CLK_FRQ_BUF, PCF8563_IIC_Read(CLK_FRQ_BUF) | 0x80);
}

/******************************************************************************
function: Disable CLKOUT
parameter:
Info:
******************************************************************************/
void PCF8563_CLKOUT_Disable(void)
{
    PCF8563_IIC_Write(CLK_FRQ_BUF, PCF8563_IIC_Read(CLK_FRQ_BUF) & 0x7f);
}
