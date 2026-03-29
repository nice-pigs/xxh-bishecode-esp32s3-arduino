/**
 ****************************************************************************************************
 * @file        dht11.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       DHT11 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20231201
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __DHT11_H
#define __DHT11_H

#include "Arduino.h"
#include "esp_system.h"

/* 引脚定义 - DNESP32-S3: IO0 */
#define DHT11_DQ_PIN      GPIO_NUM_0       

/* 宏函数定义 */
#define DHT11_DQ_OUT(x)   gpio_set_level(DHT11_DQ_PIN, x)       //digitalWrite(DHT11_DQ, x) 
#define DHT11_DQ_IN       gpio_get_level(DHT11_DQ_PIN)          //digitalRead(DHT11_DQ)

#define DHT11_MODE_IN     gpio_set_direction(DHT11_DQ_PIN, GPIO_MODE_INPUT)   //pinMode(DHT11_DQ, INPUT_PULLUP)
#define DHT11_MODE_OUT    gpio_set_direction(DHT11_DQ_PIN, GPIO_MODE_OUTPUT)  //pinMode(DHT11_DQ, OUTPUT)

/* 函数声明 */
uint8_t dht11_init(void);                               /* dht11初始化函数 */
uint8_t dht11_check(void);                              /* 检测是否存在DHT11 */
uint8_t dht11_read_data(uint8_t *temp, uint8_t *humi);  /* 读取温湿度 */

#endif
