/**
 ******************************************************************************
 * @file     es8388.h
 * @brief    ES8388音频编解码芯片驱动 - DNESP32-S3版
 * @author   基于正点原子官方驱动修改
 ******************************************************************************
 */

#ifndef __ES8388_H
#define __ES8388_H

#include <Arduino.h>
#include <Wire.h>

/* ES8388 I2C地址 (7位地址) */
#define ES8388_ADDR         0x10    /* ES8388 I2C地址 */

/* I2S引脚定义 - DNESP32-S3（以标准例程myi2s.h为准）*/
#define I2S_MCLK_PIN        3       /* GPIO3  - MCLK（必须提供，ES8388需要主时钟）*/
#define I2S_SCK_PIN         46      /* GPIO46 - BCLK */
#define I2S_LRCK_PIN        9       /* GPIO9  - LRCK/WS */
#define I2S_SDOUT_PIN       10      /* GPIO10 - 播放输出(ES8388_SDIN) */
#define I2S_SDIN_PIN        14      /* GPIO14 - 录音输入(ES8388_SDOUT) */

/* 音量范围 */
#define ES8388_VOL_MIN      0       /* 最小音量 */
#define ES8388_VOL_MAX      100     /* 最大音量 */

/* 函数声明 */
bool es8388_init(void);
void es8388_set_volume(uint8_t volume);
void es8388_hpvol_set(uint8_t volume);
void es8388_spkvol_set(uint8_t volume);
uint8_t es8388_get_volume(void);
void es8388_mute(bool enable);
void es8388_play_mode(void);
void es8388_record_mode(void);
void es8388_stop(void);
bool es8388_write_reg(uint8_t reg, uint8_t val);
uint8_t es8388_read_reg(uint8_t reg);
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen);
void es8388_input_cfg(uint8_t in);
void es8388_output_cfg(uint8_t o1en, uint8_t o2en);

#endif /* __ES8388_H */
