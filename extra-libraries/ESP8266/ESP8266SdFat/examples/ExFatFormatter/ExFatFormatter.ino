// Force exFAT formatting for all SD cards larger than 512MB.
#include "SdFat.h"

using namespace sdfat;


/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Select fastest interface.
#if HAS_SDIO_CLASS
// SD config for Teensy 3.6 SDIO.
#define SD_CONFIG SdioConfig(FIFO_SDIO)
//#define SD_CONFIG SdioConfig(DMA_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS

SdExFat sd;
//------------------------------------------------------------------------------
void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}
//------------------------------------------------------------------------------
void errorHalt() {
  sd.printSdError(&Serial);
  SysCall::halt();
}
#define error(s) (Serial.println(F(s)),errorHalt())
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println(F("Type any character to begin"));

  while (!Serial.available()) {
    yield();
  }
  clearSerialInput();
  Serial.println();
  Serial.println(F(
    "Your SD will be formated exFAT.\r\n"
    "All data on the SD will be lost.\r\n"
    "Type 'Y' to continue.\r\n"));

  while (!Serial.available()) {
    yield();
  }
  if (Serial.read() != 'Y') {
    Serial.println(F("Exiting, 'Y' not typed."));
    return;
  }
  if (!sd.cardBegin(SD_CONFIG)) {
    error("cardBegin failed");
  }
  if(!sd.format(&Serial)) {
    error("format failed");
  }
  if (!sd.volumeBegin()) {
    error("volumeBegin failed");
  }
  Serial.print(F("Bytes per cluster: "));
  Serial.println(sd.bytesPerCluster());
  Serial.println(F("Done"));
}
void loop() {
}
