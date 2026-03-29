/**
 ****************************************************************************************************
 * @file        spi_sdcard.cpp
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2023-12-01
 * @brief       SDCARD 驱动代码
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

#include "spi_sdcard.h"
#include <SD.h>
#include <SPI.h>
#include "myfs.h"

File myFile;
SPIClass spi_sdcard;
 
/**
 * @brief       初始化SD卡
 * @param       无
 * @retval      返回值:0 初始化正确；其他值，初始化错误
 */
uint8_t sdcard_init(void)
{
    spi_sdcard.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);    /* 设置SD卡的SPI用到的引脚 */ 

    pinMode(SD_CS_PIN, OUTPUT);                 /* 片选引脚设置为输出 */

    if (!SD.begin(SD_CS_PIN, spi_sdcard))       /* SD卡初始化 */
    {
        Serial.println("SD卡初始化失败!");

        if (!SD.begin(SD_CS_PIN, spi_sdcard))   /* SD卡初始化失败再次初始化 */
        {
            Serial.println("SD卡初始化失败!");
            return 1;
        }
    }
    Serial.println("SD卡初始化成功");

    uint8_t cardType = SD.cardType();   /* 获取卡的类型 */
    if (cardType == CARD_NONE)          /* 未连接存储卡 */
    {
        Serial.println("没有卡连接");
        return 2;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)           /* mmc卡 */
    {
        Serial.println("MMC");
    } 
    else if (cardType == CARD_SD)       /* sd卡，最大2G */
    {
        Serial.println("SDSC");
    } 
    else if (cardType == CARD_SDHC)     /* sdhc卡，最大32G */
    {
        Serial.println("SDHC");
    } 
    else                                /* 未知存储卡 */
    {
        Serial.println("UNKNOWN");
    }
    return 0;
}

/**
 * @brief       SD卡测试代码
 * @param       无
 * @retval      无
 */
void sd_test(void)
{
    myFile = SD.open("/test.txt", FILE_WRITE);    /* 打开创建文件 */
    if (myFile)                                   /* 文件成功被打开 */
    {
        Serial.print("Writing to test.txt...");
        myFile.println("testing 1, 2, 3.");       /* 写数据到文件中 */

        myFile.close();                           /* 关闭文件 */
        Serial.println("done.");
    } 
    else                                          /* 文件打开失败 */
    {
        Serial.println("error opening test.txt");
    }

    myFile = SD.open("/test.txt");               /* 打开前面操作的文件 */
    if (myFile) 
    {
        Serial.println("test.txt:");

        while (myFile.available())                  
        {
            Serial.write(myFile.read());          /* 读取文件中全部内容 */
        }

        myFile.close();                           /* 关闭文件 */
    } 
    else                                          /* 文件打开失败 */
    {
        Serial.println("error opening test.txt");
    }

    listDir(SD, "/", 0);                          /* 遍历目录下的文件 */
    createDir(SD, "/mydir");                      /* 创建文件夹 */
    listDir(SD, "/", 0);                          /* 遍历目录下的文件 */
    removeDir(SD, "/mydir");                      /* 移除文件夹 */
    listDir(SD, "/", 2);                          /* 遍历目录下的文件 */
    writeFile(SD, "/hello.txt", "Hello ");        /* 写数据到文件中 */
    appendFile(SD, "/hello.txt", "World!\n");     /* 在文件后追加数据 */
    readFile(SD, "/hello.txt");                   /* 从文件中读取数据 */
    deleteFile(SD, "/foo.txt");                   /* 删除文件 */
    renameFile(SD, "/hello.txt", "/foo.txt");     /* 修改文件名字 */
    readFile(SD, "/foo.txt");                     /* 从文件中读取数据 */ 
    testFileIO(SD, "/test.txt");                  /* 测试文件IO性能 */
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));  /* 打印SD卡文件系统总容量大小 */
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));    /* 打印SD卡文件系统已用容量大小 */
}
