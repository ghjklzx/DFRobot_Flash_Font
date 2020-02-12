#include <DFRobot_M0Font.h>

DFRobot_M0Font::DFRobot_M0Font() {
}

void DFRobot_M0Font::begin()
{
	// uint8_t buf[FONT_INFO_BYTES]; 
  pinMode(CS, OUTPUT); //设置SPI FLASH的片选引脚为输出模式
  SPI.begin();

	W25Q16_Read(HEAD_ADDRESS, &uniInfo , FONT_INFO_BYTES);
  //SerialUSB.println(uniInfo.lastChar);
}

uint8_t DFRobot_M0Font::W25Q16_BUSY(void) {//判断W25Q16是否繁忙函数 繁忙则返回1
    uint8_t flag;
    CS_L;
    SPI.transfer(0x05);
    flag=SPI.transfer(0xFF);
    CS_H;
    flag&=0x01;
    return flag;
}

void DFRobot_M0Font::W25Q16_Read(uint32_t address,void *data, uint16_t j) {
  // 中
    uint16_t i;
    uint8_t *p = (uint8_t *)data;
    while(W25Q16_BUSY());
    CS_L;
    SPI.transfer(0x03);
    SPI.transfer(address>>16);
    SPI.transfer(address>>8);
    SPI.transfer(address);
    for(i=0;i<j;i++)
    {
        *(p+i)=SPI.transfer(0xFF);
    }
    CS_H;
}

uint16_t DFRobot_M0Font::u8x8_utf8_next(uint8_t b) {
  if ( b == 0 || b == '\n' )  /* '\n' terminates the string to support the string list procedures */
    return 0x0ffff; /* end of string detected, pending UTF8 is discarded */
  if ( utf8State == 0 ){
    if ( b >= 0xfc )  /* 6 byte sequence */{
      utf8State = 5;
      b &= 1;
    }else if ( b >= 0xf8 ){
      utf8State = 4;
      b &= 3;
    }else if ( b >= 0xf0 ){
      utf8State = 3;
      b &= 7;      
    }else if ( b >= 0xe0 ){
      utf8State = 2;
      b &= 15;
    }else if ( b >= 0xc0 ){
      utf8State = 1;
      b &= 0x01f;
    }else{/* do nothing, just use the value as encoding */
      return b;
    }
    encoding = b;
    return 0x0fffe;
  }else{
    utf8State--;
    /* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
    encoding<<=6;
    b &= 0x03f;
    encoding |= b; 
    if ( utf8State != 0 )
      return 0x0fffe; /* nothing to do yet */
  }
  return encoding;
}

void DFRobot_M0Font::printString(const String &string)  {
  int len = string.length();
  SerialUSB.print("length=");SerialUSB.println(len);
  const char *buffer1 = string.c_str();
  const uint8_t *buffer2 = (const uint8_t*)buffer1;
  //pos_x = x;
  //pos_y = y;
  uint16_t ucode;
  for (int i = 0; i < len ; i++) {
    //x = pos_x;
    //y = pos_y;
    ucode = u8x8_utf8_next(buffer2[i]);
    if(ucode <0x0fffe) {
      SerialUSB.print("unicode=");SerialUSB.println(ucode);
      //drawStringMap(ucode, x, y);
      drawStringMap(ucode);
    }
  }
}

