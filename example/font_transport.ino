#include <DFRobot_M0Font.h>
DFRobot_M0Font m0_font;

void setup() {
  // put your setup code here, to run once:
  SPI.begin();
  pinMode(CS, OUTPUT);
  // Open SerialUSB communications and wait for port to open:
  SerialUSB.begin(115200);
  while (!SerialUSB) {
    ; // wait for SerialUSB port to connect. Needed for native USB port only
  }
  
  m0_font.eraseSpace();
 
  SerialUSB.print("Initializing SD card...");
  if (!SD.begin(CS)) {
    SerialUSB.println("initialization failed!");
    while (1);
  }
  SerialUSB.println("initialization done.");
  m0_font.resourceFile = SD.open("Noto.xbf");
  
  //targetFile = SD.open("test.xbf", FILE_WRITE);

  if (m0_font.resourceFile) {
    SerialUSB.println("Read from Noto.xbf...");
    m0_font.fileSize = m0_font.resourceFile.size();
    SerialUSB.print("Ready to write to flash, total size: ");
    SerialUSB.println(m0_font.fileSize);
   } 
  else {
    SerialUSB.println("error opening Noto.xbf, please check connection or file...");
  while (1);
    }
  SerialUSB.println();
  SerialUSB.println("--------------------------starting write----------------------------");
  SerialUSB.print("Writing | ");
}

void loop() {
  // put your main code here, to run repeatedly:
   m0_font.transport();
}
