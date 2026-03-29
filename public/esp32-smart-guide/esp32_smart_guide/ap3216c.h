/**
 ****************************************************************************************************
 * @file        24c02.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-08-18
 * @brief       24C02 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32S3开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20230818
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __AP3216C_H
#define __AP3216C_H

#include "Arduino.h"

/* 引脚定义 */
#define IIC_SCL         42
#define IIC_SDA         41

#define AP3216C_ADDR    0X1E    /* 7位器件地址 */

/* 函数声明 */
uint8_t ap3216c_init(void);                                         /* AP3216C初始化函数 */
void ap3216c_write_one_byte(uint8_t reg, uint8_t data);             /* 写AP3216C寄存器函数 */
uint8_t ap3216c_read_one_byte(uint8_t reg);                         /* 读取AP3216C寄存器数据函数 */
void ap3216c_read_data(uint16_t *ir, uint16_t *ps, uint16_t *als);  /* 读取AP3216C的数据 */

#endif
