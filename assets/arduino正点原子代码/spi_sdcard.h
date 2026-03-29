/**
 ****************************************************************************************************
 * @file        spi_sdcard.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       SDCARD 驱动代码
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

#ifndef __SPI_SDCARD_H
#define __SPI_SDCARD_H

#include "Arduino.h"


/* 引脚定义 */
#define SD_CS_PIN         2   
#define SD_MISO_PIN       13 
#define SD_MOSI_PIN       11
#define SD_SCK_PIN        12

/* 函数声明 */
uint8_t sdcard_init(void);    /* 初始化SD卡 */
void sd_test(void);           /* SD卡测试函数 */

#endif