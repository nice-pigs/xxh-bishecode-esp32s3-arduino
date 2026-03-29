import { NextRequest, NextResponse } from "next/server";

// 简单的上传测试接口 - 不调用大模型
// 用于测试ESP32的HTTPS上传功能

export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const { image, deviceId } = body;

    console.log('[测试接口] 收到请求');
    console.log('[测试接口] DeviceID:', deviceId);
    
    if (!image) {
      return NextResponse.json(
        { error: "缺少图片数据" },
        { status: 400 }
      );
    }

    console.log('[测试接口] 图片大小:', image.length);
    console.log('[测试接口] 图片前缀:', image.substring(0, 50));

    // 检查图片格式
    if (!image.startsWith('data:image/')) {
      console.error('[测试接口] 图片格式错误');
      return NextResponse.json(
        { error: "图片格式错误" },
        { status: 400 }
      );
    }

    // 简单验证Base64
    const base64Data = image.split(',')[1];
    if (!base64Data) {
      return NextResponse.json(
        { error: "Base64数据格式错误" },
        { status: 400 }
      );
    }

    console.log('[测试接口] Base64数据大小:', base64Data.length);

    // 成功响应 - 不调用大模型
    return NextResponse.json({
      success: true,
      message: "上传测试成功！",
      data: {
        deviceId,
        imageSize: image.length,
        base64Size: base64Data.length,
        timestamp: new Date().toISOString(),
      },
    });
  } catch (error) {
    console.error("[测试接口] 错误:", error);
    const errorMessage = error instanceof Error ? error.message : "测试失败";
    return NextResponse.json(
      { error: errorMessage },
      { status: 500 }
    );
  }
}
