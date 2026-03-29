/**
 ******************************************************************************
 * @file     20_camera.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    摄像头 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习摄像头的使用
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
 * 5,  CAMERA --> ESP32S3 IO / XL9555
 *     OV_SCL --> 38
 *     OV_SDA --> 39
 *      VSYNC --> 47
 *       HREF --> 48
 *       PCLK --> 45
 *         D0 --> 4
 *         D1 --> 5
 *         D2 --> 6
 *         D3 --> 7
 *         D4 --> 15
 *         D5 --> 16
 *         D6 --> 17
 *         D7 --> 18
 *      RESET --> XL9535_P05
 *       PWDN --> XL9535_P04
 *
 * 实验现象：
 * 1, LCD会显示摄像头拍摄的内容
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

#include "uart.h"
#include "led.h"
#include "led.h"
#include "uart.h"
#include "xl9555.h"
#include "spilcd.h"
#include "camera.h"


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

    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "CAMERA TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);

    // 摄像头初始化（可能需要重试一次）
    int retry = 0;
    while (camera_init())
    {
        retry++;
        if (retry == 1) {
            Serial.println("第一次初始化失败（I2C冲突），重试中...");
        }
        lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Init camera...", RED);
        delay(500);
        lcd_fill(30, 110, 230, 126, WHITE);
        delay(500);

        if (retry > 3) {
            lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Camera FAILED!", RED);
            Serial.println("摄像头初始化失败！请检查硬件。");
            while(1) { delay(1000); }
        }
    }
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    camera_capture_show();  /* 在LCD显示摄像头捕获的数据 */
}
