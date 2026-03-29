/**
 ******************************************************************************
 * @file     14_iic_ap3216c.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    光环境传感器实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习光环境传感器AP3216C的使用,实现光照强度(ALS)/接近距离(PS)/红外光强(IR)等的测量
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
 * 4, AP3216C --> ESP32S3 IO
 *        SCL --> 40
 *        SDA --> 41
 *        INT --> XL9555_P00
 *
 * 实验现象：
 * 1, LCD会显示光环境传感器的ALS+PS+IR数据
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
#include "ap3216c.h"


uint16_t ir, als, ps;       /* 光照强度(ALS)/接近距离(PS)/红外光强(IR) */

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

    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "AP3216C TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);

    while (ap3216c_init())  /* 检测不到AP3216C */
    {
        lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "AP3216C Check Failed!", RED);
        delay(500);
    }
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "AP3216C Ready!", RED);
        
    lcd_show_string(30, 160, 200, 16, LCD_FONT_16, " IR:", RED);
    lcd_show_string(30, 180, 200, 16, LCD_FONT_16, " PS:", RED);
    lcd_show_string(30, 200, 200, 16, LCD_FONT_16, "ALS:", RED);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    ap3216c_read_data(&ir, &ps, &als);                      /* 读取数据  */

    lcd_show_num(30 + 32, 160, ir, 5, LCD_FONT_16, BLUE);   /* 显示IR数据 */
    lcd_show_num(30 + 32, 180, ps, 5, LCD_FONT_16, BLUE);   /* 显示PS数据 */
    lcd_show_num(30 + 32, 200, als, 5, LCD_FONT_16, BLUE);  /* 显示ALS数据  */

    delay(500); 
}
