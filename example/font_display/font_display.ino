#include <Flash_Font.h>
Flash_Font Flash_font;
void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(115200);
  SPI.begin();
  delay(3000);
  Flash_font.begin(); 
  delay(10);
  while (!SerialUSB); // wait for SerialUSB port to connect. Needed for native USB port only
}

void loop() {
  String string1 = "abc"; 
  uint8_t len = string1.length();
  const uint8_t *fontstr = (const uint8_t *)string1.c_str();
  uint8_t *buf[len];
  uint16_t ucode;
  for(int i=0;i<len;i++)
  {
    ucode = Flash_font.u8x8_utf8_next(fontstr[i]);
    buf[i] = Flash_font.getFont(ucode);
    }
  
  Flash_font.printString(string1);
  delay(1000);
  while(1);
}
