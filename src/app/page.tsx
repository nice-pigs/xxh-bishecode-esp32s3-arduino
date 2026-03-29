"use client";

import { useState, useEffect, useCallback, useRef } from "react";
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { Separator } from "@/components/ui/separator";
import { ScrollArea } from "@/components/ui/scroll-area";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { Input } from "@/components/ui/input";
import { Textarea } from "@/components/ui/textarea";
import { 
  MapPin, 
  Camera, 
  Clock, 
  Volume2, 
  RefreshCw, 
  Wifi, 
  Battery,
  Satellite,
  Mountain,
  Upload,
  X,
  Image as ImageIcon,
  ZoomIn
} from "lucide-react";
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogDescription } from "@/components/ui/dialog";

// 类型定义
interface SpotRecord {
  id: string;
  imageUrl: string;
  spotName: string;
  description: string;
  latitude: number;
  longitude: number;
  timestamp: string;
  audioUrl?: string;      // PCM格式（ESP32用）
  audioUrlMp3?: string;   // MP3格式（网页用）
}

interface DeviceStatus {
  deviceId: string;
  latitude: number;
  longitude: number;
  altitude: number;
  satellites: number;
  speed: number;
  lastUpdate: string;
  batteryLevel?: number;
  temperature?: number;    // 温度 (DHT11)
  humidity?: number;       // 湿度 (DHT11)
  lightIntensity?: number; // 光照强度 (AP3216C ALS)
  irIntensity?: number;    // 红外强度 (AP3216C IR)
  proximity?: number;      // 接近度 (AP3216C PS)
}

