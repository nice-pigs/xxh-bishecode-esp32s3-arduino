/**
 ******************************************************************************
 * @file     esp32_smart_guide.ino
 * @brief    智能景区讲解系统 - 基于正点原子官方代码
 * 
 * 功能：
 * 1. KEY0：进入/退出预览模式
 * 2. KEY1：拍照识别
 * 3. 预览用RGB565，拍照用JPEG（deinit/reinit切换）
 ******************************************************************************
 */

#include "xl9555.h"
#include "camera.h"
#include "spilcd.h"
#include "led.h"
#include "uart.h"
#include "audio.h"
#include "dht11.h"
#include "ap3216c.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <driver/i2c.h>

// 为了兼容audio.cpp，必须在全局作用域声明！
WiFiClientSecure httpsClient;

// 初始化httpsClient（跳过证书验证）
void init_https_client() {
    httpsClient.setInsecure();
}

/* ============== 配置区域 ============== */

const char* WIFI_SSID = "cs";
const char* WIFI_PASSWORD = "88888888";
const char* PROXY_URL = "http://10.150.235.40:8080";
const char* DEVICE_ID = "ESP32_001";

/* ============== GPS配置 ============== */
#define GPS_RX_PIN 3
#define GPS_TX_PIN -1
double gps_lat = 0;
double gps_lng = 0;
bool gps_valid = false;
bool gps_initialized = false;
TinyGPSPlus gps;
#define LOCATION_REPORT_INTERVAL 30000
unsigned long lastLocationReport = 0;

/* ============== 全局变量 ============== */
bool wifi_connected = false;
bool isCapturing = false;

/* ============== 传感器配置 ============== */
bool dht11_ok = false;
bool ap3216c_ok = false;
uint8_t dht11_temp = 0;
uint8_t dht11_humi = 0;
uint16_t ap3216c_ir = 0;
uint16_t ap3216c_ps = 0;
uint16_t ap3216c_als = 0;
unsigned long last_sensor_read = 0;
#define SENSOR_READ_INTERVAL 10000  /* 改为10秒，减少输出 */

/* ============== 模式控制变量 ============== */
bool preview_mode = false;
bool need_redraw_main = true;

// 状态机
typedef enum {
    STATE_IDLE,      // 待机
    STATE_PREVIEW,   // 预览模式
    STATE_CAPTURING, // 识别中
} app_state_t;

static app_state_t app_state = STATE_IDLE;

// 按键状态
static int lastKey0 = 1, lastKey1 = 1;
static unsigned long lastKey0Time = 0, lastKey1Time = 0;
#define BUTTON_DEBOUNCE_MS 300

/* ============== 前向声明 ============== */
void wifi_connect();
void capture_and_upload();
void read_sensors();
void update_sensor_display();

/* ============== 传感器读取函数 ============== */

void read_sensors()
{
    if (millis() - last_sensor_read < SENSOR_READ_INTERVAL) {
        return;
    }
    last_sensor_read = millis();
    
    // 读取DHT11
    if (dht11_ok) {
        if (dht11_read_data(&dht11_temp, &dht11_humi) == 0) {
            Serial.printf("DHT11: Temp=%d°C, Humi=%d%%\n", dht11_temp, dht11_humi);
        } else {
            Serial.println("DHT11 read failed!");
        }
    }
    
    // 读取AP3216C
    if (ap3216c_ok) {
        ap3216c_read_data(&ap3216c_ir, &ap3216c_ps, &ap3216c_als);
        Serial.printf("AP3216C: IR=%d, PS=%d, ALS=%d\n", ap3216c_ir, ap3216c_ps, ap3216c_als);
    }
    
    // 更新显示
    if (app_state == STATE_IDLE) {
        update_sensor_display();
    }
}

