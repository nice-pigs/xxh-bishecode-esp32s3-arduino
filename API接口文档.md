# 智能景区讲解系统 - API接口文档

## 基础信息

**服务地址**: `https://5f587623-7cdf-4670-91d9-45ff048194b3.dev.coze.site`

**数据格式**: JSON

---

## 接口列表

### 1. 景点识别接口 (POST /api/recognize)

一站式接口：图片识别 + TTS语音合成 + 保存记录

**请求URL**: `POST /api/recognize`

**请求参数**:
```json
{
  "image": "data:image/jpeg;base64,/9j/4AAQSkZJRg...",
  "latitude": 39.9042,
  "longitude": 116.4074,
  "deviceId": "ESP32_001",
  "speaker": "zh_female_xiaohe_uranus_bigtts",
  "speechRate": 0
}
```

**参数说明**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `image` | string | 是 | Base64编码的图片Data URL |
| `latitude` | number | 否 | 纬度 |
| `longitude` | number | 否 | 经度 |
| `deviceId` | string | 否 | 设备ID |
| `speaker` | string | 否 | 语音角色（默认：zh_female_xiaohe_uranus_bigtts） |
| `speechRate` | number | 否 | 语速（-10~10，默认0） |

**响应示例**:
```json
{
  "success": true,
  "data": {
    "id": "spot_1774793851096_7omre10",
    "spotName": "九寨沟",
    "description": "九寨沟在四川阿坝，因九个藏族村寨得名...",
    "audioUrl": "https://coze-coding-project.tos.coze.site/xxx.pcm",
    "audioUrlMp3": "https://coze-coding-project.tos.coze.site/xxx.mp3",
    "audioSize": 583378,
    "timestamp": "2026-03-29T14:17:31.096Z"
  }
}
```

---

### 2. 位置上报接口 (POST /api/location)

上报设备GPS位置和传感器数据

**请求URL**: `POST /api/location`

**请求参数**:
```json
{
  "deviceId": "ESP32_001",
  "latitude": 39.9042,
  "longitude": 116.4074,
  "altitude": 50.5,
  "satellites": 8,
  "speed": 1.2,
  "batteryLevel": 85,
  "temperature": 25,
  "humidity": 60,
  "lightIntensity": 150,
  "irIntensity": 5,
  "proximity": 230
}
```

**参数说明**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `deviceId` | string | 是 | 设备ID |
| `latitude` | number | 是 | 纬度 |
| `longitude` | number | 是 | 经度 |
| `altitude` | number | 否 | 海拔（米） |
| `satellites` | number | 否 | 卫星数 |
| `speed` | number | 否 | 速度（m/s） |
| `batteryLevel` | number | 否 | 电量（0-100） |
| `temperature` | number | 否 | 温度（DHT11） |
| `humidity` | number | 否 | 湿度（DHT11） |
| `lightIntensity` | number | 否 | 光照强度（AP3216C ALS） |
| `irIntensity` | number | 否 | 红外强度（AP3216C IR） |
| `proximity` | number | 否 | 接近度（AP3216C PS） |

**响应示例**:
```json
{
  "success": true,
  "data": {
    "deviceId": "ESP32_001",
    "lastUpdate": "2026-03-29T15:32:31.401Z"
  }
}
```

---

### 3. 获取设备状态 (GET /api/location)

获取当前设备状态

**请求URL**: `GET /api/location`

**响应示例**:
```json
{
  "success": true,
  "data": {
    "deviceId": "ESP32_001",
    "latitude": 39.9042,
    "longitude": 116.4074,
    "altitude": 50.5,
    "satellites": 8,
    "speed": 1.2,
    "batteryLevel": 85,
    "temperature": 25,
    "humidity": 60,
    "lightIntensity": 150,
    "irIntensity": 5,
    "proximity": 230,
    "lastUpdate": "2026-03-29T15:32:31.401Z"
  }
}
```

---

### 4. 获取识别历史 (GET /api/history)

获取景点识别历史记录

**请求URL**: `GET /api/history?limit=20`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `limit` | number | 否 | 返回记录数（默认50） |
| `id` | string | 否 | 指定记录ID，获取单条记录 |

**响应示例**:
```json
{
  "success": true,
  "data": [
    {
      "id": "spot_1774793851096_7omre10",
      "imageUrl": "data:image/jpeg;base64,/9j/4AAQSkZJRg...",
      "spotName": "九寨沟",
      "description": "九寨沟在四川阿坝...",
      "latitude": 39.9042,
      "longitude": 116.4074,
      "audioUrl": "https://coze-coding-project.tos.coze.site/xxx.pcm",
      "audioUrlMp3": "https://coze-coding-project.tos.coze.site/xxx.mp3",
      "timestamp": "2026-03-29T14:17:31.096Z"
    }
  ],
  "total": 1
}
```

---

### 5. 音频中转接口 (GET /api/audio)

通过代理下载音频（用于ESP32避免HTTPS问题）

**请求URL**: `GET /api/audio?url=<音频URL>`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `url` | string | 是 | 音频URL |

**响应**: 音频二进制数据

---

## ESP32调用示例

### 识别接口调用示例

```cpp
HTTPClient http;
http.begin("http://10.150.235.40:8080/api/recognize");
http.addHeader("Content-Type", "application/json");

String requestBody = "{\"image\":\"data:image/jpeg;base64," + base64Img + "\"}";
int httpCode = http.POST(requestBody);

if (httpCode == HTTP_CODE_OK) {
  String response = http.getString();
  // 解析响应...
}
```

### 位置上报示例

```cpp
HTTPClient http;
http.begin("http://10.150.235.40:8080/api/location");
http.addHeader("Content-Type", "application/json");

String json = "{";
json += "\"deviceId\":\"ESP32_001\",";
json += "\"latitude\":0,";
json += "\"longitude\":0,";
json += "\"temperature\":25,";
json += "\"humidity\":60,";
json += "\"lightIntensity\":150";
json += "}";

int httpCode = http.POST(json);
```

---

## 错误码说明

| HTTP状态码 | 说明 |
|-----------|------|
| 200 | 成功 |
| 400 | 请求参数错误 |
| 404 | 资源不存在 |
| 500 | 服务器内部错误 |

---

## 注意事项

1. **图片格式**: 必须是Base64编码的Data URL，以 `data:image/jpeg;base64,` 开头
2. **音频格式**: 提供PCM（ESP32用）和MP3（网页用）两种格式
3. **代理使用**: ESP32通过HTTP代理访问，避免校园网封锁443端口
4. **传感器数据**: 所有传感器字段都是可选的
