/**
 ****************************************************************************************************
 * @file        myfs.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       文件系统操作 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32S3 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20231201
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __MYFS_H
#define __MYFS_H

#include "Arduino.h"
#include <FS.h>
#include <SD.h>

/* 函数声明 */
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);         /* 遍历目录下的文件 */
void createDir(fs::FS &fs, const char * path);                          /* 创建文件夹 */
void removeDir(fs::FS &fs, const char * path);                          /* 移除文件夹  */
void readFile(fs::FS &fs, const char * path);                           /* 从文件中读取数据 */
void writeFile(fs::FS &fs, const char * path, const char * message);    /* 写数据到文件中 */
void appendFile(fs::FS &fs, const char * path, const char * message);   /* 在文件后追加数据 */
void renameFile(fs::FS &fs, const char * path1, const char * path2);    /* 修改文件/目录名字 */
void deleteFile(fs::FS &fs, const char * path);                         /* 删除文件/目录 */
void testFileIO(fs::FS &fs, const char * path);                         /* 测试文件IO性能 */

#endif
