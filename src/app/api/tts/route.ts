import { NextRequest, NextResponse } from "next/server";
import { TTSClient, Config, HeaderUtils } from "coze-coding-dev-sdk";

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { text, speaker, speechRate, loudnessRate } = body;

    if (!text) {
      return NextResponse.json(
        { error: "缺少要转换的文字" },
        { status: 400 }
      );
    }

    // 提取请求头
    const customHeaders = HeaderUtils.extractForwardHeaders(request.headers);
    const config = new Config();
    const client = new TTSClient(config, customHeaders);

    // 调用TTS服务
    // 使用PCM格式，适合ESP32等嵌入式设备直接播放
    const response = await client.synthesize({
      uid: `device_${Date.now()}`,
      text,
      speaker: speaker || "zh_female_xiaohe_uranus_bigtts", // 默认女声
      audioFormat: "pcm",  // PCM格式，ESP32可直接播放
      sampleRate: 16000,   // 16kHz采样率
      speechRate: speechRate || 0, // 语速调整 -50 ~ 100
      loudnessRate: loudnessRate || 0, // 音量调整 -50 ~ 100
    });

    return NextResponse.json({
      success: true,
      data: {
        audioUrl: response.audioUri,
        audioSize: response.audioSize,
      },
    });
  } catch (error) {
    console.error("TTS转换失败:", error);
    return NextResponse.json(
      { error: "语音合成失败，请稍后重试" },
      { status: 500 }
    );
  }
}

// 获取可用的语音列表
export async function GET() {
  const voices = [
    { id: "zh_female_xiaohe_uranus_bigtts", name: "小荷", type: "女声", description: "通用女声" },
    { id: "zh_female_vv_uranus_bigtts", name: "Vivi", type: "女声", description: "中英双语" },
    { id: "zh_male_m191_uranus_bigtts", name: "云舟", type: "男声", description: "通用男声" },
    { id: "zh_male_taocheng_uranus_bigtts", name: "小天", type: "男声", description: "通用男声" },
    { id: "zh_female_xueayi_saturn_bigtts", name: "故事姐姐", type: "女声", description: "儿童读物" },
    { id: "zh_male_dayi_saturn_bigtts", name: "大义", type: "男声", description: "视频解说" },
  ];

  return NextResponse.json({
    success: true,
    data: voices,
  });
}
