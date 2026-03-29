/**
 ******************************************************************************
 * @file     04_uart.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    串口实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习UART外设的使用，进行串口数据接收和发送
 *
 * 硬件资源及引脚分配： 
 * 1, UART0 --> ESP32S3 IO
 *     TXD0 --> IO43
 *     RXD0 --> IO44
 * 
 * 实验现象：
 * 1, 打印芯片参数，打开串口调试助手，选择好串口，波特率115200等参数，在串口助手上发送数据，ESP32S3串口接收后回传到串口助手显示
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


uint32_t chip_id = 0;       /* 芯片ID */

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    uart_init(0, 115200);   /* 串口0初始化 */

    for(int i = 0; i < 17; i = i + 8) 
    {
        chip_id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;       /* 获取ESP32芯片MAC地址(6Byte)，该地址也可作为芯片ID */
    }

    Serial.printf("ESP32 Chip model = %s Rev %d \n", ESP.getChipModel(), ESP.getChipRevision());   /* 打印芯片类型和芯片版本号 */
    Serial.printf("This chip has %d cores \n", ESP.getChipCores());     /* 打印芯片的内核数 */
    Serial.print("Chip ID: "); Serial.println(chip_id);                 /* 打印芯片ID */
    Serial.printf("CpuFreqMHz: %d MHz\n", ESP.getCpuFreqMHz());         /* 打印芯片主频 */
    Serial.printf("SdkVersion: %s \n", ESP.getSdkVersion());            /* 打印SDK版本 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop()
{
    Serial.println("Waitting for Serial Data  \n");   /* 等待串口助手发过来的串口数据 */

    while (Serial.available() > 0)                    /* 当串口0接收到数据 */
    {
        Serial.println("Serial Data Available...");   /* 通过串口监视器通知用户 */

        String serial_data;                           /* 存放接收到的串口数据 */

        int c = Serial.read();                        /* 读取一字节串口数据 */
        while (c >= 0)
        {
            serial_data += (char)c;                   /* 存放到serial_data变量中 */
            c = Serial.read();                        /* 继续读取一字节串口数据 */
        }

        // serial_data = Serial.readString();            /* 将接收到的信息使用readString()存储于serial_data变量(跟前面4行代码具有同样效果) */
        Serial.print("Received Serial Data: ");       /* 然后通过串口监视器输出serial_data变量内容 */
        Serial.println(serial_data);                  /* 以便查看serial_data变量的信息 */
    }

    delay(1000);
}
