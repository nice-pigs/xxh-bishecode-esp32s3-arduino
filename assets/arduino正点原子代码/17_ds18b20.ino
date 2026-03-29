/**
 ******************************************************************************
 * @file     17_ds18b20.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    温度传感器 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习温度传感器的使用
 *
 * 硬件资源及引脚分配：
 * 1,     LED --> ESP32S3 IO
 *        LED --> IO1
 * 2,   UART0 --> ESP32S3 IO
 *       TXD0 --> IO43
 *       RXD0 --> IO44
 * 3,  XL9555 --> ESP32S3 IO
 *        SCL --> IO42
 *        SDA --> IO41
 * 4, SPI_LCD --> ESP32S3 IO / XL9555
 *         CS --> IO21
 *        SCK --> IO12
 *        SDA --> IO11
 *         DC --> IO40(跳线帽连接)
 *        PWR --> XL9555_P13
 *        RST --> XL9555_P12
 * 5, DS18B20 --> ESP32S3 IO
 *         DQ --> IO0(跳线帽连接)
 *
 * 实验现象：
 * 1, LCD会显示DS18B20温度传感器采集的温度数据
 * 
 * 注意事项：
 * 1, 需要使用跳线帽连接1WIRE_DQ和IO0
 * 2, IO0和BOOT按键是共用一个IO口的，需要分时复用
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

#include "led.h"
#include "uart.h"
#include "xl9555.h"
#include "spilcd.h"
#include "ds18b20.h"


short temperature;  /* 温度值 */
uint8_t t = 0;

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    led_init();             /* LED初始化 */
    uart_init(0, 115200);   /* 串口0初始化 */
    xl9555_init();          /* IO扩展芯片初始化 */
    lcd_init();             /* LCD初始化 */
    ds18b20_init();         /* DS18B20初始化 */

    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "DS18B20 TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Temp:   . C", BLUE);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    if (t % 10 == 0)
    {
        temperature = ds18b20_get_temperature();

        if (temperature < 0)
        {
            lcd_show_char(30 + 40, 110, '-', LCD_FONT_16, 0, BLUE); 
            temperature = -temperature;                    
        }
        else
        {
            lcd_show_char(30 + 40, 110, ' ', LCD_FONT_16, 0, BLUE); 
        }

        lcd_show_num(30 + 40 + 8, 110, temperature / 10, 2, LCD_FONT_16, BLUE); 
        lcd_show_num(30 + 40 + 32, 110, temperature % 10, 1, LCD_FONT_16, BLUE); 
    }

    delay(10);

    t++;
    if (t == 20)
    {
        t = 0;
        LED_TOGGLE();
    }
}
