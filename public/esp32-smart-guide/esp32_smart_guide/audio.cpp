/**
 ******************************************************************************
 * @file     audio.cpp
 * @brief    音频播放模块 - DNESP32-S3版
 * @author   基于正点原子例程修改
 * 
 * 说明：
 * 使用ESP32 I2S驱动播放音频
 * 支持PCM格式（服务器默认返回PCM格式）
 * 
 * 注意：ES8388使用内部PLL，无需外部MCLK
 ******************************************************************************
 */

#include "audio.h"
#include "es8388.h"
#include <driver/i2s.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

/* 引用主文件中定义的HTTPS客户端和代理地址 */
extern WiFiClientSecure httpsClient;
extern const char* PROXY_URL;

/* 是否使用ESP8266Audio库进行MP3解码 */
// #define USE_MP3_DECODER  /* 取消注释以启用MP3解码 */

#ifdef USE_MP3_DECODER
#include <AudioFileSourceHTTPStream.h>
#include <AudioFileSourceBuffer.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

/* MP3播放器对象 */
static AudioFileSourceHTTPStream *file = nullptr;
static AudioFileSourceBuffer *buff = nullptr;
static AudioGeneratorMP3 *mp3 = nullptr;
static AudioOutputI2S *out = nullptr;
#endif

/* I2S配置 */
#define I2S_NUM         I2S_NUM_0
#define I2S_SAMPLE_RATE 44100
#define I2S_BITS        I2S_BITS_PER_SAMPLE_16BIT

/* 音频状态 */
static audio_state_t audio_state = AUDIO_STATE_IDLE;
static uint8_t audio_volume = 100;  /* 音量调到最大，喇叭输出 */

/* I2S配置结构 */
static i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = true,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0  /* 不使用固定MCLK，ES8388使用内部PLL */
};

/* I2S引脚配置 */
static i2s_pin_config_t i2s_pin_config = {
    .mck_io_num = I2S_MCLK_PIN,          /* MCLK = GPIO3（ES8388必须有主时钟）*/
    .bck_io_num = I2S_SCK_PIN,           /* BCK = GPIO46 */
    .ws_io_num = I2S_LRCK_PIN,           /* LRCK/WS = GPIO9 */
    .data_out_num = I2S_SDOUT_PIN,       /* DOUT = GPIO10 (ES8388_SDIN) */
    .data_in_num = I2S_SDIN_PIN          /* DIN  = GPIO14 (ES8388_SDOUT) */
};

/**
 * @brief  初始化音频系统
 * @retval true:成功 false:失败
 */
bool audio_init(void)
{
    Serial.println("音频系统初始化...");
    
    /* 初始化I2S驱动 */
    esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("I2S驱动安装失败: %d\n", err);
        return false;
    }
    
    /* 设置I2S引脚 */
    err = i2s_set_pin(I2S_NUM, &i2s_pin_config);
    if (err != ESP_OK) {
        Serial.printf("I2S引脚设置失败: %d\n", err);
        return false;
    }
    
    /* 设置I2S时钟 (立体声模式) */
    err = i2s_set_clk(I2S_NUM, I2S_SAMPLE_RATE, I2S_BITS, I2S_CHANNEL_STEREO);
    if (err != ESP_OK) {
        Serial.printf("I2S时钟设置失败: %d\n", err);
        return false;
    }
    
    Serial.println("I2S初始化完成");
    
    /* 初始化ES8388编解码器 */
    if (!es8388_init()) {
        Serial.println("ES8388初始化失败");
        return false;
    }
    
    /* 设置为播放模式 */
    es8388_play_mode();
    
    /* 设置初始音量 */
    es8388_set_volume(audio_volume);
    
    audio_state = AUDIO_STATE_IDLE;
    Serial.println("音频系统初始化完成");
    return true;
}

/**
 * @brief  设置音量 (0-100)
 * @param  volume: 音量值
 */
void audio_set_volume(uint8_t volume)
{
    if (volume > 100) volume = 100;
    audio_volume = volume;
    es8388_set_volume(volume);
}

/**
 * @brief  获取当前音量
 * @retval 音量值 (0-100)
 */
uint8_t audio_get_volume(void)
{
    return audio_volume;
}

