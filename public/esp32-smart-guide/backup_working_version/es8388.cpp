/**
 ******************************************************************************
 * @file     es8388.cpp
 * @brief    ES8388音频编解码芯片驱动 - DNESP32-S3版
 * @author   基于正点原子官方驱动修改
 * 
 * 注意：ES8388与XL9555共用I2C0总线(GPIO41/42)
 * 使用driver/i2c.h底层驱动，与XL9555共享I2C
 ******************************************************************************
 */

#include "es8388.h"
#include "xl9555.h"  // 用于扬声器使能
#include <driver/i2c.h>

/* 使用I2C_NUM_0，与XL9555共用 */
#define ES8388_I2C_NUM      I2C_NUM_0
#define ES8388_I2C_TIMEOUT  1000

/* 当前音量 */
static uint8_t current_volume = 80;  /* 音量调高，喇叭输出 */
static bool es8388_detected = false;

/**
 * @brief  写ES8388寄存器（使用driver/i2c.h）
 * @param  reg: 寄存器地址
 * @param  val: 写入值
 * @retval true:成功 false:失败
 */
bool es8388_write_reg(uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8388_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(ES8388_I2C_NUM, cmd, pdMS_TO_TICKS(ES8388_I2C_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    
    return (ret == ESP_OK);
}

/**
 * @brief  读ES8388寄存器（使用driver/i2c.h）
 * @param  reg: 寄存器地址
 * @retval 寄存器值
 */
uint8_t es8388_read_reg(uint8_t reg)
{
    uint8_t data = 0;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8388_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8388_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    i2c_master_cmd_begin(ES8388_I2C_NUM, cmd, pdMS_TO_TICKS(ES8388_I2C_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    
    return data;
}

/**
 * @brief  检测ES8388是否存在
 * @retval true:存在 false:不存在
 */
static bool es8388_check_device(void)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ES8388_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(ES8388_I2C_NUM, cmd, pdMS_TO_TICKS(ES8388_I2C_TIMEOUT));
    i2c_cmd_link_delete(cmd);
    
    return (ret == ESP_OK);
}

/**
 * @brief  初始化ES8388（使用正点原子官方初始化序列）
 * @retval true:成功 false:失败
 */
bool es8388_init(void)
{
    Serial.println("ES8388初始化...");
    
    /* ⚠️ 重要：使能扬声器功放！SPK_EN 低电平有效 */
    xl9555_pin_set(SPK_EN, IO_SET_LOW);
    Serial.println("扬声器功放已使能");
    
    /* 检测ES8388是否存在（I2C0已由XL9555初始化）*/
    if (!es8388_check_device()) {
        Serial.println("ES8388未检测到！地址: 0x10");
        Serial.println("请检查：");
        Serial.println("1. ES8388芯片是否焊接正确");
        Serial.println("2. I2C总线GPIO41/42连接是否正常");
        return false;
    }
    
    Serial.println("ES8388已检测到，开始配置...");
    es8388_detected = true;
    
    /* 以下使用正点原子官方初始化序列 */
    
    /* 软复位ES8388 */
    es8388_write_reg(0x00, 0x80);   /* 软复位 */
    es8388_write_reg(0x00, 0x00);   /* 退出复位 */
    delay(100);                     /* 等待复位完成 */
    
    /* 电源管理配置 */
    es8388_write_reg(0x01, 0x58);   /* Chip Control 1 */
    es8388_write_reg(0x01, 0x50);
    es8388_write_reg(0x02, 0xF3);   /* Chip Control 2 */
    es8388_write_reg(0x02, 0xF0);
    
    /* ADC电源管理 - 麦克风偏置电源关闭 */
    es8388_write_reg(0x03, 0x09);
    
    /* 使能参考 500K驱动使能 */
    es8388_write_reg(0x00, 0x06);
    
    /* DAC电源管理 */
    es8388_write_reg(0x04, 0x00);   /* 暂不打开任何通道 */
    
    /* MCLK不分频 */
    es8388_write_reg(0x08, 0x00);
    
    /* DAC控制 DACLRC与ADCLRC相同 */
    es8388_write_reg(0x2B, 0x80);
    
    /* ADC L/R PGA增益配置为+24dB */
    es8388_write_reg(0x09, 0x88);
    
    /* ADC数据选择为left data = left ADC, right data = left ADC，音频数据为16bit */
    es8388_write_reg(0x0C, 0x4C);
    
    /* ADC配置 MCLK/采样率=256 */
    es8388_write_reg(0x0D, 0x02);
    
    /* ADC数字音量控制 */
    es8388_write_reg(0x10, 0x00);   /* L */
    es8388_write_reg(0x11, 0x00);   /* R */
    
    /* DAC 音频数据为16bit */
    es8388_write_reg(0x17, 0x18);
    
    /* DAC 配置 MCLK/采样率=256 */
    es8388_write_reg(0x18, 0x02);
    
    /* DAC数字音量控制 */
    es8388_write_reg(0x1A, 0x00);   /* L */
    es8388_write_reg(0x1B, 0x00);   /* R */
    
    /* 混频器配置 */
    es8388_write_reg(0x27, 0xB8);   /* L混频器 */
    es8388_write_reg(0x2A, 0xB8);   /* R混频器 */
    
    delay(100);
    
    /* 打开DAC输出 */
    es8388_adda_cfg(1, 0);          /* DAC使能，ADC关闭 */
    es8388_output_cfg(1, 1);        /* 通道1和通道2都使能（喇叭接OUT2）*/
    
    /* 设置初始音量 */
    es8388_set_volume(current_volume);
    
    Serial.println("ES8388初始化完成");
    return true;
}

/**
 * @brief  设置耳机音量 (0-33)
 * @param  volume: 音量值 (0-33)
 */
void es8388_hpvol_set(uint8_t volume)
{
    if (volume > 33) volume = 33;
    
    /* 设置耳机音量 */
    es8388_write_reg(0x2E, volume);  /* 耳机L */
    es8388_write_reg(0x2F, volume);  /* 耳机R */
}

/**
 * @brief  设置喇叭音量 (0-33)
 * @param  volume: 音量值 (0-33)
 */
void es8388_spkvol_set(uint8_t volume)
{
    if (volume > 33) volume = 33;
    
    /* 设置喇叭音量 */
    es8388_write_reg(0x30, volume);  /* 喇叭L */
    es8388_write_reg(0x31, volume);  /* 喇叭R */
}

/**
 * @brief  设置音量 (0-100)
 * @param  volume: 音量值 (0-100)
 */
void es8388_set_volume(uint8_t volume)
{
    if (volume > 100) volume = 100;
    current_volume = volume;
    
    /* 音量范围: 0-33, 对应 -45dB ~ +4.5dB */
    /* 转换: volume(0-100) -> es_vol(0-33) */
    uint8_t es_vol = (volume * 33) / 100;
    if (es_vol > 33) es_vol = 33;
    
    /* 设置耳机和喇叭音量 */
    es8388_hpvol_set(es_vol);
    es8388_spkvol_set(es_vol);
}

/**
 * @brief  获取当前音量
 * @retval 音量值 (0-100)
 */
uint8_t es8388_get_volume(void)
{
    return current_volume;
}

/**
 * @brief  静音控制
 * @param  enable: true=静音 false=取消静音
 */
void es8388_mute(bool enable)
{
    if (enable) {
        es8388_write_reg(0x1A, 0x00);  /* DAC L音量最小 */
        es8388_write_reg(0x1B, 0x00);  /* DAC R音量最小 */
    } else {
        es8388_set_volume(current_volume);
    }
}

/**
 * @brief  ES8388 DAC/ADC配置
 * @param  dacen: dac使能(1) / 关闭(0)
 * @param  adcen: adc使能(1) / 关闭(0)
 */
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen)
{
    uint8_t tempreg = 0;
    tempreg |= !dacen << 0;
    tempreg |= !adcen << 1;
    tempreg |= !dacen << 2;
    tempreg |= !adcen << 3;
    es8388_write_reg(0x02, tempreg);
}

/**
 * @brief  ES8388 ADC输入通道配置
 * @param  in: 输入通道
 *    @arg   0, 通道1输入
 *    @arg   1, 通道2输入
 * @retval 无
 */
void es8388_input_cfg(uint8_t in)
{
    es8388_write_reg(0x0A, (5 * in) << 4);   /* ADC1 输入通道选择L/R	INPUT1 */
}

/**
 * @brief  ES8388 DAC输出通道配置
 * @param  o1en: 通道1使能(1)/禁止(0)
 * @param  o2en: 通道2使能(1)/禁止(0)
 */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en)
{
    uint8_t tempreg = 0;
    tempreg |= o1en * (3 << 4);
    tempreg |= o2en * (3 << 2);
    es8388_write_reg(0x04, tempreg);
}

/**
 * @brief  设置为播放模式
 */
void es8388_play_mode(void)
{
    es8388_adda_cfg(1, 0);  /* DAC开，ADC关 */
    es8388_output_cfg(1, 1);  /* 通道1和通道2都使能 */
}

/**
 * @brief  设置为录音模式
 */
void es8388_record_mode(void)
{
    es8388_adda_cfg(0, 1);  /* DAC关，ADC开 */
    es8388_output_cfg(0, 0);
}

/**
 * @brief  停止音频
 */
void es8388_stop(void)
{
    es8388_adda_cfg(0, 0);  /* DAC和ADC都关闭 */
    es8388_output_cfg(0, 0);
}
