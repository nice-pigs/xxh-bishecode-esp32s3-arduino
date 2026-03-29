import { NextRequest, NextResponse } from "next/server";
import { updateDeviceStatus, getDeviceStatus } from "@/lib/store";

// POST: 上报GPS位置
export async function POST(request: NextRequest) {
  try {
    const body = await request.json();
    const {
      deviceId,
      latitude,
      longitude,
      altitude,
      satellites,
      speed,
      batteryLevel,
      temperature,
      humidity,
      lightIntensity,
      irIntensity,
      proximity,
    } = body;

    if (!deviceId || latitude === undefined || longitude === undefined) {
      return NextResponse.json(
        { error: "缺少必要的位置信息" },
        { status: 400 }
      );
    }

    // 更新设备状态
    const status = await updateDeviceStatus({
      deviceId,
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude),
      altitude: altitude ? parseFloat(altitude) : 0,
      satellites: satellites || 0,
      speed: speed ? parseFloat(speed) : 0,
      batteryLevel,
      temperature: temperature !== undefined ? parseFloat(temperature) : undefined,
      humidity: humidity !== undefined ? parseFloat(humidity) : undefined,
      lightIntensity: lightIntensity !== undefined ? parseFloat(lightIntensity) : undefined,
      irIntensity: irIntensity !== undefined ? parseFloat(irIntensity) : undefined,
      proximity: proximity !== undefined ? parseFloat(proximity) : undefined,
    });

    return NextResponse.json({
      success: true,
      data: {
        deviceId: status.deviceId,
        lastUpdate: status.lastUpdate,
      },
    });
  } catch (error) {
    console.error("位置上报失败:", error);
    return NextResponse.json(
      { error: "位置上报失败" },
      { status: 500 }
    );
  }
}

// GET: 获取当前设备状态
export async function GET() {
  const status = await getDeviceStatus();
  
  if (!status) {
    return NextResponse.json({
      success: true,
      data: null,
      message: "设备尚未连接",
    });
  }

  return NextResponse.json({
    success: true,
    data: status,
  });
}