/**
 * @brief  播放网络音频URL（MP3格式）
 * @param  url: 音频URL
 * @retval true:成功 false:失败
 */
bool audio_play_url(const char* url)
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi未连接，无法播放网络音频");
        return false;
    }
    
    Serial.printf("播放音频: %s\n", url);
    
    audio_state = AUDIO_STATE_PLAYING;
    es8388_play_mode();
    
    HTTPClient http;
    http.begin(httpsClient, url);  /* 使用WiFiClientSecure进行HTTPS请求 */
    http.setTimeout(30000);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("音频下载失败: %d\n", httpCode);
        audio_state = AUDIO_STATE_ERROR;
        http.end();
        return false;
    }
    
    /* 获取音频流大小 */
    int totalLen = http.getSize();
    int readLen = 0;
    
    Serial.printf("音频大小: %d bytes\n", totalLen);
    
    /* 创建缓冲区 */
    uint8_t* audioBuffer = (uint8_t*)malloc(4096);
    if (!audioBuffer) {
        Serial.println("内存不足");
        audio_state = AUDIO_STATE_ERROR;
        http.end();
        return false;
    }
    
    /* 读取并发送音频数据到I2S */
    WiFiClient* stream = http.getStreamPtr();
    
    while (http.connected() && (totalLen > 0 || totalLen == -1)) {
        size_t size = stream->available();
        if (size) {
            int c = stream->readBytes(audioBuffer, ((size > 4096) ? 4096 : size));
            
            /* 写入I2S（这里需要MP3解码器） */
            /* 简化版本：直接播放PCM数据 */
            /* 完整版本需要使用ESP8266Audio库解码MP3 */
            
            size_t bytesWritten;
            i2s_write(I2S_NUM, audioBuffer, c, &bytesWritten, portMAX_DELAY);
            
            readLen += c;
            if (totalLen > 0) {
                totalLen -= c;
            }
        }
        delay(1);
    }
    
    free(audioBuffer);
    http.end();
    
    audio_state = AUDIO_STATE_IDLE;
    Serial.println("音频播放完成");
    return true;
}

/**
 * @brief  停止播放
 */
void audio_stop(void)
{
    i2s_zero_dma_buffer(I2S_NUM);
    es8388_stop();
    audio_state = AUDIO_STATE_IDLE;
}

/**
 * @brief  暂停播放
 */
void audio_pause(void)
{
    if (audio_state == AUDIO_STATE_PLAYING) {
        es8388_mute(true);
        audio_state = AUDIO_STATE_PAUSED;
    }
}

/**
 * @brief  恢复播放
 */
void audio_resume(void)
{
    if (audio_state == AUDIO_STATE_PAUSED) {
        es8388_mute(false);
        audio_state = AUDIO_STATE_PLAYING;
    }
}

/**
 * @brief  获取音频状态
 * @retval 音频状态
 */
audio_state_t audio_get_state(void)
{
    return audio_state;
}

/**
 * @brief  检查是否正在播放
 * @retval true:正在播放 false:未播放
 */
bool audio_is_playing(void)
{
    return audio_state == AUDIO_STATE_PLAYING;
}

/**
 * @brief  播放提示音（简单的方波，更可靠）
 * @param  duration_ms: 持续时间（毫秒）
 */
