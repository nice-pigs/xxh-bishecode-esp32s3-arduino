 /**
 ******************************************************************************
 * @file     11_spi_lcd.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    SPI_LCD实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习SPI_LCD的使用
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
 * 1, 跳线帽连接LCD_DC和IO_SEL后，下载程序成功后，LCD会显示实验信息并开始刷屏以及显示Logo和一个旋转的立方体
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
#include "xl9555.h"
#include "spilcd.h"
#include "alientek_logo.h"
#include "demo_show.h"


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

    /* 刷屏测试 */
    lcd_clear(BLACK);
    delay(500);
    lcd_clear(RED);
    delay(500);
    lcd_clear(GREEN);
    delay(500);
    lcd_clear(BLUE);
    delay(500);
    lcd_clear(YELLOW);
    delay(500);
    lcd_clear(WHITE);
    delay(500);

    lcd_show_pic(0, 0, 240, 82, ALIENTEK_LOGO);                           /* LCD显示ALIENTEK图片 */
    lcd_show_string(10, 100, 200, 32, LCD_FONT_32, "ESP32-S3", RED);      /* LCD显示32号字体ESP32S3 */
    lcd_show_string(10, 132, 200, 24, LCD_FONT_24, "TFTLCD TEST", RED);   /* LCD显示24号字体TFTLCD TEST */
    lcd_show_string(10, 156, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED); /* LCD显示16号字体ATOM@ALIENTEK */
    delay(500);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    demo_show_cube();   /* 演示立方体3D旋转 */
}
