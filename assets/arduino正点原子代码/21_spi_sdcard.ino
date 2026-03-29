/**
 ******************************************************************************
 * @file     21_spi_sdcard.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    SD 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习SD的使用
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
 * 5,      SD --> ESP32S3 IO
 *       SDCS --> IO2
 *        SCK --> IO12
 *       MOSI --> IO11
 *       MISO --> IO13
 *
 * 实验现象：
 * 1, LCD会显示SD卡的相关信息(容量),通过按键可以进行SD卡测试,通过串口助手进行查看
 * 
 * 注意事项：
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
#include "key.h"
#include "uart.h"
#include "xl9555.h"
#include "spilcd.h"
#include "spi_sdcard.h"
#include <SD.h>


/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    led_init();             /* LED初始化 */
    key_init();             /* KEY初始化 */
    uart_init(0, 115200);   /* 串口0初始化 */
    xl9555_init();          /* IO扩展芯片初始化 */
    lcd_init();             /* LCD初始化 */

    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "SD TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);

    while (sdcard_init())    /* 检测不到SD卡 */    
    {
        lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "SD Card Error!", RED);
        delay(500);
        lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Please Check! ", RED);
        delay(500);
        LED_TOGGLE();       /* 红灯闪烁 */
    }

    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "SD Card OK    ", BLUE);
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "SD Card Size:     MB", BLUE);
    lcd_show_num(30 + 13 * 8, 130, SD.cardSize() / (1024 * 1024), 5, LCD_FONT_16, BLUE); /* 显示SD卡容量, 转换成MB单位 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    if (KEY == 0)
    {
        sd_test();
    }

    LED_TOGGLE();    
    delay(500);
}
