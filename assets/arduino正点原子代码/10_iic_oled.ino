/**
 ******************************************************************************
 * @file     10_iic_oled.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    OLED显示实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习OLED模块的使用
 *
 * 硬件资源及引脚分配：
 * 1,  UART0 --> ESP32S3 IO
 *      TXD0 --> IO43
 *      RXD0 --> IO44
 * 2, XL9555 --> ESP32S3 IO
 *       SCL --> IO42
 *       SDA --> IO41
 *       INT --> IO0(跳线帽连接) 
 * 3,   OLED --> ESP32S3 IO
 *   D0(SCL) --> IO40
 *   D1(SDA) --> IO41
 *        D2 --> IO6
 *        DC --> IO38
 *
 * 实验现象：
 * 1, 把设置好IIC通信接口的OLED模块插入开发板左下角的OLED/CAMERA模块接口，OLED模块不停地显示ASCII码和码值
 * 
 * 注意事项：
 * 1, 需要在软件中选择"项目"-->"加载库"-->"添加一个.ZIP库..."-->选择到资料包目录下的1个压缩文件包“esp8266-oled-ssd1306-master.zip”安装即可。
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

#include "oled.h"
#include "uart.h"
#include "xl9555.h"


/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    uart_init(0, 115200);   /* 串口0初始化 */
    xl9555_init();          /* IO扩展芯片初始化 */
    oled_init();            /* OLED模块初始化 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    oled_show_demo();
}
