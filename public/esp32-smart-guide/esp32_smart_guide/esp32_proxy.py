#!/usr/bin/env python3
"""
ESP32 HTTP代理服务器
用于将ESP32的HTTP请求转发到HTTPS服务器
避免校园网封锁443端口的问题

使用方法：
1. 修改 ESP32_TARGET_URL 为您的服务器地址
2. 运行此脚本：python esp32_proxy.py
3. 修改ESP32代码，连接到您电脑的IP地址:8080
"""

import http.server
import urllib.request
import urllib.error
import socketserver
import socket
import json
from typing import Optional, Tuple

# ========== 配置区域 ==========

# 目标HTTPS服务器地址（您的扣子服务器）
ESP32_TARGET_URL = "https://5f587623-7cdf-4670-91d9-45ff048194b3.dev.coze.site"

# 本地Next.js服务器地址（用于音频中转等本地API）
# 注意：使用公网域名而不是localhost，因为代理运行在Windows电脑上
LOCAL_SERVER_URL = "https://5f587623-7cdf-4670-91d9-45ff048194b3.dev.coze.site"

# 本地监听端口（ESP32连接这个端口）
LISTEN_PORT = 8080

# 监听所有网络接口（让ESP32能连接）
LISTEN_HOST = "0.0.0.0"

# ========== 代理实现 ==========

class ESP32ProxyHandler(http.server.BaseHTTPRequestHandler):
    """ESP32 HTTP代理请求处理器"""
    
    def do_GET(self):
        """处理GET请求"""
        self._forward_request('GET')
    
    def do_POST(self):
        """处理POST请求"""
        self._forward_request('POST')
    
    def _forward_request(self, method: str):
        """转发请求到目标服务器"""
        
        # /api/audio 走本地Next.js服务器（音频中转），其余走coze服务器
        if self.path.startswith("/api/audio"):
            target_url = LOCAL_SERVER_URL + self.path
        else:
            target_url = ESP32_TARGET_URL + self.path
        
        print(f"\n{'='*60}")
        print(f"[{method}] 收到请求: {self.path}")
        print(f"目标URL: {target_url}")
        
        # 读取请求体（如果有）
        content_length = int(self.headers.get('Content-Length', 0))
        request_body = None
        if content_length > 0:
            request_body = self.rfile.read(content_length)
            print(f"请求体大小: {len(request_body)} bytes")
            if len(request_body) < 500:
                print(f"请求体预览: {request_body[:500]}...")
        
        # 构建转发请求
        headers = {}
        for key, value in self.headers.items():
            if key.lower() not in ['host', 'content-length']:
                headers[key] = value
        
        try:
            # 创建请求对象
            req = urllib.request.Request(target_url, data=request_body, headers=headers, method=method)
            
            # 发送请求
            print("正在转发到目标服务器...")
            with urllib.request.urlopen(req, timeout=30) as response:
                # 读取响应
                response_body = response.read()
                
                # 发送响应给ESP32
                self.send_response(response.status)
                
                # 转发响应头
                for key, value in response.getheaders():
                    if key.lower() not in ['transfer-encoding', 'content-encoding']:
                        self.send_header(key, value)
                
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                # 发送响应体
                self.wfile.write(response_body)
                
                print(f"✅ 转发成功！状态码: {response.status}")
                print(f"响应大小: {len(response_body)} bytes")
                if len(response_body) < 500:
                    print(f"响应预览: {response_body[:500]}...")
        
        except urllib.error.HTTPError as e:
            # HTTP错误（如404, 500等）
            print(f"❌ HTTP错误: {e.code}")
            self.send_response(e.code)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            error_response = json.dumps({"error": f"HTTP Error: {e.code}", "details": str(e.reason)}).encode()
            self.wfile.write(error_response)
        
        except Exception as e:
            # 其他错误
            print(f"❌ 错误: {type(e).__name__}: {e}")
            self.send_response(502)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            error_response = json.dumps({"error": "Proxy Error", "details": str(e)}).encode()
            self.wfile.write(error_response)
    
    def log_message(self, format, *args):
        """自定义日志格式"""
        print(f"[{self.address_string()}] {format % args}")


def get_local_ip() -> str:
    """获取本机IP地址（用于显示给ESP32连接）"""
    try:
        # 创建一个临时socket连接到外部服务器，获取本机IP
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except Exception:
        return "127.0.0.1"


def main():
    """主函数"""
    local_ip = get_local_ip()
    
    print("\n" + "="*60)
    print("    ESP32 HTTP代理服务器")
    print("="*60)
    print(f"\n📡 目标服务器: {ESP32_TARGET_URL}")
    print(f"🔍 监听地址: {LISTEN_HOST}:{LISTEN_PORT}")
    print(f"💻 您的电脑IP: {local_ip}")
    print(f"\n📱 ESP32配置:")
    print(f"   服务器地址: http://{local_ip}:{LISTEN_PORT}")
    print(f"\n🚀 代理已启动！等待ESP32连接...")
    print(f"\n提示: 按 Ctrl+C 停止代理\n")
    print("="*60 + "\n")
    
    try:
        # 创建服务器
        with socketserver.TCPServer((LISTEN_HOST, LISTEN_PORT), ESP32ProxyHandler) as httpd:
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n✅ 代理服务器已停止")
    except OSError as e:
        if e.errno == 48:  # Address already in use
            print(f"\n❌ 端口 {LISTEN_PORT} 已被占用！")
            print(f"请关闭其他占用端口的程序，或修改脚本中的 LISTEN_PORT")
        else:
            raise


if __name__ == "__main__":
    main()
