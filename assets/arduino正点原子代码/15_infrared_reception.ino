/**
 ******************************************************************************
 * @file     15_infrared_reception.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    红外接收 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习红外接收
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
 *        INT --> IO0(跳线帽连接) 
 * 4, SPI_LCD --> ESP32S3 IO / XL9555
 *         CS --> IO21
 *        SCK --> IO12
 *        SDA --> IO11
 *         DC --> IO40(跳线帽连接)
 *        PWR --> XL9555_P13
 *        RST --> XL9555_P12
 * 5,    R_IN --> ESP32S3 IO
 *         IN --> IO2
 *
 * 实验现象：
 * 1, 按下红外遥控器，LCD会显示键值以及标识
 * 
 * 注意事项：
 * 无
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
#include "remote.h"


uint8_t rmt_key;
char *str = "0";

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
    remote_init();          /* 红外接收初始化 */
    
    lcd_show_string(30,  50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30,  70, 200, 16, LCD_FONT_16, "REMOTE TEST", RED);
    lcd_show_string(30,  90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "KEYVAL:", RED);
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "SYMBOL:", RED);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    rmt_key  = remote_scan();

    if (rmt_key)
    {
        lcd_show_num(86, 110, rmt_key, 3, LCD_FONT_16, BLUE);   /* 显示键值 */

        switch (rmt_key)   /* 判断键值 */
        {
            case 0:
                str = "ERROR";
                break;

            case 162:
                str = "POWER";
                break;

            case 98:
                str = "UP";
                break;

            case 2:
                str = "PLAY";
                break;

            case 226:
                str = "ALIENTEK";
                break;

            case 194:
                str = "RIGHT";
                break;

            case 34:
                str = "LEFT";
                break;

            case 224:
                str = "VOL-";
                break;

            case 168:
                str = "DOWN";
                break;

            case 144:
                str = "VOL+";
                break;

            case 104:
                str = "1";
                break;

            case 152:
                str = "2";
                break;

            case 176:
                str = "3";
                break;

            case 48:
                str = "4";
                break;

            case 24:
                str = "5";
                break;

            case 122:
                str = "6";
                break;

            case 16:
                str = "7";
                break;

            case 56:
                str = "8";
                break;

            case 90:
                str = "9";
                break;

            case 66:
                str = "0";
                break;

            case 82:
                str = "DELETE";
                break;
        }

        lcd_fill(86, 130, 116 + 8 * 8, 170 + 16, WHITE);              /* 清楚之前的显示 */
        lcd_show_string(86, 130, 200, 16, LCD_FONT_16, str, BLUE);    /* 显示SYMBOL */
    }

    LED_TOGGLE();
    delay(500);
}
