/**
 ****************************************************************************************************
 * @file        ap3216c.cpp
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       AP3216C 驱动代码
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
 * V1.0 20230818
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "ap3216c.h"
#include <Wire.h>

/**
 * @brief       初始化光环境传感器
 * @param       无
 * @retval      0:初始化成功；1:初始化失败
 */
uint8_t ap3216c_init(void)
{
    uint8_t temp;

    Wire.begin(IIC_SDA, IIC_SCL, 400000);   /* 初始化IIC连接 */

    ap3216c_write_one_byte(0x00, 0X04);     /* 复位AP3216C */
    delay(50);                              /* AP3216C复位至少10ms */

    ap3216c_write_one_byte(0x00, 0X03);     /* 开启ALS、PS+IR */

    temp = ap3216c_read_one_byte(0X00);     /* 读取刚刚写进去的0X03 */
    if (temp == 0X03)
    {
        return 0;           /* AP3216C正常 */
    }
    else 
    {
        return 1;           /* AP3216C失败 */
    }
}

/**
 * @brief       向ap3216c指定寄存器写入一个数据
 * @param       reg: 要写入的寄存器
 * @param       data: 要写入的数据
 * @retval      无
 */
void ap3216c_write_one_byte(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(AP3216C_ADDR);     /* 发送从机的7位器件地址到发送队列 */
    Wire.write(reg);                          /* 发送要写入从机寄存器的地址到发送队列 */
    Wire.write(data);                         /* 发送要写入从机寄存器的数据到发送队列 */
    Wire.endTransmission();                   /* IIC 发送 发送队列的数据(不带参数,表示发送stop信号,结束传输) */         
}

/**
 * @brief       在ap3216c指定寄存器读出一个数据
 * @param       reg: 要读取的寄存器
 * @retval      寄存器的值 / 0xFF:未接收到数据
 */
uint8_t ap3216c_read_one_byte(uint8_t reg)
{
    uint8_t rd_num = 0;
    uint8_t rd_data = 0;

    Wire.beginTransmission(AP3216C_ADDR);       /* 发送从机的7位器件地址到发送队列 */
    Wire.write(reg);                            /* 发送要读取从机的寄存器地址到发送队列 */
    Wire.endTransmission(0);                    /* IIC 发送 发送队列的数据(传参为0,表示重新发送一个start信号,保持IIC总线有效连接) */

    rd_num = Wire.requestFrom(AP3216C_ADDR, 1); /* 主机向从机发送数据请求,并获取到数据 */

    if (Wire.available() != 0)                  /* 得到已经接收到的数据字节数 */
    {
        return Wire.read();                     /* 到数据缓冲区读取数据 */
    }

    return 0xFF;
}

/**
 * @brief       读取AP3216C的数据
 * @note        读取原始数据，包括ALS,PS和IR
 *              如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param       ir      : IR传感器值
 * @param       ps      : PS传感器值
 * @param       als     : ALS传感器值
 * @retval      无
 */
void ap3216c_read_data(uint16_t *ir, uint16_t *ps, uint16_t *als)
{
    uint8_t buf[6];
    uint8_t i;

    for (i = 0; i < 6; i++)
    {
        buf[i] = ap3216c_read_one_byte(0X0A + i);   /* 循环读取所有传感器数据 */
    }

    if (buf[0] & 0X80)
    {
        *ir = 0;                                                    /* IR_OF位为1,则数据无效 */
    }
    else 
    {
        *ir = ((uint16_t)buf[1] << 2) | (buf[0] & 0X03);            /* 读取IR传感器的数据 */
    }

    *als = ((uint16_t)buf[3] << 8) | buf[2];                        /* 读取ALS传感器的数据 */ 

    if (buf[4] & 0x40) 
    {
        *ps = 0;                                                    /* IR_OF位为1,则数据无效 */
    }
    else
    {
        *ps = ((uint16_t)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);   /* 读取PS传感器的数据 */
    }
}

