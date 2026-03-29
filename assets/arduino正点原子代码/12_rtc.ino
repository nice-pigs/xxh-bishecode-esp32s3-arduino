/**
 ******************************************************************************
 * @file     12_rtc.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    RTC 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习RTC外设的使用
 *
 * 硬件资源及引脚分配：
 * 1,   UART0 --> ESP32S3 IO
 *       TXD0 --> IO43
 *       RXD0 --> IO44
 * 2,  XL9555 --> ESP32S3 IO
 *        SCL --> IO42
 *        SDA --> IO41
 *        INT --> IO0(跳线帽连接) 
 * 3, SPI_LCD --> ESP32S3 IO / XL9555
 *         CS --> IO21
 *        SCK --> IO12
 *        SDA --> IO11
 *         DC --> IO40(跳线帽连接)
 *        PWR --> XL9555_P13
 *        RST --> XL9555_P12
 *
 * 实验现象：
 * 1, LCD显示RTC实时时钟年月日时分秒星期信息
 * 
 * 注意事项：
 * 1, 需要用到ESP32Time库,具体操作：在软件中选择"项目"-->"加载库"-->"管理库"-->输入"ESP32Time"安装即可
 * 
 ******************************************************************************
 * 
 * 实验平台:正点原子 ESP32S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com/forum.php
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ******************************************************************************
 */

#include "uart.h"
#include "xl9555.h"
#include "spilcd.h"
#include <ESP32Time.h>    /* 需要安装ESP32Time库 */


ESP32Time rtc;
uint8_t tbuf[100];        /* 存放RTC信息 */

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    uart_init(0, 115200);   /* 串口0初始化 */
    xl9555_init();          /* IO扩展芯片初始化 */
    lcd_init();             /* LCD初始化 */
    rtc.setTime(00, 51, 17, 1, 12, 2023);  /* 2023年12月1日17:52:00 */
    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "RTC TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    struct tm timeinfo = rtc.getTimeStruct();
    /* 根据time.h头文件中tm结构体的定义进行调整显示 */
    sprintf((char *)tbuf, "Time:%02d:%02d:%02d", timeinfo.tm_hour - 1, timeinfo.tm_min, timeinfo.tm_sec);     
    lcd_show_string(30, 130, 210, 16, LCD_FONT_16, (char *)tbuf, RED);
    sprintf((char *)tbuf, "Date:%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    lcd_show_string(30, 150, 210, 16, LCD_FONT_16, (char *)tbuf, RED);
    sprintf((char *)tbuf, "Week:%d", timeinfo.tm_wday);
    lcd_show_string(30, 170, 210, 16, LCD_FONT_16, (char *)tbuf, RED);

    delay(1000);

    /* ESP32Time其他函数接口,可以自行使用Serial.println函数打印
    getTime()           //  (String) 15:24:38
    getDate()           //  (String) Sun, Jan 17 2021
    getDate(true)       //  (String) Sunday, January 17 2021
    getDateTime()       //  (String) Sun, Jan 17 2021 15:24:38
    getDateTime(true)   //  (String) Sunday, January 17 2021 15:24:38
    getTimeDate()       //  (String) 15:24:38 Sun, Jan 17 2021
    getTimeDate(true)   //  (String) 15:24:38 Sunday, January 17 2021

    getMicros()         //  (unsigned long) 723546
    getMillis()         //  (unsigned long) 723
    getEpoch()          //  (unsigned long) 1609459200
    getLocalEpoch()     //  (unsigned long) 1609459200 // local epoch without offset
    getSecond()         //  (int)     38    (0-59)
    getMinute()         //  (int)     24    (0-59)
    getHour()           //  (int)     3     (0-12)
    getHour(true)       //  (int)     15    (0-23)
    getAmPm()           //  (String)  pm
    getAmPm(false)      //  (String)  PM
    getDay()            //  (int)     17    (1-31)
    getDayofWeek()      //  (int)     0     (0-6)
    getDayofYear()      //  (int)     16    (0-365)
    getMonth()          //  (int)     0     (0-11)
    getYear()           //  (int)     2021
    */
}
