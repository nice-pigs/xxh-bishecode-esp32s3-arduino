/**
 ******************************************************************************
 * @file     19_iic_qma6100p.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    三轴加速度计 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习三轴加速度计的使用
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
 * 5, QMA6100P--> ESP32S3 IO
 *        SCL --> IO40
 *        SDA --> IO41
 *        INT --> XL9555_P01
 *
 * 实验现象：
 * 1, LCD会显示三轴加速度传感器的俯仰角pitch和横滚角roll数据
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
#include "qma6100p.h"
#include "imu.h"


float g_acc_data[3];
float g_angle_data[2];

/**
 * @brief       显示角度
 * @param       x, y : 坐标
 * @param       title: 标题
 * @param       angle: 角度
 * @retval      无
 */
void user_show_angle(uint16_t x, uint16_t y, char *title, float angle)
{
    char buf[20];

    sprintf(buf,"%s%3.1f", title, angle);                     /* 格式化输出 */
    lcd_fill(x, y, x + 160, y + 16, WHITE);                   /* 清除上次数据(最多显示20个字符,20*8=160) */
    lcd_show_string(x, y, 160, 16, LCD_FONT_16, buf, BLUE);   /* 显示字符串 */
}

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
    qma6100p_init();        /* 三轴加速度计初始化 */
    
    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "IMU TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    qma6100p_read_acc_xyz(g_acc_data);
    acc_get_angle(g_acc_data, g_angle_data);

    user_show_angle(30, 130, "Pitch :", g_angle_data[0]);
    user_show_angle(30, 150, " Roll :", g_angle_data[1]);

    LED_TOGGLE();
    delay(500);
}
