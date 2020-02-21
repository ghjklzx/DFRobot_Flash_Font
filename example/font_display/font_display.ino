#include <DFRobot_Flash_Font.h>
DFRobot_Flash_Font font;

void drawStringMap(uint8_t *charBuf,uint8_t wid, uint8_t len,uint8_t bytePerLine ) 
{
    int count = 0;
    int w = 0;
    for (int i = 0; i < len; i++) 
    {
      uint8_t mask = charBuf[i];
      int n = 8;
      if ((w + 8) > wid)
        n =wid - w;
      for (int p = 0; p < n; p++) {
      if (mask & 0x80) SerialUSB.print("■");
        else SerialUSB.print("□");
        mask <<= 1;
      }
      w += 8;
      count++;
      if (count == bytePerLine) 
      {
        count = 0;
        w = 0;
        SerialUSB.println();
      }
     }
  SerialUSB.println();
}

void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(115200);
  SPI.begin();
  delay(300);
  font.begin(); 
  delay(10);
  while (!SerialUSB); // wait for SerialUSB port to connect. Needed for native USB port only
}

void loop() {
    String string1 = "你好";
    String string2 = "成都";
    uint8_t charBuf[32];
    uint8_t width,lenth,bytePerLine;
    font.cache(string2);
    ucode = font.utf8Get();
    font.cache(string1);
    while(font.avaible())
    {
      uint32_t ucode;
      ucode = font.readUni();
      font.getFont(ucode,charBuf,width,lenth,bytePerLine);//结构体
      drawStringMap(charBuf,width,lenth,bytePerLine);
     };
     SerialUSB.print("loop");
    while(1);
}
