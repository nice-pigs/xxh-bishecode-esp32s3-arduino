/**
 ******************************************************************************
 * @file     06_watch_dog.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    看门狗实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习看门狗的使用(定时器虚拟看门狗功能)
 *
 * 硬件资源及引脚分配： 
 * 1,   KEY --> ESP32S3 IO
 *      KEY --> IO0
 * 2, UART0 --> ESP32S3 IO
 *     TXD0 --> IO43
 *     RXD0 --> IO44
 * 
 * 实验现象：
 * 1, 串口打印运行时间信息，当按键按下时，运行时间变长，导致无法在1.2秒内进行喂狗操作，而进入到定时器中断回调函数中执行软件复位操作
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
#include "key.h"
#include "watchdog.h"


#define wdg_timeout 1200    /* 看门狗定时时间,1200ms */

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    key_init();                           /* KEY初始化 */
    uart_init(0, 115200);                 /* 串口0初始化 */
    Serial.println("running setup");      /* 打印标志性信息 方便查看系统开始 */

    wdg_init(wdg_timeout * 1000, 80);     /* 初始化看门狗,80分频,定时时间1.2秒 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    Serial.println("running main loop");  /* 打印标志性信息 方便查看系统开始 */

    timerWrite(wdg_timer, 0);             /* 复位定时器(喂狗) */

    long looptime = millis();             /* 通过millis函数获取开发板开始运行当前程序以来经过的毫秒数 */

    while (!KEY)                          /* 按下按键会延时500ms,最终会导致looptime时间变为1.5秒,还没有来得及喂狗,就进入到定时器中断回调函数中复位 */
    {
        Serial.println("key pressed, delay_500ms");
        delay(500);
    }

    delay(1000);                          
    looptime = millis() - looptime;       /* 监控上过程运行时间 */

    Serial.print("loop time is = ");      /* 打印上过程运行时间 */
    Serial.println(looptime); 
}