void audio_beep(int duration_ms)
{
    #define BEEP_FREQ        1000   /* 1kHz蜂鸣，更柔和 */
    #define BEEP_SAMPLE_RATE 16000  /* 降低采样率，更稳定 */
    #define BEEP_BUF_SAMPLES 128    /* 更小的缓冲区 */

    int total_samples = (BEEP_SAMPLE_RATE * duration_ms) / 1000;
    int16_t buf[BEEP_BUF_SAMPLES * 2];  /* 立体声 */

    Serial.printf("播放提示音: %d ms, 音量:26（约80%%）\n", duration_ms);

    es8388_play_mode();
    es8388_set_volume(26);  /* 设为26，约80%音量 */

    /* 先切换到16kHz采样率 */
    i2s_set_clk(I2S_NUM, BEEP_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

    int played = 0;
    while (played < total_samples) {
        int batch = min(BEEP_BUF_SAMPLES, total_samples - played);
        for (int i = 0; i < batch; i++) {
            /* 简单的方波，幅度适中 */
            int16_t v = ((played + i) % (BEEP_SAMPLE_RATE / BEEP_FREQ / 2)) < (BEEP_SAMPLE_RATE / BEEP_FREQ / 4) ? 10000 : -10000;
            buf[i * 2]     = v;  /* 左声道 */
            buf[i * 2 + 1] = v;  /* 右声道 */
        }
        size_t bytesWritten;
        i2s_write(I2S_NUM, (uint8_t*)buf, batch * 4, &bytesWritten, portMAX_DELAY);
        played += batch;
    }

    /* 恢复默认采样率 */
    i2s_set_clk(I2S_NUM, I2S_SAMPLE_RATE, I2S_BITS, I2S_CHANNEL_STEREO);

    Serial.println("提示音播放完成");
}

/**
 * @brief  播放WAV格式的网络音频（简化版本）
 * @param  url: 音频URL
 * @note   此函数用于播放服务器返回的TTS WAV音频
 */
bool audio_play_wav_url(const char* url)
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi未连接");
        return false;
    }
    
    // 通过代理的 /api/audio?url= 接口下载音频，避免ESP32直连外部HTTPS失败
    String proxyAudioUrl = String(PROXY_URL) + "/api/audio?url=" + String(url);
    Serial.printf("播放WAV音频(via proxy): %s\n", proxyAudioUrl.c_str());
    
    WiFiClient httpClient;
    HTTPClient http;
    http.begin(httpClient, proxyAudioUrl);
    http.setTimeout(60000);  /* 60秒超时 */
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("下载失败: %d\n", httpCode);
        http.end();
        return false;
    }
    
    audio_state = AUDIO_STATE_PLAYING;
    
    /* 完全按照正点原子官方代码配置 */
    es8388_adda_cfg(1, 0);       /* 开启DAC关闭ADC */
    es8388_input_cfg(0);         /* 关闭输入 */
    es8388_output_cfg(1, 1);     /* DAC选择通道输出 */
    es8388_hpvol_set(26);        /* 设置耳机音量26（约80%）*/
    es8388_spkvol_set(26);       /* 设置喇叭音量26（约80%）*/
    
    Serial.println("ES8388配置完成，音量26（约80%）");
    
    /* 获取音频流 */
    WiFiClient* stream = http.getStreamPtr();
    
    /* 检测Content-Type判断格式 */
    String contentType = http.header("Content-Type");
    bool isPCM = (contentType.indexOf("pcm") >= 0) || (String(url).indexOf("pcm") >= 0);
    
    Serial.printf("Content-Type: %s, PCM模式: %s\n", contentType.c_str(), isPCM ? "是" : "否");
    
    /* 创建缓冲区 */
    uint8_t* audioBuffer = (uint8_t*)malloc(4096);
    if (!audioBuffer) {
        Serial.println("内存不足");
        audio_state = AUDIO_STATE_ERROR;
        http.end();
        return false;
    }
    
    if (isPCM) {
        /* PCM格式：原始音频数据，无需跳过头部 */
        /* 假设：24kHz, 16bit, 单声道（Coze TTS默认输出） */
        Serial.println("播放PCM格式音频...");
        
        /* ES8388是立体声codec，I2S必须用STEREO，否则时序错误没有声音 */
        /* 采样率16kHz匹配Coze TTS输出（sampleRate: 16000） */
        i2s_set_clk(I2S_NUM, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
        
        /* 立体声转换缓冲区：输入2048个单声道采样 -> 输出4096个立体声采样 */
        static int16_t stereoBuffer[4096];
        
        while (http.connected()) {
            size_t size = stream->available();
            if (size > 0) {
                /* 每次最多读2048字节（1024个单声道采样），转成立体声后刚好4096字节 */
                int c = stream->readBytes(audioBuffer, (size > 2048) ? 2048 : size);
                
                int16_t* src = (int16_t*)audioBuffer;
                int samples = c / 2;  /* 16bit = 2字节/采样 */
                
                /* 单声道 -> 立体声：左右声道复制 */
                for (int i = 0; i < samples; i++) {
                    stereoBuffer[i * 2]     = src[i];  /* 左声道 */
                    stereoBuffer[i * 2 + 1] = src[i];  /* 右声道 */
                }
                
                size_t bytesWritten;
                i2s_write(I2S_NUM, (uint8_t*)stereoBuffer, samples * 4, &bytesWritten, portMAX_DELAY);
                Serial.printf("写入I2S: %d -> %d bytes\n", c, (int)bytesWritten);
            } else {
                delay(1);
            }
        }
        
        /* 恢复I2S配置 */
        i2s_set_clk(I2S_NUM, I2S_SAMPLE_RATE, I2S_BITS, I2S_CHANNEL_STEREO);
        
    } else {
        /* WAV格式：跳过44字节头部 */
        uint8_t header[44];
        
        /* 等待头部数据 */
        unsigned long startTime = millis();
        while (stream->available() < 44) {
            if (millis() - startTime > 5000) {
                Serial.println("等待WAV头超时");
                audio_state = AUDIO_STATE_ERROR;
                http.end();
                free(audioBuffer);
                return false;
            }
            delay(10);
        }
        
        /* 读取并跳过头部 */
        stream->readBytes(header, 44);
        
        /* 读取并播放音频数据 */
        while (http.connected()) {
            size_t size = stream->available();
            if (size > 0) {
                int c = stream->readBytes(audioBuffer, (size > 4096) ? 4096 : size);
                
                size_t bytesWritten;
                i2s_write(I2S_NUM, audioBuffer, c, &bytesWritten, portMAX_DELAY);
            } else {
                delay(1);
            }
        }
    }
    
    free(audioBuffer);
    http.end();
    
    audio_state = AUDIO_STATE_IDLE;
    Serial.println("音频播放完成");
    return true;
}

