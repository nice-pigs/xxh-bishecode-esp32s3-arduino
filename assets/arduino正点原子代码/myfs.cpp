/**
 ****************************************************************************************************
 * @file        myfs.cpp
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

#include "myfs.h"
#include <FS.h>
#include <SD.h>

/**
 * @brief       遍历目录下的文件
 * @param       fs : 文件对象
 * @param       dirname : 目录路径
 * @param       levels : 遍历深度
 * @retval      无
 */
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root)
    {
        Serial.println("Failed to open directory");
        return;
    }

    if(!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
            {
                listDir(fs, file.path(), levels -1);
            }
        } 
        else 
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

/**
 * @brief       创建文件夹
 * @param       fs : 文件对象
 * @param       path : 需要创建的文件夹名。其中需要包含路径，路径用'/'分隔，比如"/mydir"
 * @retval      无
 */
void createDir(fs::FS &fs, const char * path)
{
    Serial.printf("Creating Dir: %s\n", path);

    if(fs.mkdir(path))
    {
        Serial.println("Dir created");
    } 
    else 
    {
        Serial.println("mkdir failed");
    }
}

/**
 * @brief       移除文件夹 
 * @param       fs : 文件对象
 * @param       path : 需要移除的文件夹名。其中需要包含路径，路径用'/'分隔，比如"/mydir"
 * @retval      无
 */
void removeDir(fs::FS &fs, const char * path)
{
    Serial.printf("Removing Dir: %s\n", path);

    if(fs.rmdir(path))
    {
        Serial.println("Dir removed");
    } 
    else 
    {
        Serial.println("rmdir failed");
    }
}

/**
 * @brief       从文件中读取数据 
 * @param       fs : 文件对象
 * @param       path : 需要读取的文件。其中需要包含路径，路径用'/'分隔，比如"/test.txt"
 * @retval      无
 */
void readFile(fs::FS &fs, const char * path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

/**
 * @brief       写数据到文件中
 * @param       fs : 文件对象
 * @param       path : 需要写入的文件。其中需要包含路径，路径用'/'分隔，比如"/test.txt"
 * @retval      无
 */
void writeFile(fs::FS &fs, const char * path, const char * message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print(message))
    {
        Serial.println("File written");
    } 
    else 
    {
        Serial.println("Write failed");
    }
    file.close();
}

/**
 * @brief       在文件后追加数据
 * @param       fs : 文件对象
 * @param       path : 需要写入的文件。其中需要包含路径，路径用'/'分隔，比如"/test.txt"
 * @param       message : 需要追加写入到文件的数据
 * @retval      无
 */
void appendFile(fs::FS &fs, const char * path, const char * message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }

    if(file.print(message))
    {
        Serial.println("Message appended");
    } 
    else 
    {
        Serial.println("Append failed");
    }
    file.close();
}

/**
 * @brief       修改文件/目录名字(如果目录不同,还可以移动文件哦!)
 * @param       fs : 文件对象
 * @param       path1 : 之前的名字
 * @param       path2 : 新名字
 * @retval      无
 */
void renameFile(fs::FS &fs, const char * path1, const char * path2)
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) 
    {
        Serial.println("File renamed");
    } 
    else 
    {
        Serial.println("Rename failed");
    }
}

/**
 * @brief       删除文件/目录
 * @param       fs : 文件对象
 * @param       path : 文件/目录路径+名字
 * @retval      无
 */
void deleteFile(fs::FS &fs, const char * path)
{
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path))
    {
        Serial.println("File deleted");
    } 
    else 
    {
        Serial.println("Delete failed");
    }
}

/**
 * @brief       测试文件IO性能
 * @param       fs : 文件对象
 * @param       path : 文件/目录路径+名字
 * @retval      无
 */
void testFileIO(fs::FS &fs, const char * path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;

    if(file)
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len)
        {
            size_t toRead = len;
            if(toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } 
    else 
    {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i = 0; i < 2048; i++)
    {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}
