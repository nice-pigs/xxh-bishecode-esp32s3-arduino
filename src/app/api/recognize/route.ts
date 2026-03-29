import { NextRequest, NextResponse } from "next/server";
import { LLMClient, TTSClient, Config, HeaderUtils } from "coze-coding-dev-sdk";
import { addSpotRecord, getDeviceStatus } from "@/lib/store";

// 一站式接口：图片识别 + TTS语音合成
// 适合ESP32等嵌入式设备调用，一次请求完成所有操作
// 支持格式：JPEG Base64

const SYSTEM_PROMPT = `你是一个专业的景区景点识别助手。当用户上传景点照片时，你需要：

1. 识别照片中的景点名称（如果能够识别）
2. 提供该景点的详细介绍，包括：
   - 景点名称
   - 历史背景
   - 主要特色
   - 游览建议

请用生动有趣的语言介绍，适合语音播报，总字数控制在150-200字以内。

回复格式：
【景点名称】xxx
【景点介绍】xxx`;

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { 
      image, 
      latitude, 
      longitude,
      speaker = "zh_female_xiaohe_uranus_bigtts",
      speechRate = 0,
    } = body;

    if (!image) {
      return NextResponse.json(
        { error: "缺少图片数据" },
        { status: 400 }
      );
    }

    console.log('[识别接口] 收到请求，图片大小:', image.length);

    // 检查图片格式
    if (!image.startsWith('data:image/')) {
      console.error('[识别接口] 图片格式错误，缺少data:image前缀');
      return NextResponse.json(
        { error: "图片格式错误，需要Base64编码的Data URL" },
        { status: 400 }
      );
    }

    const customHeaders = HeaderUtils.extractForwardHeaders(request.headers);
    const config = new Config();

    // Step 1: 调用视觉模型识别景点
    console.log('[识别接口] 调用大模型...');
    const llmClient = new LLMClient(config, customHeaders);
    
    const messages = [
      { role: "system" as const, content: SYSTEM_PROMPT },
      {
        role: "user" as const,
        content: [
          { type: "text" as const, text: "请识别这张景点照片并提供介绍。" },
          {
            type: "image_url" as const,
            image_url: { url: image, detail: "high" as const },
          },
        ],
      },
    ];

    const llmResponse = await llmClient.invoke(messages, {
      model: "doubao-seed-1-6-vision-250815",
      temperature: 0.7,
    });

    const content = llmResponse.content;
    console.log('[识别接口] 大模型响应长度:', content.length);

    // 解析景点信息
    let spotName = "未知景点";
    let description = content;

    const nameMatch = content.match(/【景点名称】(.+?)(?:\n|【|$)/);
    const descMatch = content.match(/【景点介绍】([\s\S]+)$/);

    if (nameMatch) spotName = nameMatch[1].trim();
    if (descMatch) description = descMatch[1].trim();

    // Step 2: 调用TTS生成语音（两种格式）
    const ttsClient = new TTSClient(config, customHeaders);
    
    // 格式1: PCM格式（给ESP32设备播放）
    const ttsResponsePcm = await ttsClient.synthesize({
      uid: `device_${Date.now()}_pcm`,
      text: description,
      speaker,
      audioFormat: "pcm",  // PCM格式，ESP32无需解码器即可播放
      sampleRate: 16000,   // 16kHz采样率，适合语音
      speechRate,
    });
    
    // 格式2: MP3格式（给网页浏览器播放）
    const ttsResponseMp3 = await ttsClient.synthesize({
      uid: `device_${Date.now()}_mp3`,
      text: description,
      speaker,
      audioFormat: "mp3",  // MP3格式，浏览器可直接播放
      sampleRate: 16000,   // 16kHz采样率
      speechRate,
    });

    // Step 3: 保存记录（保存完整图片数据）
    const deviceStatus = await getDeviceStatus();
    const record = await addSpotRecord({
      imageUrl: image,  // 保存完整的base64图片数据
      spotName,
      description,
      latitude: latitude || deviceStatus?.latitude || 0,
      longitude: longitude || deviceStatus?.longitude || 0,
      audioUrl: ttsResponsePcm.audioUri,  // PCM格式（ESP32用）
      audioUrlMp3: ttsResponseMp3.audioUri, // MP3格式（网页用）
    });

    return NextResponse.json({
      success: true,
      data: {
        id: record.id,
        spotName,
        description,
        audioUrl: ttsResponsePcm.audioUri,  // PCM格式（ESP32用）
        audioUrlMp3: ttsResponseMp3.audioUri, // MP3格式（网页用）
        audioSize: ttsResponsePcm.audioSize,
        timestamp: record.timestamp,
      },
    });
  } catch (error) {
    console.error("[识别接口] 错误:", error);
    // 返回更详细的错误信息（开发环境）
    const errorMessage = error instanceof Error ? error.message : "识别失败，请稍后重试";
    return NextResponse.json(
      { error: errorMessage, details: String(error) },
      { status: 500 }
    );
  }
}
