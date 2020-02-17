#include <Flash_Font.h>

Flash_Font::Flash_Font() {}

void Flash_Font::begin(void)
{
    pinMode(CS, OUTPUT); //设置SPI FLASH的片选引脚为输出模式
	W25Q.read( HEAD_ADDRESS, &uniInfo , FONT_INFO_BYTES );//读取字模文件头部信息
}

uint16_t Flash_Font::utf8(uint8_t b) 
{
	if ( b == 0 || b == '\n' )  /* '\n' terminates the string to support the string list procedures */
	return 0x0ffff; /* end of string detected, pending UTF8 is discarded */
	if ( utf8State == 0 ) {
		if ( b >= 0xfc ) {/* 6 byte sequence */
		    utf8State = 5;
			b &= 1;
		} else if ( b >= 0xf8 ) {
			utf8State = 4;
			b &= 3;
		} else if ( b >= 0xf0 ) {
			utf8State = 3;
			b &= 7;      
		} else if ( b >= 0xe0 ) {
			utf8State = 2;
			b &= 15;
		} else if ( b >= 0xc0 ) {
			utf8State = 1;
			b &= 0x01f;
		} else {/* do nothing, just use the value as encoding */
			return b;
		}
		encoding = b;
		return 0x0fffe;
	} else {
		utf8State--;
		/* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
		encoding <<= 6;
		b &= 0x03f;
		encoding |= b; 
		if ( utf8State != 0 )
			return 0x0fffe; /* nothing to do yet */
	}
	return encoding;
}

// void Flash_Font::printString(const String &string) 
// {
// 	uint16_t buf;
// 	int len = string.length();
// 	SerialUSB.print("length=");SerialUSB.println(len);
// 	const uint8_t *buffer2 = (const uint8_t *)string.c_str();
// 	uint16_t ucode;
// 	for (int i = 0; i < len; i++) 
// 	{
// 		ucode = utf8(buffer2[i]);
// 		if(ucode <0x0fffe) 
// 		{
// 			SerialUSB.print("unicode=");SerialUSB.println(ucode);
// 			drawStringMap(ucode);
// 		}
// 	}
// }
void Flash_Font::printString(const String &string)
{

	int len = string.length();
	const uint8_t *buffer2 = (const uint8_t *)string.c_str();
	uint16_t ucode;
	uint8_t *charBuf;
	for (int i = 0; i < len; i++) 
	{
		ucode = utf8(buffer2[i]);
		if(ucode <0x0fffe) 
		{
			uint8_t *charBuf = getFont(ucode);
			drawStringMap(charBuf);
		}
	}
}

uint8_t Flash_Font::getFont(uint16_t uni)
{	uint8_t buf;
    if (uni > (uniInfo.lastChar) || uni < (uniInfo.firstChar))
		return &buf;

	address = HEAD_ADDRESS + FONT_INFO_BYTES + uni * 6;
	// SerialUSB.print("address=");SerialUSB.println(address);
	
	charInfo_t charInfo;
	W25Q.read(address, &charInfo, CHAR_ADDRESS_AND_BYTES_LEN);
	delay(10);
	// SerialUSB.print("len");SerialUSB.print(" ");SerialUSB.print(charInfo.len);SerialUSB.print(" ");SerialUSB.print(" ");
	// SerialUSB.print("ptrCharData");SerialUSB.print(" ");SerialUSB.println(charInfo.ptrCharData);
	if (charInfo.ptrCharData == 0 || charInfo.len == 0)
		return &buf;

	charSpec_t charSpec;
	address = (charInfo.ptrCharData) + HEAD_ADDRESS;
	W25Q.read(address, &charSpec, CHAR_WIGTH_AND_BYTE_PER_LINE);
	delay(10);
	// SerialUSB.print("width");SerialUSB.print(" ");SerialUSB.print(charSpec.width);SerialUSB.print(" ");SerialUSB.print(" ");
	// SerialUSB.print("bytePerLine");SerialUSB.print(" ");SerialUSB.println(charSpec.bytePerLine);

	uint8_t charBufLen = (charInfo.len) - CHAR_WIGTH_AND_BYTE_PER_LINE;
	uint8_t charBuf[charBufLen];
	address = address + CHAR_WIGTH_AND_BYTE_PER_LINE;
	W25Q.read(address, charBuf, charBufLen);
	return charBuf;
}

void Flash_Font::drawStringMap(uint8_t &charBuf) 
{
	int count = 0;
	int w = 0;
	for (int i = 0; i < charBufLen; i++) {
		uint8_t mask = charBuf[i];
		int n = 8;
		if ((w + 8) > charSpec.width)
			n = charSpec.width - w;
		for (int p = 0; p < n; p++) {
			if (mask & 0x80) SerialUSB.print("■");
			else SerialUSB.print("□");
			mask <<= 1;
		}
		w += 8;
		count++;
		if (count == charSpec.bytePerLine) {
			count = 0;
			w = 0;
			SerialUSB.println();
		}
	}
	SerialUSB.println();
}

void Flash_Font::eraseSpace(void) 
{
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

void Flash_Font::SDInit(void) 
{
	pinMode(CS, OUTPUT);
	SerialUSB.print("Initializing SD card...");
	if (!SD.begin(CS)) {
		SerialUSB.println("initialization failed!");
		while (1);
	}
	SerialUSB.println("initialization done.");
	File_f = SD.open("Noto.xbf");

	if (File_f) {
		fileSize = File_f.size();
		SerialUSB.print("Ready to write to flash, total size: ");
		SerialUSB.println(fileSize);
	} else {
		SerialUSB.println("error opening Noto.xbf, please check connection or file...");
		while (1);
	}
	SerialUSB.println();
}

void Flash_Font::burnFontLib(uint32_t address,uint32_t MAXBUFSIZE) 
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
			
		File_f.read(buf, len);
		W25Q.write(address, buf, len);
		
		rest -= len;
		address += len;
		if (File_f.position() == (fileSize - rest)) {
			if ((fileSize - rest) >= (count * fileSize / 50)) {
				SerialUSB.print("▋");
				count++;
			}
		} else {break;}
	}
	if (File_f.position() == fileSize) {
		SerialUSB.println(" | 100%");
		SerialUSB.println("---------------------------------------------------------------");
		
		SerialUSB.println("\nFinish transfer. Now you can use the font Library form board.");
	} else {
		SerialUSB.println(" | failed");
		SerialUSB.println("---------------------------------------------------------------");
		SerialUSB.println("\nTransfer failed. please reset your board and transfer one more time!");
	}
	File_f.close();
}
