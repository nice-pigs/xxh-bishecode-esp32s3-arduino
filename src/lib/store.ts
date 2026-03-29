/**
 * Supabase数据库存储
 * 替代之前的内存存储，数据持久化到数据库
 */

import { getSupabaseClient } from '@/storage/database/supabase-client';

export interface SpotRecord {
  id: string;
  imageUrl: string;
  spotName: string;
  description: string;
  latitude: number;
  longitude: number;
  timestamp: Date;
  audioUrl?: string;
  audioUrlMp3?: string;
}

export interface DeviceStatus {
  deviceId: string;
  latitude: number;
  longitude: number;
  altitude: number;
  satellites: number;
  speed: number;
  lastUpdate: Date;
  batteryLevel?: number;
  temperature?: number;    // 温度 (DHT11)
  humidity?: number;       // 湿度 (DHT11)
  lightIntensity?: number; // 光照强度 (AP3216C ALS)
  irIntensity?: number;    // 红外强度 (AP3216C IR)
  proximity?: number;      // 接近度 (AP3216C PS)
}

// 生成唯一ID
export function generateId(): string {
  return `spot_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
}

// 添加识别记录
export async function addSpotRecord(record: Omit<SpotRecord, 'id' | 'timestamp'>): Promise<SpotRecord> {
  const client = getSupabaseClient();
  const newRecord: SpotRecord = {
    ...record,
    id: generateId(),
    timestamp: new Date(),
  };

  // 转换为snake_case存入数据库
  const dbRecord = {
    id: newRecord.id,
    image_url: newRecord.imageUrl,
    spot_name: newRecord.spotName,
    description: newRecord.description,
    latitude: newRecord.latitude,
    longitude: newRecord.longitude,
    audio_url: newRecord.audioUrl,
    audio_url_mp3: newRecord.audioUrlMp3,
    created_at: newRecord.timestamp.toISOString(),
  };

  const { data, error } = await client
    .from('spot_records')
    .insert(dbRecord)
    .select()
    .single();

  if (error) {
    throw new Error(`添加记录失败: ${error.message}`);
  }

  // 转换回camelCase
  return {
    id: data.id,
    imageUrl: data.image_url,
    spotName: data.spot_name,
    description: data.description,
    latitude: data.latitude,
    longitude: data.longitude,
    audioUrl: data.audio_url,
    audioUrlMp3: data.audio_url_mp3,
    timestamp: new Date(data.created_at),
  };
}

// 获取识别历史
export async function getSpotHistory(limit: number = 50): Promise<SpotRecord[]> {
  const client = getSupabaseClient();
  
  const { data, error } = await client
    .from('spot_records')
    .select('*')
    .order('created_at', { ascending: false })
    .limit(limit);

  if (error) {
    throw new Error(`获取历史记录失败: ${error.message}`);
  }

  // 转换为camelCase
  return data.map((item: any) => ({
    id: item.id,
    imageUrl: item.image_url,
    spotName: item.spot_name,
    description: item.description,
    latitude: item.latitude,
    longitude: item.longitude,
    audioUrl: item.audio_url,
    audioUrlMp3: item.audio_url_mp3,
    timestamp: new Date(item.created_at),
  }));
}

// 获取单个记录
export async function getSpotById(id: string): Promise<SpotRecord | undefined> {
  const client = getSupabaseClient();
  
  const { data, error } = await client
    .from('spot_records')
    .select('*')
    .eq('id', id)
    .maybeSingle();

  if (error) {
    throw new Error(`获取记录失败: ${error.message}`);
  }

  if (!data) {
    return undefined;
  }

  // 转换为camelCase
  return {
    id: data.id,
    imageUrl: data.image_url,
    spotName: data.spot_name,
    description: data.description,
    latitude: data.latitude,
    longitude: data.longitude,
    audioUrl: data.audio_url,
    audioUrlMp3: data.audio_url_mp3,
    timestamp: new Date(data.created_at),
  };
}

// 更新设备状态
export async function updateDeviceStatus(status: Omit<DeviceStatus, 'lastUpdate'>): Promise<DeviceStatus> {
  const client = getSupabaseClient();
  
  const newStatus: DeviceStatus = {
    ...status,
    lastUpdate: new Date(),
  };

  // 转换为snake_case
  const dbStatus = {
    device_id: newStatus.deviceId,
    latitude: newStatus.latitude,
    longitude: newStatus.longitude,
    altitude: newStatus.altitude,
    satellites: newStatus.satellites,
    speed: newStatus.speed,
    battery_level: newStatus.batteryLevel,
    temperature: newStatus.temperature,
    humidity: newStatus.humidity,
    light_intensity: newStatus.lightIntensity,
    ir_intensity: newStatus.irIntensity,
    proximity: newStatus.proximity,
    last_update: newStatus.lastUpdate.toISOString(),
  };

  // 使用upsert，如果device_id已存在则更新，否则插入
  const { data, error } = await client
    .from('device_status')
    .upsert(dbStatus, { onConflict: 'device_id' })
    .select()
    .single();

  if (error) {
    throw new Error(`更新设备状态失败: ${error.message}`);
  }

  // 转换回camelCase
  return {
    deviceId: data.device_id,
    latitude: data.latitude,
    longitude: data.longitude,
    altitude: data.altitude,
    satellites: data.satellites,
    speed: data.speed,
    batteryLevel: data.battery_level,
    temperature: data.temperature,
    humidity: data.humidity,
    lightIntensity: data.light_intensity,
    irIntensity: data.ir_intensity,
    proximity: data.proximity,
    lastUpdate: new Date(data.last_update),
  };
}

// 获取设备状态
export async function getDeviceStatus(): Promise<DeviceStatus | null> {
  const client = getSupabaseClient();
  
  // 获取最新的设备状态
  const { data, error } = await client
    .from('device_status')
    .select('*')
    .order('last_update', { ascending: false })
    .limit(1)
    .maybeSingle();

  if (error) {
    throw new Error(`获取设备状态失败: ${error.message}`);
  }

  if (!data) {
    return null;
  }

  // 转换为camelCase
  return {
    deviceId: data.device_id,
    latitude: data.latitude,
    longitude: data.longitude,
    altitude: data.altitude,
    satellites: data.satellites,
    speed: data.speed,
    batteryLevel: data.battery_level,
    lastUpdate: new Date(data.last_update),
  };
}

// 清空历史记录
export async function clearHistory(): Promise<void> {
  const client = getSupabaseClient();
  
  const { error } = await client
    .from('spot_records')
    .delete()
    .not('id', 'is', null); // 删除所有记录

  if (error) {
    throw new Error(`清空历史记录失败: ${error.message}`);
  }
}
