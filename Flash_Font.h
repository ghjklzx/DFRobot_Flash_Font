#ifndef __Flash_Font_H
#define __Flash_Font_H


#include "Arduino.h"
#include <SPI.h>
#include <UD.h>
#include <W25QSPIFlash.h>

#define HEAD_ADDRESS 0xc00000//SPIflash首地址
// #define MAXBUFSIZE 1024//开在ino里

#define CS 32//SPIflash片选引脚
#define CS_H  digitalWrite(CS, HIGH)
#define CS_L  digitalWrite(CS, LOW)

#define FONT_INFO_BYTES                18//单个字模大小
#define CHAR_ADDRESS_AND_BYTES_LEN     6//
#define CHAR_WIGTH_AND_BYTE_PER_LINE   4//
#endif

class Flash_Font
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
    uint32_t transSize = 0;
    uint32_t fileSize;
    File File_f;

public:
    Flash_Font();

	void begin(void);

    uint16_t utf8 (uint8_t b);

    void printString(const String &string) ;

    uint8_t getFont(uint16_t uni);

	void drawStringMap(uint8_t *charBuf);

    void eraseSpace (void);

    void burnFontLib(uint32_t fontAddress,uint32_t MAXBUFSIZE);
    
    void SDInit(void);


private:
    uint16_t utf8State = 0;
    uint16_t encoding = 0 ;
	uint32_t address = HEAD_ADDRESS; 
};
