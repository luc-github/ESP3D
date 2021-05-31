// Simple test of Unicode file name.
// Note: Unicode is only supported by the SdExFat class.
//       No exFAT functions will be defined for char* paths.
//       The SdFs class cannot be used.
#include "SdFat.h"
#if USE_EXFAT_UNICODE_NAMES

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Use SPI, SD_CS_PIN, SHARED_SPI, 50 MHz.
#define SD_CONFIG SdSpiConfig(SD_CS_PIN)

SdExFat sd;

ExFile file;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  Serial.println("Type any character to begin");
  while (!Serial.available()) {
    yield();
  }
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  if (!file.open(u"Euros \u20AC test.txt", FILE_WRITE)) {
    Serial.println("file.open failed");
    return;
  }
  file.println("This is not Unicode");
  file.close();
  Serial.println("Done!");
}
void loop() {
}
#else  // USE_EXFAT_UNICODE_NAMES
#error USE_EXFAT_UNICODE_NAMES must be nonzero in SdFat/src/ExFatLib/ExFatCongfig.h
#endif  // USE_EXFAT_UNICODE_NAMES