void update_sensor_display()
{
    char buf[64];
    
    // 清除传感器数据区域
    lcd_fill(30, 210, 300, 320, BLACK);
    
    int y = 210;
    
    // 显示DHT11数据
    if (dht11_ok) {
        snprintf(buf, sizeof(buf), "Temp: %d C", dht11_temp);
        lcd_show_string(30, y, 200, 16, LCD_FONT_16, buf, YELLOW);
        y += 20;
        snprintf(buf, sizeof(buf), "Humi: %d %%", dht11_humi);
        lcd_show_string(30, y, 200, 16, LCD_FONT_16, buf, YELLOW);
        y += 20;
    }
    
    // 显示AP3216C数据
    if (ap3216c_ok) {
        snprintf(buf, sizeof(buf), "ALS: %d", ap3216c_als);
        lcd_show_string(30, y, 200, 16, LCD_FONT_16, buf, CYAN);
        y += 20;
        snprintf(buf, sizeof(buf), "IR: %d  PS: %d", ap3216c_ir, ap3216c_ps);
        lcd_show_string(30, y, 200, 16, LCD_FONT_16, buf, CYAN);
    }
}

/* ============== 初始化 ============== */

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("  智能景区讲解系统 DNESP32-S3");
    Serial.println("  正点原子官方代码版");
    Serial.println("========================================\n");
    
    led_init();
    LED(1);
    
    Serial.println("步骤1: 初始化XL9555...");
    xl9555_init();
    
    Serial.println("步骤2: 初始化LCD...");
    lcd_init();
    lcd_clear(WHITE);
    xl9555_pin_set(SLCD_PWR, IO_SET_HIGH);
    
    lcd_show_string(30, 50, 200, 16, LCD_FONT_16, "ESP32-S3", RED);
    lcd_show_string(30, 70, 200, 16, LCD_FONT_16, "Smart Guide", RED);
    lcd_show_string(30, 90, 200, 16, LCD_FONT_16, "ATOM@ALIENTEK", RED);
    
    Serial.println("步骤3: 初始化摄像头...");
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Init camera...", RED);
    
    // 用RGB565格式初始化（用于预览）
    Serial.println("用RGB565格式初始化摄像头...");
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.xclk_freq_hz = 24000000;
    config.pin_d7 = OV_D7_PIN;
    config.pin_d6 = OV_D6_PIN;
    config.pin_d5 = OV_D5_PIN;
    config.pin_d4 = OV_D4_PIN;
    config.pin_d3 = OV_D3_PIN;
    config.pin_d2 = OV_D2_PIN;
    config.pin_d1 = OV_D1_PIN;
    config.pin_d0 = OV_D0_PIN;
    config.pin_xclk = OV_XCLK_PIN;
    config.pin_pclk = OV_PCLK_PIN;
    config.pin_vsync = OV_VSYNC_PIN;
    config.pin_href = OV_HREF_PIN;
    config.pin_sscb_sda = OV_SDA_PIN;
    config.pin_sscb_scl = OV_SCL_PIN;
    config.pin_pwdn = OV_PWDN_PIN;
    config.pin_reset = OV_RESET_PIN;
    config.frame_size = FRAMESIZE_QVGA;  // 320x240用于预览
    config.pixel_format = PIXFORMAT_RGB565; // RGB565格式用于预览
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // 摄像头上电（如果需要）
    if (OV_PWDN_PIN == -1) {
        xl9555_io_config(OV_PWDN, IO_SET_OUTPUT);
        xl9555_pin_set(OV_PWDN, IO_SET_LOW);
    }
    if (OV_RESET_PIN == -1) {
        xl9555_io_config(OV_RESET, IO_SET_OUTPUT);
        xl9555_pin_set(OV_RESET, IO_SET_LOW);
        delay(20);
        xl9555_pin_set(OV_RESET, IO_SET_HIGH);
        delay(20);
    }
    
    int retry = 0;
    esp_err_t err = ESP_FAIL;
    while (err != ESP_OK && retry < 3) {
        err = esp_camera_init(&config);
        if (err != ESP_OK) {
            retry++;
            Serial.printf("摄像头初始化失败，重试 %d/3, 错误: 0x%x\n", retry, err);
            delay(500);
        }
    }
    
    if (err != ESP_OK) {
        lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Camera FAILED!", RED);
        Serial.println("摄像头初始化失败！");
        while(1) delay(1000);
    }
    
    sensor_t *s = esp_camera_sensor_get();
    Serial.printf("摄像头ID: 0x%x\n", s->id.PID);
    lcd_show_string(30, 110, 200, 16, LCD_FONT_16, "Camera: OK", GREEN);
    
    Serial.println("步骤4: 连接WiFi...");
    wifi_connect();
    init_https_client();
    
    Serial.println("步骤5: 初始化GPS...");
    gps_initialized = true;
    
    Serial.println("步骤6: 初始化音频...");
    audio_init();
    
    Serial.println("步骤7: 初始化DHT11...");
    lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "Init DHT11...", RED);
    if (dht11_init() == 0) {
        dht11_ok = true;
        Serial.println("DHT11 初始化成功！");
        lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "DHT11: OK", GREEN);
    } else {
        dht11_ok = false;
        Serial.println("DHT11 初始化失败！");
        lcd_show_string(30, 130, 200, 16, LCD_FONT_16, "DHT11: FAIL", RED);
    }
    
    Serial.println("步骤8: 初始化AP3216C...");
    lcd_show_string(30, 150, 200, 16, LCD_FONT_16, "Init AP3216C...", RED);
    if (ap3216c_init() == 0) {
        ap3216c_ok = true;
        Serial.println("AP3216C 初始化成功！");
        lcd_show_string(30, 150, 200, 16, LCD_FONT_16, "AP3216C: OK", GREEN);
    } else {
        ap3216c_ok = false;
        Serial.println("AP3216C 初始化失败！");
        lcd_show_string(30, 150, 200, 16, LCD_FONT_16, "AP3216C: FAIL", RED);
    }
    
    // 主界面
    lcd_clear(BLACK);
    xl9555_pin_set(SLCD_PWR, IO_SET_HIGH);
    lcd_show_string(30, 20, 200, 24, LCD_FONT_24, "Smart Guide", RED);
    lcd_show_string(30, 55, 200, 16, LCD_FONT_16, "KEY1: Capture", BLUE);
    
    if (wifi_connected) {
        lcd_show_string(30, 100, 200, 16, LCD_FONT_16, "WiFi: OK", GREEN);
        lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Proxy: Ready", GREEN);
    }
    
    // 显示传感器状态
    lcd_show_string(30, 145, 200, 16, LCD_FONT_16, "Sensors:", WHITE);
    if (dht11_ok) {
        lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: OK", GREEN);
    } else {
        lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: FAIL", RED);
    }
    if (ap3216c_ok) {
        lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: OK", GREEN);
    } else {
        lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: FAIL", RED);
    }
    
    // 配置按键为输入
    xl9555_io_config(KEY0, IO_SET_INPUT);
    xl9555_io_config(KEY1, IO_SET_INPUT);
    delay(100);
    
    LED(0);
    delay(500);
    
    Serial.println("\n=== 系统就绪 ===");
    int k0 = xl9555_get_pin(KEY0);
    int k1 = xl9555_get_pin(KEY1);
    Serial.printf("按键状态 - KEY0:%d KEY1:%d\n", k0, k1);
    
    Serial.println("播放测试提示音...");
    delay(200);
    // 确保ES8388在播放模式，音量最大
    audio_set_volume(100);
    audio_beep(500);  // 延长到500ms
    delay(200);
    audio_beep(500);
    Serial.println("测试提示音播放完成");
}

