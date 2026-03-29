/**
 ******************************************************************************
 * @file     audio.h
 * @brief    音频播放模块 - DNESP32-S3版
 * @author   基于正点原子例程修改
 * 
 * 功能：
 * 1. I2S音频输出
 * 2. MP3解码播放
 * 3. 网络音频流播放
 * 
 * 依赖库：
 * 1. ESP8266Audio - MP3解码
 * 2. ESP8266SAM - 可选，TTS合成
 ******************************************************************************
 */

#ifndef __AUDIO_H
#define __AUDIO_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

/* 音频状态 */
typedef enum {
    AUDIO_STATE_IDLE = 0,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
    AUDIO_STATE_ERROR
} audio_state_t;

/* 函数声明 */
bool audio_init(void);
void audio_set_volume(uint8_t volume);
uint8_t audio_get_volume(void);
bool audio_play_url(const char* url);
bool audio_play_wav_url(const char* url);
bool audio_play_network(const char* url);
void audio_beep(int duration_ms);
void audio_stop(void);
void audio_pause(void);
void audio_resume(void);
audio_state_t audio_get_state(void);
bool audio_is_playing(void);

#ifdef USE_MP3_DECODER
bool audio_play_mp3_url(const char* url);
void audio_stop_mp3(void);
#endif

#endif /* __AUDIO_H */
