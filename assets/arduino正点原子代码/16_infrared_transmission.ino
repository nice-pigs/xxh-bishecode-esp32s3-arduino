/**
 ******************************************************************************
 * @file     16_infrared_transmission.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    红外发送 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习红外发送
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
 * 6,   R_OUT --> ESP32S3 IO
 *        OUT --> IO8(跳线帽连接)
 *
 * 实验现象：
 * 1, 按照程序设定发送红外遥控数据，红外接收头接收到数据，在LCD会显示键值以及标识
 * 
 * 注意事项：
 * 1, 需要使用跳线帽连接AIN和RMT
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
#include "emission.h"


uint8_t key_value = 0;
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
    remote_init();          /* 红外接收初始化 */
    emission_init();        /* 红外发送初始化 */

    lcd_show_string(30,  50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30,  70, 200, 16, LCD_FONT_16, "EMISSION TEST", RED);
    lcd_show_string(30,  90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "TX KEYVAL:", RED);
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "RX KEYVAL:", RED);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    t++;
    if (t == 0)
    {
        t = 1;
    }

    emission_send(t);                                 /* 红外发送键值 */
    lcd_show_num(110, 110, t, 3, LCD_FONT_16, BLUE);  /* 显示红外发送键值 */

    key_value = remote_scan();                        /* 红外接收键值 */
    if (key_value)
    {
        lcd_show_num(110, 130, key_value, 3, LCD_FONT_16, BLUE);  /* 显示红外接收键值 */
    }

    LED_TOGGLE();
    delay(200);
}