/* ============== WiFi连接 ============== */

void wifi_connect()
{
    Serial.printf("连接WiFi: %s\n", WIFI_SSID);
    lcd_show_string(80, 200, 200, 16, LCD_FONT_16, "Connecting...", BLUE);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        Serial.println("\nWiFi连接成功！");
        Serial.printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
        lcd_show_string(80, 200, 200, 16, LCD_FONT_16, "Connected!", GREEN);
    } else {
        wifi_connected = false;
        Serial.println("\nWiFi连接失败！");
        lcd_show_string(80, 200, 200, 16, LCD_FONT_16, "Failed!", RED);
    }
}

/* ============== 主循环 ============== */

void loop()
{
    // 读取传感器
    read_sensors();
    
    // 读取按键
    int key0 = xl9555_get_pin(KEY0);
    int key1 = xl9555_get_pin(KEY1);
    
    bool key0_pressed = (key0 == 0 && lastKey0 == 1 && millis() - lastKey0Time > BUTTON_DEBOUNCE_MS);
    bool key1_pressed = (key1 == 0 && lastKey1 == 1 && millis() - lastKey1Time > BUTTON_DEBOUNCE_MS);
    
    if (key0_pressed) lastKey0Time = millis();
    if (key1_pressed) lastKey1Time = millis();
    lastKey0 = key0;
    lastKey1 = key1;
    
    // 完整状态机
    switch (app_state) {
        
        case STATE_IDLE:
            // 重绘主界面
            if (need_redraw_main) {
                lcd_clear(BLACK);
                xl9555_pin_set(SLCD_PWR, IO_SET_HIGH);
                lcd_show_string(30, 20, 200, 24, LCD_FONT_24, "Smart Guide", RED);
                lcd_show_string(30, 55, 200, 16, LCD_FONT_16, "KEY0: Preview", BLUE);
                lcd_show_string(30, 75, 200, 16, LCD_FONT_16, "KEY1: Capture", BLUE);
                
                if (wifi_connected) {
                    lcd_show_string(30, 100, 200, 16, LCD_FONT_16, "WiFi: OK", GREEN);
                    lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Proxy: Ready", GREEN);
                }
                
                // 显示传感器状态
                lcd_show_string(30, 145, 200, 16, LCD_FONT_16, "Sensors:", WHITE);
                if (dht11_ok) {
                    lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: OK", GREEN);
                } else {
                    lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: FAIL", RED);
                }
                if (ap3216c_ok) {
                    lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: OK", GREEN);
                } else {
                    lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: FAIL", RED);
                }
                
                // 立即更新传感器数据显示
                update_sensor_display();
                
                need_redraw_main = false;
            }
            
            if (key0_pressed) {
                Serial.println(">>> KEY0进入预览");
                app_state = STATE_PREVIEW;
                need_redraw_main = true;
                lcd_clear(BLACK);
            } else if (key1_pressed && wifi_connected) {
                Serial.println(">>> KEY1拍照识别");
                app_state = STATE_CAPTURING;
                capture_and_upload();
                app_state = STATE_IDLE;
                need_redraw_main = true;
            }
            break;
            
        case STATE_PREVIEW:
            // 预览模式
            {
                camera_fb_t *fb = esp_camera_fb_get();
                if (fb) {
                    // RGB565直接显示
                    lcd_show_pic(0, 0, fb->width, fb->height, (uint8_t *)fb->buf);
                    esp_camera_fb_return(fb);
                }
            }
            
            if (key0_pressed) {
                Serial.println(">>> KEY0退出预览");
                app_state = STATE_IDLE;
                need_redraw_main = true;
            } else if (key1_pressed && wifi_connected) {
                Serial.println(">>> KEY1拍照识别");
                app_state = STATE_CAPTURING;
                capture_and_upload();
                app_state = STATE_IDLE;
                need_redraw_main = true;
            }
            break;
            
        case STATE_CAPTURING:
            // 等待识别完成
            delay(10);
            break;
    }
}

