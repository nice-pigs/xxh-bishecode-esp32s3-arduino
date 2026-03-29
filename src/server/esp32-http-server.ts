/**
 * ESP32专用HTTP服务器
 * 监听8080端口，避免校园网封锁443端口的问题
 * 将HTTP请求转发到Next.js的API路由
 */

import http from 'http';
import { parse } from 'url';
import { createProxyServer } from 'http-proxy';

// 代理目标：本地Next.js服务器（通常在3000端口，但我们部署环境是5000端口）
const TARGET_URL = 'http://localhost:5000';

// HTTP代理服务器
const proxy = createProxyServer({
  target: TARGET_URL,
  changeOrigin: true,
  xfwd: true,
});

// 创建HTTP服务器
const server = http.createServer((req, res) => {
  const parsedUrl = parse(req.url || '/');
  console.log(`[ESP32 HTTP] ${req.method} ${parsedUrl.pathname}`);
  
  // 代理到Next.js
  proxy.web(req, res, {
    target: TARGET_URL,
  }, (err) => {
    console.error('[ESP32 HTTP] 代理错误:', err);
    res.writeHead(500, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ error: '代理失败', details: String(err) }));
  });
});

// 代理错误处理
proxy.on('error', (err, req, res) => {
  console.error('[ESP32 HTTP] 代理错误:', err);
  if (!res.headersSent) {
    res.writeHead(500, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ error: '代理错误', details: String(err) }));
  }
});

// 启动HTTP服务器
const PORT = 8080;
server.listen(PORT, '0.0.0.0', () => {
  console.log(`\n========================================`);
  console.log(`  ESP32专用HTTP服务器已启动`);
  console.log(`  监听端口: ${PORT}`);
  console.log(`  代理目标: ${TARGET_URL}`);
  console.log(`========================================\n`);
});

// 优雅关闭
process.on('SIGTERM', () => {
  console.log('[ESP32 HTTP] 收到SIGTERM，正在关闭...');
  server.close(() => {
    console.log('[ESP32 HTTP] 服务器已关闭');
    process.exit(0);
  });
});

process.on('SIGINT', () => {
  console.log('[ESP32 HTTP] 收到SIGINT，正在关闭...');
  server.close(() => {
    console.log('[ESP32 HTTP] 服务器已关闭');
    process.exit(0);
  });
});
