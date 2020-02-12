#include <DFRobot_M0Font.h>
DFRobot_M0Font m0_font;
void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(115200);
  SPI.begin();
  delay(3000);
  m0_font.begin(); 
  
  delay(10);
  while (!SerialUSB); // wait for SerialUSB port to connect. Needed for native USB port only
}

void loop() {
  String string1 = "abc"; 
  m0_font.printString(string1);
  delay(1000);
  while(1);
}