/* ============== 前向声明 ============== */
void send_to_server(String base64Img);
String base64_encode(uint8_t *data, size_t len);

/* ============== 拍照识别 ============== */

void capture_and_upload()
{
    Serial.println("[拍照] 开始...");
    lcd_clear(BLACK);
    lcd_show_string(30, 100, 200, 16, LCD_FONT_16, "Capturing...", YELLOW);
    
    bool jpeg_ok = false;
    String base64Img = "";
    
    // ---------------- 切换到JPEG格式 ----------------
    Serial.println("[拍照] 切换到JPEG格式...");
    esp_camera_deinit();
    delay(100);
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.xclk_freq_hz = 24000000;
    config.pin_d7 = OV_D7_PIN;
    config.pin_d6 = OV_D6_PIN;
    config.pin_d5 = OV_D5_PIN;
    config.pin_d4 = OV_D4_PIN;
    config.pin_d3 = OV_D3_PIN;
    config.pin_d2 = OV_D2_PIN;
    config.pin_d1 = OV_D1_PIN;
    config.pin_d0 = OV_D0_PIN;
    config.pin_xclk = OV_XCLK_PIN;
    config.pin_pclk = OV_PCLK_PIN;
    config.pin_vsync = OV_VSYNC_PIN;
    config.pin_href = OV_HREF_PIN;
    config.pin_sscb_sda = OV_SDA_PIN;
    config.pin_sscb_scl = OV_SCL_PIN;
    config.pin_pwdn = OV_PWDN_PIN;
    config.pin_reset = OV_RESET_PIN;
    config.frame_size = FRAMESIZE_SVGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 10;
    config.fb_count = 1;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println("[拍照] JPEG初始化失败！");
        lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Camera Fail!", RED);
        delay(2000);
    } else {
        Serial.println("[拍照] JPEG初始化成功！");
        delay(200);
        
        // 抓取JPEG帧
        Serial.println("[拍照] 抓取JPEG帧...");
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[拍照] 抓帧失败！");
            lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Capture Fail!", RED);
            delay(2000);
        } else {
            Serial.printf("[拍照] 抓到JPEG: %zu bytes\n", fb->len);
            lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Uploading...", BLUE);
            
            // Base64编码并上传到服务器
            Serial.println("[拍照] 编码并上传...");
            base64Img = base64_encode(fb->buf, fb->len);
            esp_camera_fb_return(fb);
            jpeg_ok = true;
        }
    }
    
    // ---------------- 恢复RGB565格式 ----------------
    Serial.println("[拍照] 恢复RGB565格式...");
    esp_camera_deinit();
    delay(100);
    
    config.frame_size = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_RGB565;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println("[拍照] RGB565恢复失败！");
    } else {
        Serial.println("[拍照] RGB565恢复成功！");
    }
    delay(100);
    
    // 如果JPEG抓取成功，则上传
    if (jpeg_ok && base64Img.length() > 0) {
        send_to_server(base64Img);
        Serial.println("[拍照] 完成！");
        delay(1000);
    }
    
    // 恢复主界面和传感器显示
    lcd_clear(BLACK);
    xl9555_pin_set(SLCD_PWR, IO_SET_HIGH);
    lcd_show_string(30, 20, 200, 24, LCD_FONT_24, "Smart Guide", RED);
    lcd_show_string(30, 55, 200, 16, LCD_FONT_16, "KEY0: Preview", BLUE);
    lcd_show_string(30, 75, 200, 16, LCD_FONT_16, "KEY1: Capture", BLUE);
    
    if (wifi_connected) {
        lcd_show_string(30, 100, 200, 16, LCD_FONT_16, "WiFi: OK", GREEN);
        lcd_show_string(30, 120, 200, 16, LCD_FONT_16, "Proxy: Ready", GREEN);
    }
    
    // 显示传感器状态
    lcd_show_string(30, 145, 200, 16, LCD_FONT_16, "Sensors:", WHITE);
    if (dht11_ok) {
        lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: OK", GREEN);
    } else {
        lcd_show_string(30, 165, 200, 16, LCD_FONT_16, "DHT11: FAIL", RED);
    }
    if (ap3216c_ok) {
        lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: OK", GREEN);
    } else {
        lcd_show_string(30, 185, 200, 16, LCD_FONT_16, "AP3216C: FAIL", RED);
    }
    
    // 立即更新传感器数据显示
    update_sensor_display();
}