#ifdef USE_MP3_DECODER
/**
 * @brief  使用ESP8266Audio库播放MP3网络音频
 * @param  url: MP3音频URL
 * @retval true:成功 false:失败
 * @note   需要安装ESP8266Audio和libhelix-mp3库
 */
bool audio_play_mp3_url(const char* url)
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi未连接");
        return false;
    }
    
    Serial.printf("播放MP3音频: %s\n", url);
    
    audio_state = AUDIO_STATE_PLAYING;
    es8388_play_mode();
    
    /* 创建音频源 */
    file = new AudioFileSourceHTTPStream(url);
    buff = new AudioFileSourceBuffer(file, 4096);  /* 4KB缓冲区 */
    
    /* 创建I2S输出 - ES8388需要外部MCLK */
    out = new AudioOutputI2S();
    out->SetPinout(I2S_SCK_PIN, I2S_LRCK_PIN, I2S_SDOUT_PIN);
    out->SetOutputModeMono(true);  /* 单声道输出 */
    out->SetGain(((float)audio_volume / 100.0) * 2.0);  /* 设置音量 */
    
    /* 创建MP3解码器 */
    mp3 = new AudioGeneratorMP3();
    
    /* 开始播放 */
    if (!mp3->begin(buff, out)) {
        Serial.println("MP3解码器启动失败");
        audio_stop_mp3();
        return false;
    }
    
    Serial.println("开始播放MP3...");
    
    /* 播放循环 */
    while (mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            break;
        }
        delay(1);
    }
    
    audio_stop_mp3();
    Serial.println("MP3播放完成");
    return true;
}

/**
 * @brief  停止MP3播放并释放资源
 */
void audio_stop_mp3(void)
{
    if (mp3) {
        mp3->stop();
        delete mp3;
        mp3 = nullptr;
    }
    if (out) {
        delete out;
        out = nullptr;
    }
    if (buff) {
        delete buff;
        buff = nullptr;
    }
    if (file) {
        delete file;
        file = nullptr;
    }
    
    audio_state = AUDIO_STATE_IDLE;
}
#endif /* USE_MP3_DECODER */

/**
 * @brief  播放网络音频（自动识别格式）
 * @param  url: 音频URL
 * @note   支持WAV格式，如需MP3请定义USE_MP3_DECODER
 */
bool audio_play_network(const char* url)
{
    /* 检查URL后缀判断格式 */
    String urlStr = String(url);
    
#ifdef USE_MP3_DECODER
    if (urlStr.endsWith(".mp3") || urlStr.indexOf("mp3") > 0) {
        return audio_play_mp3_url(url);
    }
#endif
    
    /* 默认使用WAV播放器 */
    return audio_play_wav_url(url);
}
