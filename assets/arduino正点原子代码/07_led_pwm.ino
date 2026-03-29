/**
 ******************************************************************************
 * @file     07_led_pwm.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    LED PWM实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习LED PWM外设的使用
 *
 * 硬件资源及引脚分配： 
 * 1, LED --> ESP32S3 IO
 *    LED --> IO1
 * 
 * 实验现象：
 * 1, LED指示灯实现呼吸灯效果，由暗变亮，再由亮变暗
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

#include "pwm.h"


uint16_t g_ledpwmval = 0;     /* 占空比值 */
uint8_t g_dir = 1;            /* 变化方向（1增大 0减小） */

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    led_pwm_init(1000, 10);   /* LED PWM初始化,PWM输出频率为1000HZ,占空比分辨率为10 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    if (g_dir)
    {
        g_ledpwmval += 5;
    }
    else
    {
        g_ledpwmval -= 5;
    }

    if (g_ledpwmval > 1005)
    {
        g_dir = 0;
    }

    if (g_ledpwmval < 5)
    {
        g_dir = 1;
    }

    pwm_set_duty(g_ledpwmval);
    delay(10);
}