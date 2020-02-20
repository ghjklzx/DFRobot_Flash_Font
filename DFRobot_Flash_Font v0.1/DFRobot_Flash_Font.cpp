#include <DFRobot_Flash_Font.h>

DFRobot_Flash_Font::DFRobot_Flash_Font() {}

void DFRobot_Flash_Font::begin(void)
{
    pinMode(CS, OUTPUT); //设置SPI FLASH的片选引脚为输出模式
	W25Q.read( HEAD_ADDRESS, &uniInfo , FONT_INFO_BYTES );//读取字模文件头部信息
}

void DFRobot_Flash_Font::transfer()
{
	
}

uint16_t DFRobot_Flash_Font::utf8Trans(uint8_t utf8) 
{
	if ( utf8 == 0 || utf8 == '\n' )  /* '\n' terminates the string to support the string list procedures */
	return 0x0ffff; /* end of string detected, pending UTF8 is discarded */
	if ( utf8State == 0 ) {
		if ( utf8 >= 0xfc ) {/* 6 byte sequence */
		    utf8State = 5;
			utf8 &= 1;
		} else if ( utf8 >= 0xf8 ) {
			utf8State = 4;
			utf8 &= 3;
		} else if ( utf8 >= 0xf0 ) {
			utf8State = 3;
			utf8 &= 7;      
		} else if ( utf8 >= 0xe0 ) {
			utf8State = 2;
			utf8 &= 15;
		} else if ( utf8 >= 0xc0 ) {
			utf8State = 1;
			utf8 &= 0x01f;
		} else {/* do nothing, just use the value as encoding */
			return utf8;
		}
		encoding = utf8;
		return 0x0fffe;
	} else {
		utf8State--;
		/* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
		encoding <<= 6;
		utf8 &= 0x03f;
		encoding |= utf8; 
		if ( utf8State != 0 )
			return 0x0fffe; /* nothing to do yet */
	}
	return encoding;
}



void DFRobot_Flash_Font::printString(const String &string)
{
	int len = string.length();
	const uint8_t *buffer2 = (const uint8_t *)string.c_str();
	uint16_t ucode;
	uint8_t charBuf[32];
	uint8_t width,lenth,bytePerLine;
	for (int i = 0; i < len; i++) 
	{
		DBG("len");DBG(len);
		ucode = utf8Trans(buffer2[i]);
		if(ucode <0x0fffe) 
		{
			getFont(ucode,charBuf,width,lenth,bytePerLine);
			DBG("charBuf=");DBG(charBuf[0]);
			drawStringMap(charBuf,width,lenth,bytePerLine);
		}
	}
}

bool DFRobot_Flash_Font::getFont(uint16_t uni,uint8_t *buf, uint8_t& width, uint8_t& len,uint8_t& bytePerLine)
{
	DBG(uni);
	uint32_t address;
    if (uni > (uniInfo.lastChar) || uni < (uniInfo.firstChar))
		return false;

	address = HEAD_ADDRESS + FONT_INFO_BYTES + uni * 6;
	DBG(address);
	
	charInfo_t charInfo;
	W25Q.read(address, &charInfo, CHAR_ADDRESS_AND_BYTES_LEN);
	DBG(charInfo.ptrCharData);
	delay(10);
	if (charInfo.ptrCharData == 0 || charInfo.len == 0)
		return false;

	charSpec_t charSpec;
	address = (charInfo.ptrCharData) + HEAD_ADDRESS;
	W25Q.read(address, &charSpec, CHAR_WIGTH_AND_BYTE_PER_LINE);
	DBG(charSpec.width);
	delay(10);

	uint8_t charBufLen = (charInfo.len) - CHAR_WIGTH_AND_BYTE_PER_LINE;
	address = address + CHAR_WIGTH_AND_BYTE_PER_LINE;
	W25Q.read(address, buf, charBufLen);
	
	//判断是否成功赋值
	DBG(charSpec.width);
	width = charSpec.width;
	len = charBufLen;
	bytePerLine = charSpec.bytePerLine;
	DBG("width=");DBG(width);
	DBG("len=");DBG(len);
	DBG("bytePerLine");DBG(bytePerLine);
	return true;
}

