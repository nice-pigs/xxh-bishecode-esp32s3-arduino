import { NextRequest, NextResponse } from "next/server";
import { LLMClient, Config, HeaderUtils } from "coze-coding-dev-sdk";
import { addSpotRecord, getDeviceStatus } from "@/lib/store";

// 景点识别的系统提示词
const SYSTEM_PROMPT = `你是一个专业的景区景点识别助手。当用户上传景点照片时，你需要：

1. 识别照片中的景点名称（如果能够识别）
2. 提供该景点的详细介绍，包括：
   - 景点名称
   - 历史背景
   - 主要特色
   - 游览建议
   - 有趣的小故事或冷知识

如果无法识别具体景点，请根据照片内容描述场景，并给出可能的位置信息。

请用生动有趣的语言介绍，适合语音播报，总字数控制在200-300字以内。

回复格式：
【景点名称】xxx
【景点介绍】xxx`;

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { image, latitude, longitude } = body;

    if (!image) {
      return NextResponse.json(
        { error: "缺少图片数据" },
        { status: 400 }
      );
    }

    // 提取请求头
    const customHeaders = HeaderUtils.extractForwardHeaders(request.headers);
    const config = new Config();
    const client = new LLMClient(config, customHeaders);

    // 构建消息 - 使用视觉模型
    const messages = [
      {
        role: "system" as const,
        content: SYSTEM_PROMPT,
      },
      {
        role: "user" as const,
        content: [
          {
            type: "text" as const,
            text: "请识别这张景点照片，并提供详细的景点介绍。如果照片中有文字信息，请一并参考。",
          },
          {
            type: "image_url" as const,
            image_url: {
              url: image, // 可以是URL或base64
              detail: "high" as const,
            },
          },
        ],
      },
    ];

    // 调用视觉模型识别
    const response = await client.invoke(messages, {
      model: "doubao-seed-1-6-vision-250815",
      temperature: 0.7,
    });

    const content = response.content;

    // 解析景点名称和介绍
    let spotName = "未知景点";
    let description = content;

    const nameMatch = content.match(/【景点名称】(.+?)(?:\n|【|$)/);
    const descMatch = content.match(/【景点介绍】([\s\S]+)$/);

    if (nameMatch) {
      spotName = nameMatch[1].trim();
    }
    if (descMatch) {
      description = descMatch[1].trim();
    }

    // 获取当前设备位置（如果有的话）
    const deviceStatus = await getDeviceStatus();
    const recordLat = latitude || deviceStatus?.latitude || 0;
    const recordLng = longitude || deviceStatus?.longitude || 0;

    // 保存识别记录
    const record = await addSpotRecord({
      imageUrl: image.startsWith("http") ? image : `data:image/jpeg;base64,${image.substring(0, 100)}...`,
      spotName,
      description,
      latitude: recordLat,
      longitude: recordLng,
    });

    return NextResponse.json({
      success: true,
      data: {
        id: record.id,
        spotName,
        description,
        timestamp: record.timestamp,
      },
    });
  } catch (error) {
    console.error("识别失败:", error);
    return NextResponse.json(
      { error: "识别失败，请稍后重试" },
      { status: 500 }
    );
  }
}
