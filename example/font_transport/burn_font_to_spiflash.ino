#include <Flash_Font.h>
#define MAXBUFSIZE 1024

Flash_Font Flash_font;

void setup() {
  // Open SerialUSB communications and wait for port to open:
  SerialUSB.begin(115200);
  while (!SerialUSB); // wait for SerialUSB port to connect. Needed for native USB port only
  
  SPI.begin();
  
  //eraseSpace(HEAD_ADDRESS);
  Flash_font.SDInit();
  Flash_font.burnFontLib(HEAD_ADDRESS,MAXBUFSIZE);
  
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
}
