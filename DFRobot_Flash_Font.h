#ifndef __DFRobot_Flash_Font_H
#define __DFRobot_Flash_Font_H

#include "Arduino.h"
#include <SPI.h>
#include <UD.h>
#include <W25QSPIFlash.h>

#define ENABLE_DBG
#ifdef ENABLE_DBG
#define DBG(...) {SerialUSB.print("[");SerialUSB.print(__FUNCTION__); SerialUSB.print("(): "); SerialUSB.print(__LINE__); SerialUSB.print(" ] "); SerialUSB.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

#define HEAD_ADDRESS 0xc00000//SPIflash首地址

#define CS 32//SPIflash片选引脚
#define CS_H  digitalWrite(CS, HIGH)
#define CS_L  digitalWrite(CS, LOW)

#define FONT_INFO_BYTES                18//单个字模大小
#define CHAR_ADDRESS_AND_BYTES_LEN     6//
#define CHAR_WIGTH_AND_BYTE_PER_LINE   4//
#endif

class DFRobot_Flash_Font
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
	File fileData;

public:
	DFRobot_Flash_Font();
	void begin(void);
	void cache(String &s);
	bool avaible(void);
	uint32_t utf8Get();
	uint16_t transfer(uint8_t utf8,uint16_t& index);
	bool getFont(uint32_t uni,uint8_t *buf, uint8_t& width, uint8_t& len,uint8_t& bytePerLine);
	void drawStringMap(uint8_t *charBuf,uint8_t width, uint8_t len,uint8_t bytePerLine);
	void eraseSpace(void);
	void burnFontLib(uint32_t fontAddress,uint32_t MAXBUFSIZE);
	void SDInit(void);
private:
	String _s;
	const uint8_t *_utf8;
	uint16_t _index=0;
	uint8_t _len=0;
};
