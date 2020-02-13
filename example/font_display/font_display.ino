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
  uint8_t buf;
  String string1 = "abc"; 
  Flash_font.printString(string1);
  delay(1000);
  while(1);
}
