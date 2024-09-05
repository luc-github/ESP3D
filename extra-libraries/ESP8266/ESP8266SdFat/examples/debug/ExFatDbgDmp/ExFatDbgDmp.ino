#include "SdFat.h"
#define DUMP_RAW 0
#define DUMP_UPCASE 0
const uint8_t CS_PIN = SS;
//#define SD_CONFIG SdioConfig(FIFO_SDIO)
#define SD_CONFIG SdSpiConfig(CS_PIN)

SdExFat sd;
#define error(s) sd.errorHalt(&Serial, F(s))
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  Serial.println(F("Type any character to begin"));
  while (!Serial.available()) {
    yield();
  }
  if (!sd.begin(SD_CONFIG)){
    error("begin failed");
  }
#if DUMP_RAW
  sd.dmpSector(&Serial, 0);
  for (uint8_t i = 0; i < 24; i++) {
    sd.dmpSector(&Serial, 0X8000 + i);
    Serial.println();
  }
  return;
 #endif  // DUMP_RAW
 ExFatFile root;
 if (!root.openRoot(&sd)) {
  error("openRoot failed");
 }
  sd.printDir(&Serial, &root);
  // startSector = 0, sectorCount = 1.
  sd.dmpFat(&Serial, 0, 1);
  sd.dmpBitmap(&Serial);
  sd.printVolInfo(&Serial);

  sd.checkUpcase(&Serial);
#if DUMP_UPCASE
  sd.printUpcase(&Serial);
#endif  // DUMP_UPCASE
 // sd.dmpCluster(&Serial, 8, 0, 4);
  Serial.println("Done");
}

void loop() {
  // put your main code here, to run repeatedly:

}