export default function Home() {
  const [apiBase, setApiBase] = useState('');
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus | null>(null);
  const [spotHistory, setSpotHistory] = useState<SpotRecord[]>([]);
  const [loading, setLoading] = useState(false);
  const [selectedSpot, setSelectedSpot] = useState<SpotRecord | null>(null);
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [imagePreview, setImagePreview] = useState<string | null>(null);
  const [recognizing, setRecognizing] = useState(false);
  const [failedImages, setFailedImages] = useState<Set<string>>(new Set());
  const [zoomImage, setZoomImage] = useState<string | null>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  // 客户端初始化API基础URL
  useEffect(() => {
    setApiBase(window.location.origin);
  }, []);

  // 获取设备状态
  const fetchDeviceStatus = useCallback(async () => {
    try {
      const response = await fetch(`${apiBase}/api/location`);
      const data = await response.json();
      if (data.success) {
        setDeviceStatus(data.data);
      }
    } catch (error) {
      console.error("获取设备状态失败:", error);
    }
  }, [apiBase]);

  // 获取识别历史
  const fetchHistory = useCallback(async () => {
    try {
      const response = await fetch(`${apiBase}/api/history?limit=20`);
      const data = await response.json();
      if (data.success) {
        setSpotHistory(data.data);
      }
    } catch (error) {
      console.error("获取历史记录失败:", error);
    }
  }, [apiBase]);

  // 初始化和定时刷新
  useEffect(() => {
    fetchDeviceStatus();
    fetchHistory();
    
    // 每5秒刷新一次设备状态
    const interval = setInterval(fetchDeviceStatus, 5000);
    return () => clearInterval(interval);
  }, [fetchDeviceStatus, fetchHistory]);

  // 播放语音（优先使用MP3格式）
  const playAudio = (spot: SpotRecord) => {
    const audioUrl = spot.audioUrlMp3 || spot.audioUrl;
    if (audioUrl) {
      const audio = new Audio(audioUrl);
      audio.play().catch(err => {
        console.error("播放失败:", err);
        alert("音频播放失败，可能格式不支持");
      });
    }
  };

  // 处理文件选择
  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      setSelectedFile(file);
      const reader = new FileReader();
      reader.onloadend = () => {
        setImagePreview(reader.result as string);
      };
      reader.readAsDataURL(file);
    }
  };

  // 清除选中的图片
  const clearImage = () => {
    setSelectedFile(null);
    setImagePreview(null);
    if (fileInputRef.current) {
      fileInputRef.current.value = '';
    }
  };

  // 上传图片并识别
  const handleRecognize = async () => {
    if (imagePreview) {
      setRecognizing(true);
      try {
        const response = await fetch(`${apiBase}/api/recognize`, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            image: imagePreview,
          }),
        });
        
        const data = await response.json();
        
        if (data.success) {
          alert("识别成功！");
          fetchHistory(); // 刷新历史记录
          clearImage();
        } else {
          alert("识别失败：" + (data.error || "未知错误"));
        }
      } catch (error) {
        console.error("识别失败:", error);
        alert("识别失败，请重试");
      } finally {
        setRecognizing(false);
      }
    }
  };

  // 格式化时间
  const formatTime = (dateStr: string) => {
    const date = new Date(dateStr);
    return date.toLocaleString("zh-CN", {
      month: "2-digit",
      day: "2-digit",
      hour: "2-digit",
      minute: "2-digit",
    });
  };

  // 格式化坐标
  const formatCoord = (value: number, type: "lat" | "lng") => {
    if (!value) return "--";
    const dir = type === "lat" ? (value >= 0 ? "N" : "S") : (value >= 0 ? "E" : "W");
    return `${Math.abs(value).toFixed(6)}° ${dir}`;
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 to-blue-50 dark:from-slate-950 dark:to-blue-950">
      {/* 顶部标题栏 */}
      <header className="border-b bg-white/80 backdrop-blur-sm dark:bg-slate-900/80 sticky top-0 z-50">
        <div className="container mx-auto px-4 py-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="p-2 bg-blue-500 rounded-lg">
                <Mountain className="w-6 h-6 text-white" />
              </div>
              <div>
                <h1 className="text-xl font-bold text-slate-900 dark:text-white">
                  智能景区讲解系统
                </h1>
                <p className="text-sm text-slate-500 dark:text-slate-400">
                  ESP32-S3 智能导览设备管理平台
                </p>
              </div>
            </div>
            <div className="flex items-center gap-2">
              <Button variant="outline" size="sm" onClick={() => { fetchDeviceStatus(); fetchHistory(); }}>
                <RefreshCw className="w-4 h-4 mr-1" />
                刷新
              </Button>
            </div>
          </div>
        </div>
        </header>

      <main className="container mx-auto px-4 py-6">
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* 左侧：设备状态 + 上传识别 */}
          <div className="lg:col-span-1 space-y-4">
            {/* 设备状态卡片 */}
            <Card>
              <CardHeader className="pb-3">
                <CardTitle className="flex items-center gap-2 text-lg">
                  <Wifi className="w-5 h-5 text-blue-500" />
                  设备状态
                </CardTitle>
              </CardHeader>
              <CardContent className="space-y-4">
                {deviceStatus ? (
                  <>
                    <div className="flex items-center justify-between">
                      <span className="text-sm text-slate-500">设备ID</span>
                      <Badge variant="secondary">{deviceStatus.deviceId}</Badge>
                    </div>
                    <Separator />
                    
                    {/* GPS信息 */}
                    <div className="space-y-2">
                      <div className="flex items-center gap-2 text-sm font-medium">
                        <Satellite className="w-4 h-4 text-green-500" />
                        GPS定位
                      </div>
                      <div className="grid grid-cols-2 gap-2 text-sm">
                        <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                          <div className="text-slate-500 text-xs">纬度</div>
                          <div className="font-mono">{formatCoord(deviceStatus.latitude, "lat")}</div>
                        </div>
                        <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                          <div className="text-slate-500 text-xs">经度</div>
                          <div className="font-mono">{formatCoord(deviceStatus.longitude, "lng")}</div>
                        </div>
                      </div>
                      <div className="grid grid-cols-3 gap-2 text-sm">
                        <div className="bg-slate-100 dark:bg-slate-800 rounded p-2 text-center">
                          <div className="text-slate-500 text-xs">海拔</div>
                          <div className="font-mono">{deviceStatus.altitude.toFixed(1)}m</div>
                        </div>
                        <div className="bg-slate-100 dark:bg-slate-800 rounded p-2 text-center">
                          <div className="text-slate-500 text-xs">卫星数</div>
                          <div className="font-mono">{deviceStatus.satellites}</div>
                        </div>
                        <div className="bg-slate-100 dark:bg-slate-800 rounded p-2 text-center">
                          <div className="text-slate-500 text-xs">速度</div>
                          <div className="font-mono">{deviceStatus.speed.toFixed(1)}m/s</div>
                        </div>
                      </div>
                    </div>
                    
                    <Separator />
                    
                    {/* 传感器数据 */}
                    {(deviceStatus.temperature !== undefined || deviceStatus.humidity !== undefined || 
                      deviceStatus.lightIntensity !== undefined) && (
                      <>
                        <div className="space-y-2">
                          <div className="flex items-center gap-2 text-sm font-medium">
                            <ImageIcon className="w-4 h-4 text-orange-500" />
                            传感器数据
                          </div>
                          <div className="grid grid-cols-2 gap-2 text-sm">
                            {deviceStatus.temperature !== undefined && (
                              <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                                <div className="text-slate-500 text-xs">温度</div>
                                <div className="font-mono">{deviceStatus.temperature}°C</div>
                              </div>
                            )}
                            {deviceStatus.humidity !== undefined && (
                              <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                                <div className="text-slate-500 text-xs">湿度</div>
                                <div className="font-mono">{deviceStatus.humidity}%</div>
                              </div>
                            )}
                            {deviceStatus.lightIntensity !== undefined && (
                              <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                                <div className="text-slate-500 text-xs">光照</div>
                                <div className="font-mono">{deviceStatus.lightIntensity} lux</div>
                              </div>
                            )}
                            {deviceStatus.irIntensity !== undefined && (
                              <div className="bg-slate-100 dark:bg-slate-800 rounded p-2">
                                <div className="text-slate-500 text-xs">红外</div>
                                <div className="font-mono">{deviceStatus.irIntensity}</div>
                              </div>
                            )}
                            {deviceStatus.proximity !== undefined && (
                              <div className="bg-slate-100 dark:bg-slate-800 rounded p-2 col-span-2">
                                <div className="text-slate-500 text-xs">接近度</div>
                                <div className="font-mono">{deviceStatus.proximity}</div>
                              </div>
                            )}
                          </div>
                        </div>
                        <Separator />
                      </>
                    )}
                    
                    {/* 电池和更新时间 */}
                    <div className="flex items-center justify-between text-sm">
                      <div className="flex items-center gap-1">
                        <Battery className="w-4 h-4 text-green-500" />
                        <span>{deviceStatus.batteryLevel ? `${deviceStatus.batteryLevel}%` : "外接电源"}</span>
                      </div>
                      <div className="flex items-center gap-1 text-slate-500">
                        <Clock className="w-4 h-4" />
                        <span>{formatTime(deviceStatus.lastUpdate)}</span>
                      </div>
                    </div>
                  </>
                ) : (
                  <div className="text-center py-8 text-slate-500">
                    <Wifi className="w-8 h-8 mx-auto mb-2 opacity-50" />
                    <p>设备未连接</p>
                    <p className="text-xs mt-1">等待ESP32设备上报位置...</p>
                  </div>
                )}
              </CardContent>
            </Card>

            {/* 上传图片识别卡片 */}
            <Card>
              <CardHeader className="pb-3">
                <CardTitle className="flex items-center gap-2 text-lg">
                  <Upload className="w-5 h-5 text-green-500" />
                  上传图片识别
                </CardTitle>
                <CardDescription>从电脑上传图片进行景点识别</CardDescription>
              </CardHeader>
              <CardContent className="space-y-4">
                {!imagePreview ? (
                  <div 
                    className="border-2 border-dashed border-slate-300 dark:border-slate-700 rounded-lg p-8 text-center cursor-pointer hover:border-blue-500 transition-colors"
                    onClick={() => fileInputRef.current?.click()}
                  >
                    <input
                      ref={fileInputRef}
                      type="file"
                      accept="image/*"
                      className="hidden"
                      onChange={handleFileSelect}
                    />
                    <Upload className="w-12 h-12 mx-auto mb-4 text-slate-400" />
                    <p className="text-slate-600 dark:text-slate-400">点击或拖拽图片到此处</p>
                    <p className="text-xs text-slate-500 mt-1">支持 JPG、PNG 格式</p>
                  </div>
                ) : (
                  <div className="space-y-4">
                    <div className="relative">
                      <img 
                        src={imagePreview} 
                        alt="预览" 
                        className="w-full h-48 object-cover rounded-lg"
                      />
                      <Button 
                        variant="destructive"
                        size="icon"
                        className="absolute top-2 right-2"
                        onClick={clearImage}
                      >
                        <X className="w-4 h-4" />
                      </Button>
                    </div>
                    <Button 
                      className="w-full"
                      onClick={handleRecognize}
                      disabled={recognizing}
                    >
                      {recognizing ? (
                        <>
                          <RefreshCw className="w-4 h-4 mr-2 animate-spin" />
                          识别中...
                        </>
                      ) : (
                        <>
                          <Camera className="w-4 h-4 mr-2" />
                          开始识别
                        </>
                      )}
                    </Button>
                  </div>
                )}
              </CardContent>
            </Card>
          </div>

          {/* 右侧：识别历史 */}
          <div className="lg:col-span-2">
            <Card className="h-full">
              <CardHeader>
                <div className="flex items-center justify-between">
                  <CardTitle className="flex items-center gap-2">
                    <Camera className="w-5 h-5 text-blue-500" />
                    识别历史
                  </CardTitle>
                  <Badge variant="outline">{spotHistory.length} 条记录</Badge>
                </div>
                <CardDescription>景点识别记录和语音讲解</CardDescription>
              </CardHeader>
              <CardContent>
                {spotHistory.length > 0 ? (
                  <ScrollArea className="h-[500px] pr-4">
                    <div className="space-y-4">
                      {spotHistory.map((spot) => (
                        <Card 
                          key={spot.id} 
                          className={`cursor-pointer transition-all duration-200 hover:shadow-lg hover:scale-[1.01] ${
                            selectedSpot?.id === spot.id 
                              ? "ring-2 ring-blue-500 bg-blue-50/50 dark:bg-blue-950/30" 
                              : "hover:border-blue-300 dark:hover:border-blue-700"
                          }`}
                          onClick={() => {
                            console.log('点击历史记录:', spot.id, spot.spotName);
                            setSelectedSpot(spot);
                          }}
                        >
                          <CardContent className="p-4">
                            <div className="flex gap-4">
                              {/* 图片预览 */}
                              {spot.imageUrl && spot.imageUrl.length > 100 && !failedImages.has(spot.id) ? (
                                <div className="flex-shrink-0 relative group">
                                  <img 
                                    src={spot.imageUrl} 
                                    alt={spot.spotName}
                                    className="w-32 h-24 object-cover rounded-lg shadow-sm bg-slate-200 dark:bg-slate-700"
                                    loading="lazy"
                                    onError={(e) => {
                                      console.error('缩略图加载失败:', spot.id, '图片长度:', spot.imageUrl?.length);
                                      setFailedImages(prev => new Set([...prev, spot.id]));
                                    }}
                                  />
                                  <Button
                                    variant="secondary"
                                    size="icon"
                                    className="absolute top-1 right-1 opacity-0 group-hover:opacity-100 transition-opacity bg-black/50 hover:bg-black/70"
                                    onClick={(e) => {
                                      e.stopPropagation();
                                      setZoomImage(spot.imageUrl);
                                    }}
                                  >
                                    <ZoomIn className="w-4 h-4" />
                                  </Button>
                                </div>
                              ) : (
                                <div className="flex-shrink-0 w-32 h-24 bg-slate-200 dark:bg-slate-700 rounded-lg flex items-center justify-center">
                                  <div className="text-slate-400 text-xs text-center p-2">
                                    <div className="text-lg mb-1">🖼️</div>
                                    无图片
                                  </div>
                                </div>
                              )}
                              {/* 内容区域 */}
                              <div className="flex-1 min-w-0">
                                <div className="flex items-start justify-between">
                                  <div className="flex-1">
                                    <div className="flex items-center gap-2 mb-2">
                                      <MapPin className="w-4 h-4 text-red-500" />
                                      <h3 className="font-semibold text-lg">{spot.spotName}</h3>
                                      {selectedSpot?.id === spot.id && (
                                        <Badge variant="default" className="ml-2 text-xs">已选中</Badge>
                                      )}
                                    </div>
                                    <p className="text-sm text-slate-600 dark:text-slate-400 line-clamp-2">
                                      {spot.description}
                                    </p>
                                    <div className="flex items-center gap-4 mt-3 text-xs text-slate-500">
                                      <div className="flex items-center gap-1">
                                        <Clock className="w-3 h-3" />
                                        {formatTime(spot.timestamp)}
                                      </div>
                                      {spot.latitude !== 0 && (
                                        <div className="flex items-center gap-1">
                                          <MapPin className="w-3 h-3" />
                                          {spot.latitude.toFixed(4)}, {spot.longitude.toFixed(4)}
                                        </div>
                                      )}
                                    </div>
                                  </div>
                                  {(spot.audioUrl || spot.audioUrlMp3) && (
                                    <Button 
                                      size="sm" 
                                      variant="outline"
                                      className="flex-shrink-0"
                                      onClick={(e) => {
                                        e.stopPropagation();
                                        playAudio(spot);
                                      }}
                                    >
                                      <Volume2 className="w-4 h-4 mr-1" />
                                      播放
                                    </Button>
                                  )}
                                </div>
                              </div>
                            </div>
                          </CardContent>
                        </Card>
                      ))}
                    </div>
                  </ScrollArea>
                ) : (
                  <div className="text-center py-16 text-slate-500">
                    <Camera className="w-12 h-12 mx-auto mb-4 opacity-50" />
                    <p className="text-lg">暂无识别记录</p>
                    <p className="text-sm mt-1">使用ESP32设备拍摄或上传图片开始识别</p>
                  </div>
                )}
              </CardContent>
            </Card>
          </div>
        </div>

        {/* 底部：选中景点详情 */}
        {selectedSpot && (
          <Card className="mt-6 border-2 border-blue-200 dark:border-blue-800">
            <CardHeader className="bg-blue-50 dark:bg-blue-950/50">
              <div className="flex items-center justify-between">
                <CardTitle className="flex items-center gap-2">
                  <MapPin className="w-5 h-5 text-red-500" />
                  {selectedSpot.spotName}
                </CardTitle>
                <Button variant="ghost" size="sm" onClick={() => setSelectedSpot(null)}>
                  <X className="w-4 h-4" />
                </Button>
              </div>
            </CardHeader>
            <CardContent className="space-y-6 pt-6">
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                {/* 左侧：大图预览 */}
                {selectedSpot.imageUrl && selectedSpot.imageUrl.length > 100 && !failedImages.has(selectedSpot.id) ? (
                  <div className="space-y-2">
                    <div className="flex items-center justify-between">
                      <h4 className="text-sm font-medium text-slate-500">识别图片</h4>
                      <Button
                        variant="outline"
                        size="sm"
                        onClick={() => setZoomImage(selectedSpot.imageUrl)}
                      >
                        <ZoomIn className="w-4 h-4 mr-1" />
                        放大查看
                      </Button>
                    </div>
                    <div className="bg-slate-100 dark:bg-slate-800 rounded-lg p-4 flex justify-center items-center min-h-[200px] cursor-pointer"
                         onClick={() => setZoomImage(selectedSpot.imageUrl)}>
                      <img 
                        src={selectedSpot.imageUrl} 
                        alt={selectedSpot.spotName}
                        className="max-w-full max-h-[300px] object-contain rounded-lg shadow-lg hover:opacity-90 transition-opacity"
                        loading="eager"
                        onLoad={() => {
                          console.log('详情图片加载成功:', selectedSpot.id);
                        }}
                        onError={(e) => {
                          console.error('详情图片加载失败:', selectedSpot.id, '图片长度:', selectedSpot.imageUrl?.length);
                          setFailedImages(prev => new Set([...prev, selectedSpot.id]));
                        }}
                      />
                    </div>
                  </div>
                ) : (
                  <div className="space-y-2">
                    <h4 className="text-sm font-medium text-slate-500">识别图片</h4>
                    <div className="bg-slate-100 dark:bg-slate-800 rounded-lg p-4 flex justify-center items-center min-h-[200px]">
                      <div className="text-slate-400 text-center">
                        <div className="text-4xl mb-2">🖼️</div>
                        <div>{failedImages.has(selectedSpot.id) ? '图片加载失败' : '无图片数据'}</div>
                        <div className="text-xs mt-1">
                          图片长度: {selectedSpot.imageUrl?.length || 0}
                        </div>
                      </div>
                    </div>
                  </div>
                )}
                
                {/* 右侧：详细信息 */}
                <div className="space-y-4">
                  <div>
                    <h4 className="text-sm font-medium text-slate-500 mb-2">景点名称</h4>
                    <p className="text-xl font-bold text-slate-900 dark:text-white">{selectedSpot.spotName}</p>
                  </div>
                  
                  <div>
                    <h4 className="text-sm font-medium text-slate-500 mb-2">详细介绍</h4>
                    <div className="bg-slate-50 dark:bg-slate-800/50 rounded-lg p-4">
                      <p className="text-slate-700 dark:text-slate-300 leading-relaxed whitespace-pre-wrap">
                        {selectedSpot.description}
                      </p>
                    </div>
                  </div>
                  
                  <div className="flex items-center gap-2 text-xs text-slate-500">
                    <Clock className="w-3 h-3" />
                    <span>识别时间：{formatTime(selectedSpot.timestamp)}</span>
                  </div>
                </div>
              </div>
              
              {/* 语音播放区域 */}
              {(selectedSpot.audioUrl || selectedSpot.audioUrlMp3) && (
                <div className="border-t pt-4">
                  <h4 className="text-sm font-medium text-slate-500 mb-3">语音讲解</h4>
                  <div className="flex flex-wrap items-center gap-4">
                    <Button onClick={() => playAudio(selectedSpot)} className="gap-2">
                      <Volume2 className="w-4 h-4" />
                      播放语音讲解
                    </Button>
                    {selectedSpot.audioUrlMp3 && (
                      <div className="flex-1">
                        <audio 
                          controls 
                          className="w-full h-10" 
                          src={selectedSpot.audioUrlMp3}
                          onError={(e) => console.error('音频加载失败:', e)}
                        >
                          您的浏览器不支持音频播放
                        </audio>
                      </div>
                    )}
                  </div>
                </div>
              )}
            </CardContent>
          </Card>
        )}
      </main>

      {/* 底部状态栏 */}
      <footer className="border-t bg-white/80 backdrop-blur-sm dark:bg-slate-900/80 py-3 mt-auto">
        <div className="container mx-auto px-4 flex items-center justify-between text-sm text-slate-500">
          <div className="flex items-center gap-4">
            <span>智能景区讲解系统 v1.0</span>
            <Badge variant={deviceStatus ? "default" : "secondary"} className="text-xs">
              {deviceStatus ? "设备在线" : "设备离线"}
            </Badge>
          </div>
          <div className="flex items-center gap-2">
            <span>服务地址: {apiBase}</span>
          </div>
        </div>
      </footer>
      
      {/* 图片放大查看对话框 */}
      <Dialog open={!!zoomImage} onOpenChange={(open) => !open && setZoomImage(null)}>
        <DialogContent className="max-w-4xl max-h-[90vh]">
          <DialogHeader>
            <DialogTitle>图片查看</DialogTitle>
            <DialogDescription>点击图片外区域或按ESC关闭</DialogDescription>
          </DialogHeader>
          <div className="flex items-center justify-center p-4 bg-slate-100 dark:bg-slate-800 rounded-lg min-h-[50vh]">
            {zoomImage && (
              <img 
                src={zoomImage} 
                alt="放大查看" 
                className="max-w-full max-h-[70vh] object-contain rounded-lg shadow-2xl"
              />
            )}
          </div>
        </DialogContent>
      </Dialog>
    </div>
  );
}
