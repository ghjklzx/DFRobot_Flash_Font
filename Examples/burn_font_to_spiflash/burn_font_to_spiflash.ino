/*!
 * @file burn_font_to_spiflash.ino
 * @brief 从M0内置的U盘烧录字模文件到M0的字模文件区
 * @n 实验现象：通过U盘烧录字模，在串口中显示烧录进度
 * @copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence     The MIT License (MIT)
 * @author [guo](xinyu.guo@dfrobot.com)
 * @version  V1.0
 * @date  2020-02-24
 * @https:https://github.com/ghjklzx/DFRobot_Flash_Font
 */
#include <W25QSPIFlash.h>
#include <SPI.h>
#include <UD.h>
#include <DFRobot_Flash_Font.h>
#define MAXBUFSIZE 1024
#define HEAD_ADDRESS 0xc00000 

DFRobot_Flash_Font font;
File resourceFile;
uint32_t fileSize; // font library file total size

void eraseSpace(uint32_t fontAddress) {
  pinMode(CS, OUTPUT);
  SerialUSB.println("Preparing font space in flash for font library");
  SerialUSB.println("This will erase the font space memory form flash, plaese wait...\n");
  SerialUSB.println("-----------------------Erase Space-------------------------");
  SerialUSB.print("Erasing | "); 
  for (int count = 0; count < 512; count++) {
    W25Q.eraseSector(fontAddress);
    if (count != 0 && count % 11 == 0)
    SerialUSB.print("▋");
    fontAddress += 4096;
  }
  SerialUSB.println(" | 100%");
  SerialUSB.println("-----------------------------------------------------------\n");
  SerialUSB.println("font space prepared. ");
}

void SDInit() {
  SerialUSB.print("Initializing SD card...");
  if (!SD.begin(CS)) {
    SerialUSB.println("initialization failed!");
    while (1);
  }
  SerialUSB.println("initialization done.");
  resourceFile = SD.open("Noto.xbf");

  if (resourceFile) {
    fileSize = resourceFile.size();
    SerialUSB.print("Ready to write to flash, total size: ");SerialUSB.println(fileSize);
  } else {
    SerialUSB.println("error opening Noto.xbf, please check connection or file...");
    while (1);
  }
  SerialUSB.println();
}

void burnFontLib(uint32_t fontAddress) {
  uint8_t buf[MAXBUFSIZE];
  int count = 0;
  uint32_t rest = fileSize;
  uint16_t len;
  
  SerialUSB.println("--------------------------Burn Font----------------------------");
  SerialUSB.print("Burning | ");
  
  while(rest) {
    if (rest > MAXBUFSIZE)
      len = MAXBUFSIZE;
    else
      len = rest;
      
    resourceFile.read(buf, len);
    W25Q.write(fontAddress, buf, len);
    
    rest -= len;
    fontAddress += len;
    if (resourceFile.position() == (fileSize - rest)) {
      if ((fileSize - rest) >= (count * fileSize / 50)) {
        SerialUSB.print("▋");
        count++;
      }
    } else {break;}
  }
  if (resourceFile.position() == fileSize) {
    SerialUSB.println(" | 100%");
    SerialUSB.println("---------------------------------------------------------------");
    
    SerialUSB.println("\nFinish transfer. Now you can use the font Library form board.");
  } else {
    SerialUSB.println(" | failed");
    SerialUSB.println("---------------------------------------------------------------");
    SerialUSB.println("\nTransfer failed. please reset your board and transfer one more time!");
  }
  resourceFile.close();
}

void setup() {
  // Open SerialUSB communications and wait for port to open:
  SerialUSB.begin(115200);
  while (!SerialUSB); // wait for SerialUSB port to connect. Needed for native USB port only
  SPI.begin();
  eraseSpace(HEAD_ADDRESS);                         //擦除SPIFlash字模地址的空间
  SDInit();                                         //U盘初始化，读取Noto.xbf字模文件
  burnFontLib(HEAD_ADDRESS);                        //烧录Noto.xbf字模文件，在串口中显示进度条
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}
