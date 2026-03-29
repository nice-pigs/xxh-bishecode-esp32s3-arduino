import { NextRequest, NextResponse } from "next/server";
import { getSpotHistory, getSpotById } from "@/lib/store";

// GET: 获取识别历史
export async function GET(request: NextRequest) {
  const { searchParams } = new URL(request.url);
  const limit = parseInt(searchParams.get("limit") || "50");
  const id = searchParams.get("id");

  // 如果指定了ID，返回单条记录
  if (id) {
    const record = await getSpotById(id);
    if (!record) {
      return NextResponse.json(
        { error: "记录不存在" },
        { status: 404 }
      );
    }
    return NextResponse.json({
      success: true,
      data: record,
    });
  }

  // 返回历史列表
  const history = await getSpotHistory(limit);
  
  return NextResponse.json({
    success: true,
    data: history,
    total: history.length,
  });
}
