import { NextRequest, NextResponse } from "next/server";

export async function GET(request: NextRequest) {
  try {
    const { searchParams } = new URL(request.url);
    const audioUrl = searchParams.get("url");

    if (!audioUrl) {
      return NextResponse.json(
        { error: "缺少音频URL参数" },
        { status: 400 }
      );
    }

    console.log("[音频中转] 正在下载音频:", audioUrl.substring(0, 100) + "...");

    // 从外部URL下载音频
    const response = await fetch(audioUrl);
    
    if (!response.ok) {
      console.error("[音频中转] 下载失败:", response.status);
      return NextResponse.json(
        { error: "音频下载失败" },
        { status: response.status }
      );
    }

    // 获取音频数据
    const audioData = await response.arrayBuffer();
    // 强制设置PCM Content-Type，让ESP32能正确识别格式
    const contentType = audioUrl.includes(".pcm") 
      ? "audio/pcm" 
      : (response.headers.get("content-type") || "audio/octet-stream");

    console.log(`[音频中转] 下载完成: ${audioData.byteLength} bytes, Content-Type: ${contentType}`);

    // 返回音频数据给ESP32
    return new NextResponse(audioData, {
      headers: {
        "Content-Type": contentType,
        "Content-Length": String(audioData.byteLength),
      },
    });
  } catch (error) {
    console.error("[音频中转] 错误:", error);
    return NextResponse.json(
      { error: "音频中转失败", details: String(error) },
      { status: 500 }
    );
  }
}