/* ============== 发送到代理服务器（HTTP版本） ============== */

void send_to_server(String base64Img)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi未连接！");
        lcd_show_string(30, 160, 200, 16, LCD_FONT_16, "WiFi Error!", RED);
        return;
    }
    
    Serial.printf("发送前 - 空闲堆: %d, 空闲PSRAM: %d\n", 
        ESP.getFreeHeap(), ESP.getFreePsram());
    
    String url = String(PROXY_URL) + "/api/recognize";
    Serial.println("URL: " + url);
    Serial.println("正在连接代理服务器（HTTP）...");
    
    // 使用WiFiClient（不是WiFiClientSecure！）
    WiFiClient testClient;
    testClient.setTimeout(25000);
    
    HTTPClient testHttp;
    testHttp.begin(testClient, url);
    testHttp.addHeader("Content-Type", "application/json");
    testHttp.setTimeout(25000);
    
    String requestBody = "{\"image\":\"data:image/jpeg;base64," + base64Img + "\",\"deviceId\":\"" + String(DEVICE_ID) + "\"}";
    
    Serial.printf("发送请求，数据大小: %d bytes...\n", requestBody.length());
    lcd_show_string(30, 160, 200, 16, LCD_FONT_16, "Recognizing...", BLUE);
    
    int testCode = testHttp.POST(requestBody);
    
    Serial.printf("HTTP响应码: %d\n", testCode);
    
    if (testCode < 0) {
        Serial.print("错误详情: ");
        switch(testCode) {
            case -1: Serial.println("连接被拒绝"); break;
            case -2: Serial.println("发送头失败"); break;
            case -3: Serial.println("发送payload失败"); break;
            case -4: Serial.println("没有响应或响应超时"); break;
            case -5: Serial.println("连接丢失"); break;
            case -6: Serial.println("没有足够的内存"); break;
            case -7: Serial.println("连接已存在"); break;
            case -11: Serial.println("读取超时"); break;
            default: Serial.printf("未知错误: %d\n", testCode); break;
        }
        
        lcd_clear(WHITE);
        lcd_show_string(10, 100, 220, 16, LCD_FONT_16, "HTTP Error!", RED);
        String errMsg = "Code: " + String(testCode);
        lcd_show_string(10, 120, 220, 16, LCD_FONT_16, errMsg.c_str(), BLUE);
        delay(3000);
        
        testHttp.end();
        testClient.stop();
        return;
    }
    
    Serial.printf("HTTP状态码: %d\n", testCode);
    
    String response = testHttp.getString();
    Serial.println("响应内容: " + response);
    
    if (testCode == HTTP_CODE_OK && response.length() > 0)
    {
        DynamicJsonDocument respDoc(8192);
        DeserializationError error = deserializeJson(respDoc, response);
        
        if (error) {
            Serial.print("JSON解析失败: ");
            Serial.println(error.c_str());
            lcd_show_string(30, 180, 200, 16, LCD_FONT_16, "Parse Error", RED);
        }
        else if (respDoc["success"] == true)
        {
            String spotName = respDoc["data"]["spotName"];
            String description = respDoc["data"]["description"];
            String audioUrl = respDoc["data"]["audioUrl"];
            
            Serial.println("景点: " + spotName);
            Serial.println("介绍: " + description);
            Serial.println("语音: " + audioUrl);
            
            lcd_clear(WHITE);
            lcd_show_string(10, 10, 220, 16, LCD_FONT_16, "Spot:", RED);
            
            String displayName = spotName.length() > 14 ? spotName.substring(0, 14) : spotName;
            lcd_show_string(10, 30, 220, 24, LCD_FONT_24, displayName.c_str(), BLUE);
                
            lcd_show_string(10, 70, 220, 16, LCD_FONT_16, "Info:", RED);
            String shortDesc = description.length() > 60 ? description.substring(0, 60) : description;
            lcd_show_string(10, 90, 220, 16, LCD_FONT_16, shortDesc.c_str(), BLACK);
                
            lcd_show_string(10, 200, 220, 16, LCD_FONT_16, "Audio ready!", GREEN);
                
            if (audioUrl.length() > 0) {
                Serial.println("开始播放语音讲解...");
                lcd_show_string(10, 220, 220, 16, LCD_FONT_16, "Playing audio...", BLUE);
                audio_play_network(audioUrl.c_str());
                lcd_show_string(10, 220, 220, 16, LCD_FONT_16, "Audio done!", GREEN);
            }
        }
        else
        {
            String errMsg = respDoc["error"] | "Recognition Failed";
            Serial.println("服务器错误: " + errMsg);
            lcd_show_string(30, 180, 200, 16, LCD_FONT_16, errMsg.c_str(), RED);
            delay(3000);
        }
    }
    else
    {
        String errMsg = "HTTP " + String(testCode) + " or Empty";
        Serial.println("HTTP错误或响应为空: " + errMsg);
        lcd_show_string(30, 180, 200, 16, LCD_FONT_16, errMsg.c_str(), RED);
        delay(2000);
    }
    
    testHttp.end();
    testClient.stop();
    Serial.println("请求完成，连接已关闭");
}

/* ============== Base64编码 ============== */

String base64_encode(uint8_t *data, size_t len)
{
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String result;
    int i = 0;
    uint8_t arr3[3], arr4[4];
    
    while (len--)
    {
        arr3[i++] = *(data++);
        if (i == 3)
        {
            arr4[0] = (arr3[0] & 0xfc) >> 2;
            arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
            arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
            arr4[3] = arr3[2] & 0x3f;
            for (i = 0; i < 4; i++) result += chars[arr4[i]];
            i = 0;
        }
    }
    
    if (i)
    {
        for (int j = i; j < 3; j++) arr3[j] = '\0';
        arr4[0] = (arr3[0] & 0xfc) >> 2;
        arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
        arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
        for (int j = 0; j < i + 1; j++) result += chars[arr4[j]];
        while (i++ < 3) result += '=';
    }
    
    return result;
}
