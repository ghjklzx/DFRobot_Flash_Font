#include <DFRobot_Flash_Font.h>

DFRobot_Flash_Font::DFRobot_Flash_Font() {}

void DFRobot_Flash_Font::begin(void)
{
	pinMode(CS, OUTPUT); //设置SPI FLASH的片选引脚为输出模式
	W25Q.read( HEAD_ADDRESS, &uniInfo , FONT_INFO_BYTES );//读取字模文件头部信息
}

void DFRobot_Flash_Font::cache(String &s)
{
	_s = s;
	_utf8 = (const uint8_t *)_s.c_str();
	DBG(*_utf8);
	DBG("_utf8[_index]=");DBG(_utf8[_index]);
	_len = _s.length();
}

uint32_t DFRobot_Flash_Font::utf8Get()
{
	DBG(_index);
	DBG("_utf8[_index]=");DBG(_utf8[_index]);
	uint32_t uni=0;
	if ( _utf8[_index] == 0 || _utf8[_index] == '\n' )  /* '\n' terminates the string to support the string list procedures */
	{
		uni=0xff;
		_index++;
		DBG(_index);
		DBG(uni);
		return uni;
		
	} 
	uint8_t utf8State = 0;
	if(_utf8[_index] >= 0xfc)
		{
			utf8State = 5;
			uni = _utf8[_index]&1;
			_index++;
			for(uint8_t i=1;i<=5;i++)
			{
				uni <<= 6;
				uni |= (_utf8[_index]&0x3f);
				utf8State--;
				_index++;
			}
			DBG(_index);DBG(uni);
			return uni;
		}else if(_utf8[_index] >= 0xf8)
		{
			utf8State = 4;
			uni = _utf8[_index]&3;
			_index++;
			for(uint8_t i=1;i<=4;i++)
			{
				uni <<= 6;
				uni |= (_utf8[_index]& 0x03f);
				utf8State--;
				_index++;
			}
			DBG(_index);DBG(uni);
			return uni;
			
		}else if(_utf8[_index] >= 0xf0)
		{
			utf8State = 3;
			uni = _utf8[_index]&7;
			_index++;
			for(uint8_t i=1;i<=3;i++)
			{
				uni <<= 6;
				uni |= (_utf8[_index]& 0x03f);
				utf8State--;
				_index++;
				DBG(_index);
			}
			DBG(_index);DBG(uni);
			return uni;
			
		}else if(_utf8[_index] >= 0xe0)
		{
			utf8State = 2;
			DBG("index=");DBG(_index);
			uni = _utf8[_index]&15;
			_index++;
			DBG("uni=");DBG(uni);
			for(uint8_t i=1;i<=2;i++)
			{
				uni <<= 6;
				uni |= (_utf8[_index]&0x03f);
				utf8State--;
				_index++;
				DBG("uni=");DBG(uni);
				DBG("index=");DBG(_index);
			}
			DBG(_index);DBG(uni);
			return uni;	
		}else if(_utf8[_index] >= 0xc0)
		{
			utf8State = 1;
			uni = _utf8[_index]&0x1f;
			_index++;
			for(uint8_t i=1;i<=1;i++)
			{
				uni <<= 6;
				uni |= (_utf8[_index]& 0x03f);
				utf8State--;
				_index++;
				DBG(_index);
			}
			DBG(_index);DBG(uni);
			return uni;
		}else if(_utf8[_index] <=0x80)
		{
			uni |= (_utf8[_index]&0x7f);
			_index++;
			DBG("index")DBG(_index);
			DBG("uni");DBG(uni);
			return uni;
		}
}

bool DFRobot_Flash_Font::avaible()
{
	DBG(_len);
	DBG(_index);
	DBG("_utf8[_index]=");DBG(_utf8[_index]);
	return !(_len ==_index);
}

bool DFRobot_Flash_Font::getFont(uint32_t uni,uint8_t *buf, uint8_t& width, uint8_t& len,uint8_t& bytePerLine)
{
	uint32_t address;
	if (uni > (uniInfo.lastChar) || uni < (uniInfo.firstChar))
	return false;
	address = HEAD_ADDRESS + FONT_INFO_BYTES + uni * 6;
	DBG();DBG(address);
	charInfo_t charInfo;
	W25Q.read(address, &charInfo, CHAR_ADDRESS_AND_BYTES_LEN);
	DBG(charInfo.ptrCharData);
	delay(10);
	if (charInfo.ptrCharData == 0 || charInfo.len == 0)
	return false;
	charSpec_t charSpec;
	address = (charInfo.ptrCharData) + HEAD_ADDRESS;
	W25Q.read(address, &charSpec, CHAR_WIGTH_AND_BYTE_PER_LINE);
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