void DFRobot_M0Font::drawStringMap(uint16_t uni) {
  SerialUSB.print(uniInfo.firstChar);
  SerialUSB.print(" ");
  SerialUSB.print(uniInfo.lastChar);
  // SerialUSB.println(&uniInfo);
  if (uni > (uniInfo.lastChar) || uni < (uniInfo.firstChar))
    return;
    
  uint32_t charInfoAddress = HEAD_ADDRESS + 18 + uni * 6;
  SerialUSB.print("charInfoAddress=");SerialUSB.println(charInfoAddress);
  
  charInfo_t *charInfo;
  W25Q16_Read(charInfoAddress, (uint8_t*)charInfo, CHAR_ADDRESS_AND_BYTES_LEN);
  
  if (charInfo->ptrCharData == 0 || charInfo->len == 0)
    return;

  charSpec_t *charSpec;
  address = (charInfo->ptrCharData) + HEAD_ADDRESS;
  W25Q16_Read(address, (uint8_t*)charSpec, CHAR_WIGTH_AND_BYTE_PER_LINE);
  
  uint8_t charBufLen = (charInfo->len) - CHAR_WIGTH_AND_BYTE_PER_LINE;
  uint8_t charBuf[charBufLen];
  address = address + CHAR_WIGTH_AND_BYTE_PER_LINE;
  W25Q16_Read(address, charBuf, charBufLen);

  int count = 0;
  //uint16_t px = x;
  //uint16_t py = y;
  int w = 0;
  for (int i = 0; i < charBufLen; i++){
    uint8_t mask = charBuf[i];
    int n = 8;
      if ((w + 8) > charSpec->width)
        n = charSpec->width - w;
      for (int p = 0; p < n; p++){
        int c = 0;
        if (mask & 0x80) {
          c = 1;
        } else {
          c = 0;
        }
        if (c) {
      SerialUSB.print('█');
    }
        // += 1;
        mask = mask << 1;
      }
      w += 8;
    count += 1;
    if (count == charSpec->bytePerLine) {
      //py += 1;
      // = x;
      count = 0;
      w = 0;
    SerialUSB.println();
    }
  }
  //pos_x = pos_x + (charSpec->width) + 1;
}

void DFRobot_M0Font::eraseSpace(void) {
  SerialUSB.println("Preparing specific space in flash for font library");
  SerialUSB.println("This will erase the specific space memory form flash, plaese wait...\n");
  SerialUSB.print("Preparing | ");
  for (; count < 512; count++) {
	EraseSector(address);
	if (count != 0 && count%10 == 0) {SerialUSB.print("#");}
	address += 4096;
  }
  SerialUSB.println(" | 100%\n");
  SerialUSB.println("Specific space prepared. ");
  count = 1;
  address = HEAD_ADDRESS;
}

void DFRobot_M0Font::transport(void){
    if ((transSize + MAXBUFSIZE) <= fileSize) {
    resourceFile.read(buf, MAXBUFSIZE);
    //targetFile.write(buf, MAXBUFSIZE);
	W25Q16_Write(address, buf, MAXBUFSIZE);
    transSize += MAXBUFSIZE;
	address += MAXBUFSIZE;
    if (transSize >= (count*fileSize/50)) {
      SerialUSB.print("#");
      count++;
    }
  } else {
    uint16_t rest;
    rest = fileSize - transSize;
    resourceFile.read(buf, rest);
    //targetFile.write(buf, rest);
	W25Q16_Write(address, buf, rest);
  }
  if (resourceFile.position() == fileSize) 
  {
	SerialUSB.println("# | 100%");
	SerialUSB.println("--------------------------------------------------------------------");
	resourceFile.close();
	//targetFile.close();
	SerialUSB.println("\nFinish transfer. Now you can use the font Library form board.");
	while (1);
  }
}

void DFRobot_M0Font::EraseSector(uint32_t address) {
    while(W25Q16_BUSY());
    Write_Enable();                              
    CS_L;                 //置cs低选中
    SPI.transfer(0x20);
    SPI.transfer(address>>16);
    SPI.transfer(address>>8);
    SPI.transfer(address);
    CS_H;
  
}

void DFRobot_M0Font::W25Q16_Write(uint32_t address,const uint8_t *data,uint16_t left) {
    uint16_t max;
	while(left) {
      while(W25Q16_BUSY());//如果芯片繁忙就等在这里
  //    EraseSector(address);
      Write_Enable();//要先写入允许命令
      CS_L;
      SPI.transfer(0x02);
      SPI.transfer(address>>16);
      SPI.transfer(address>>8);
      SPI.transfer(address);
	  if(left>256)
		max = 256;
	  else 
		max = left;
      for(uint16_t i=0;i<max;i++)
      {
        SPI.transfer(*(data+i));
      }
      CS_H;
	  address += max;
	  data += max;
	  left -= max;
	}  
}

void DFRobot_M0Font::Write_Enable(void) { //写使能函数 对W25Q16进行写操作之前要进行这一步操作
    CS_L;
    SPI.transfer(0x06);
    CS_H;
}