void DFRobot_Flash_Font::drawStringMap(uint8_t *charBuf,uint8_t width, uint8_t len,uint8_t bytePerLine ) 
{
	DBG(charBuf[0]);
	DBG(width);
	DBG(len);
	DBG(bytePerLine);
	int count = 0;
	int w = 0;
	for (int i = 0; i < len; i++) {
		uint8_t mask = charBuf[i];
		int n = 8;
		if ((w + 8) > width)
			n =width - w;
		for (int p = 0; p < n; p++) {
			if (mask & 0x80) SerialUSB.print("■");
			else SerialUSB.print("□");
			mask <<= 1;
		}
		w += 8;
		count++;
		if (count == bytePerLine) {
			count = 0;
			w = 0;
			SerialUSB.println();
		}
	}
	SerialUSB.println();
	
}

void DFRobot_Flash_Font::eraseSpace(void) 
{	uint32_t address=HEAD_ADDRESS;
 	pinMode(CS, OUTPUT);
	SerialUSB.println("Preparing font space in flash for font library");
	SerialUSB.println("This will erase the font space memory form flash, plaese wait...\n");
	SerialUSB.println("-----------------------Erase Space-------------------------");
	SerialUSB.print("Erasing | ");
	
	for (int count = 0; count < 512; count++) {
		W25Q.eraseSector(address);
		if (count != 0 && count % 11 == 0)
			SerialUSB.print("▋");
		address += 4096;
	}
	
	SerialUSB.println(" | 100%");
	SerialUSB.println("-----------------------------------------------------------\n");
	SerialUSB.println("font space prepared. ");
}

void DFRobot_Flash_Font::SDInit(void) 
{
	pinMode(CS, OUTPUT);
	SerialUSB.print("Initializing SD card...");
	if (!SD.begin(CS)) {
		SerialUSB.println("initialization failed!");
		while (1);
	}
	SerialUSB.println("initialization done.");
	fileData = SD.open("Noto.xbf");

	if (fileData) {
		fileSize = fileData.size();
		SerialUSB.print("Ready to write to flash, total size: ");
		SerialUSB.println(fileSize);
	} else {
		SerialUSB.println("error opening Noto.xbf, please check connection or file...");
		while (1);
	}
	SerialUSB.println();
}

void DFRobot_Flash_Font::burnFontLib(uint32_t address,uint32_t MAXBUFSIZE) 
{
	uint8_t buf[MAXBUFSIZE];
	int count = 0;
	uint32_t rest = fileSize;
	uint16_t len;
	
	pinMode(CS, OUTPUT);
	SerialUSB.println("--------------------------Burn Font----------------------------");
	SerialUSB.print("Burning | ");
	
	while(rest) {
		if (rest > MAXBUFSIZE)
			len = MAXBUFSIZE;
		else
			len = rest;
			
		fileData.read(buf, len);
		W25Q.write(address, buf, len);
		
		rest -= len;
		address += len;
		if (fileData.position() == (fileSize - rest)) {
			if ((fileSize - rest) >= (count * fileSize / 50)) {
				SerialUSB.print("▋");
				count++;
			}
		} else {break;}
	}
	if (fileData.position() == fileSize) {
		SerialUSB.println(" | 100%");
		SerialUSB.println("---------------------------------------------------------------");
		
		SerialUSB.println("\nFinish transfer. Now you can use the font Library form board.");
	} else {
		SerialUSB.println(" | failed");
		SerialUSB.println("---------------------------------------------------------------");
		SerialUSB.println("\nTransfer failed. please reset your board and transfer one more time!");
	}
	fileData.close();
}
