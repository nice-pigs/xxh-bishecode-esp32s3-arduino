/**
 ******************************************************************************
 * @file     09_iic_exio.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    IO扩展实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习IO扩展芯片XL9555的使用
 *
 * 硬件资源及引脚分配：
 * 1,    LED --> ESP32S3 IO
 *       LED --> IO1
 * 2,  UART0 --> ESP32S3 IO
 *      TXD0 --> IO43
 *      RXD0 --> IO44
 * 3, XL9555 --> ESP32S3 IO
 *       SCL --> IO42
 *       SDA --> IO41
 *       INT --> IO0(跳线帽连接) 
 *
 * 实验现象：
 * 1, 按下开发板上KEY0和KEY1控制蜂鸣器工作，按下KEY2和KEY3控制LED灯亮灭
 * 
 * 注意事项：
 * 1, 需要用跳线帽连接IO0和IIC_INT
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

#include "xl9555.h"
#include "led.h"
#include "uart.h"


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

    xl9555_io_config(KEY0 | KEY1 | KEY2 | KEY3, IO_SET_INPUT);  /* 初始化IO扩展芯片用作按键的引脚为输入状态 */
    xl9555_io_config(BEEP, IO_SET_OUTPUT);                      /* 初始化IO扩展芯片用作蜂鸣器控制的引脚为输出状态 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    if (xl9555_get_pin(KEY0) == 0)
    {
        Serial.printf("KEY0 is pressed, BEEP is on \r\n");
        xl9555_pin_set(BEEP, IO_SET_LOW);
    }
    else if (xl9555_get_pin(KEY1) == 0)
    {
        Serial.printf("KEY1 is pressed, BEEP is off \r\n");
        xl9555_pin_set(BEEP, IO_SET_HIGH);
    }
    else if (xl9555_get_pin(KEY2) == 0)
    {
        Serial.printf("KEY2 is pressed, LED is on \r\n");
        LED(0);
    }
    else if (xl9555_get_pin(KEY3) == 0)
    {
        Serial.printf("KEY3 is pressed, LED is off \r\n");
        LED(1);
    }

    delay(200);
}
