#ifndef __DFRobot_M0Font_H
#define __DFRobot_M0Font_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <SPI.h>
#include <UD.h>
#define HEAD_ADDRESS 0xc00000//SPIflash首地址
#define MAXBUFSIZE 1024//

#define CS 32//SPIflash片选引脚
#define CS_H  digitalWrite(CS, HIGH)
#define CS_L  digitalWrite(CS, LOW)

#define FONT_INFO_BYTES                18//单个字模大小
#define CHAR_ADDRESS_AND_BYTES_LEN     6//
#define CHAR_WIGTH_AND_BYTE_PER_LINE   4//
#endif

class DFRobot_M0Font
{
public:
    typedef struct 
    {
    char title[4];//juix
    uint16_t height;//16
    uint16_t width;//16
    uint16_t baseline;//12
    uint16_t xHeight;
    uint16_t yHeight;
    uint16_t firstChar;//0
    uint16_t lastChar; //65535
    } __attribute__ ((packed)) uniInfo_t;//所有字符的信息

    typedef struct {
    uint32_t ptrCharData;
    uint16_t len;
    } __attribute__ ((packed)) charInfo_t;//字符信息

    typedef struct {
    uint16_t width;
    uint16_t bytePerLine;
    } __attribute__ ((packed)) charSpec_t;//字符信息
	
	uniInfo_t uniInfo;
    //uint16_t pos_x = 0;
    //uint16_t pos_y = 0;

    uint8_t buf[MAXBUFSIZE];//
    uint32_t transSize = 0;
    uint32_t fileSize;
    File resourceFile;

public:
    DFRobot_M0Font();

	void begin(void);

    uint16_t u8x8_utf8_next (uint8_t b);

    void printString(const String &string) ;
    
	void drawStringMap(uint16_t uni);

    void eraseSpace (void);
    
    void transport (void);

    void EraseSector (uint32_t address);

private:
    uint16_t utf8State = 0;
    uint16_t encoding = 0 ;
	uint32_t address = HEAD_ADDRESS;
    int count = 0;
    

private:

	uint8_t W25Q16_BUSY(void);
    void Write_Enable(void);
    void W25Q16_Read (uint32_t address,void* data, uint16_t j);
    void W25Q16_Write(uint32_t address,const uint8_t *data,uint16_t left);
    
    
};
