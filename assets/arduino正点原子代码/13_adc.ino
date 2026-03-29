/**
 ******************************************************************************
 * @file     13_adc.ino
 * @author   正点原子团队(正点原子)
 * @version  V1.0
 * @date     2023-08-08
 * @brief    ADC实验
 * @license  Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * 
 * 实验目的：学习ADC外设的使用
 *
 * 硬件资源及引脚分配：
 * 1,   UART0 --> ESP32S3 IO
 *       TXD0 --> IO43
 *       RXD0 --> IO44
 * 2,  XL9555 --> ESP32S3 IO
 *        SCL --> IO42
 *        SDA --> IO41
 *        INT --> IO0(跳线帽连接) 
 * 3, SPI_LCD --> ESP32S3 IO / XL9555
 *         CS --> IO21
 *        SCK --> IO12
 *        SDA --> IO11
 *         DC --> IO40(跳线帽连接)
 *        PWR --> XL9555_P13
 *        RST --> XL9555_P12
 * 4,      RV --> ESP32S3 IO
 *     ADC_IN --> IO8(跳线帽连接)
 *
 * 实验现象：
 * 1, 在LCD上显示可调变阻器的ADC值以及电压值，调节变阻器可改变检测电压
 * 
 * 注意事项：
 * 1, 需要跳线帽连接AIN和RV
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
#include "xl9555.h"
#include "spilcd.h"
#include "adc.h"


uint16_t adc_value = 0;
float adc_vol = 0;

/**
 * @brief    当程序开始执行时，将调用setup()函数，通常用来初始化变量、函数等
 * @param    无
 * @retval   无
 */
void setup() 
{
    uart_init(0, 115200);   /* 串口0初始化 */
    xl9555_init();          /* IO扩展芯片初始化 */
    lcd_init();             /* LCD初始化 */

    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "ADC TEST", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "ADC_VAL:", BLUE);
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "ADC_VOL:0.000V", BLUE);   /* 先在固定位置显示小数点 */
}

/**
 * @brief    循环函数，通常放程序的主体或者需要不断刷新的语句
 * @param    无
 * @retval   无
 */
void loop() 
{
    adc_value = adc_get(ADC_IN_PIN);                                                      /* 读取GPIO8引脚的ADC值 */
    lcd_show_xnum(94, 110, adc_value, 5, LCD_FONT_16, NUM_SHOW_NOZERO, BLUE);             /* 显示ADC采样后的原始值 */

    adc_vol = (float)adc_value * 3.3 / 4095;                                              /* 换算得到电压值 */
    adc_value = adc_vol;                                                                  /* 赋值整数部分给adc_value变量，因为adc_value为u16整形 */
    lcd_show_xnum(94, 130, (uint16_t)adc_value, 1, LCD_FONT_16, NUM_SHOW_NOZERO, BLUE);   /* 显示电压值的整数部分，3.1111的话，这里就是显示3 */

    adc_vol -= adc_value;                                                                 /* 把已经显示的整数部分去掉，留下小数部分，比如3.1111-3=0.1111 */
    adc_vol *= 1000;                                                                      /* 小数部分乘以1000，例如：0.1111就转换为111.1，相当于保留三位小数。 */
    lcd_fill(110, 130, 110 + 24, 130 + 16, WHITE);
    lcd_show_xnum(110, 130, adc_vol, 3, LCD_FONT_16, NUM_SHOW_ZERO, BLUE);                /* 显示小数部分（前面转换为了整形显示），这里显示的就是111. */

    delay(500);
}
