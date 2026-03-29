/**
 ******************************************************************************
 * @file     08_iic_eeprom.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-12-01
 * @brief    EEPROM 实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习IIC外设的使用，对EEPROM器件进行读写操作
 *
 * 硬件资源及引脚分配： 
 * 1,    KEY --> ESP32S3 IO
 *       KEY --> IO0
 * 2,  UART0 --> ESP32S3 IO
 *      TXD0 --> IO43
 *      RXD0 --> IO44
 * 3, EEPROM --> ESP32S3 IO
 *       SCL --> IO42
 *       SDA --> IO41
 * 
 * 实验现象：
 * 1, 检测IIC总线上是否有24C02器件，按下KEY会写入数据到EEPROM，每隔1秒钟会打印出EEPROM的0地址开始存放的有效内容
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
 
#include "24c02.h"
#include "key.h"
#include "uart.h"


const uint8_t g_text_buf[] = {"ESP32S3 IIC TEST"};  /* 要写入到24c02的字符串数组 */
#define TEXT_SIZE   sizeof(g_text_buf)              /* TEXT字符串长度 */
uint8_t datatemp[TEXT_SIZE];                        /* 从EEPROM读取到的数据 */

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    key_init();               /* KEY初始化 */
    uart_init(0, 115200);     /* 串口0初始化 */
    at24c02_init();           /* 初始化24CXX */
    
    while (at24c02_check())   /* 检测不到24c02 */
    {
        Serial.println("24C02 Check Failed!");
        delay(500);
    }
    Serial.println("24C02 Ready!");
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    at24c02_read(0, datatemp, TEXT_SIZE);                       /* 从24C02的0地址处中读取TEXT_SIZE长度数据 */
    Serial.printf("The Data Readed Is:%s \r\n", datatemp);

    if (KEY == 0)
    {
        at24c02_write(0, (uint8_t *)g_text_buf, TEXT_SIZE);     /* 向24C02的0地址处写入TEXT_SIZE长度数据 */
        Serial.printf("24C02 Write %s Finished! \r\n", g_text_buf);
    }

    delay(1000);
}
