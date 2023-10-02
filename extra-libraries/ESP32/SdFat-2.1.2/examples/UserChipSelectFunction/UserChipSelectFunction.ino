// An example of an external chip select functions.
// Useful for port expanders or replacement of the standard GPIO functions.
//
#include "SdFat.h"

// SD_CHIP_SELECT_MODE must be set to one or two in SdFat/SdFatConfig.h.
// A value of one allows optional replacement and two requires replacement.
#if SD_CHIP_SELECT_MODE == 1 || SD_CHIP_SELECT_MODE == 2

// SD chip select pin.
#define SD_CS_PIN SS

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50))

SdFat sd;

// Stats to verify function calls.
uint32_t initCalls = 0;
uint32_t writeCalls = 0;
//------------------------------------------------------------------------------
// Modify these functions for your port expander or custom GPIO library.
void sdCsInit(SdCsPin_t pin) {
  initCalls++;
  pinMode(pin, OUTPUT);
}
void sdCsWrite(SdCsPin_t pin, bool level) {
  writeCalls++;
  digitalWrite(pin, level);
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  sd.ls(&Serial, LS_SIZE);

  Serial.print(F("sdCsInit calls: "));
  Serial.println(initCalls);
  Serial.print(F("sdCsWrite calls: "));
  Serial.println(writeCalls);
}
//------------------------------------------------------------------------------
void loop() {}
#else  // SD_CHIP_SELECT_MODE == 1 || SD_CHIP_SELECT_MODE == 2
#error SD_CHIP_SELECT_MODE must be one or two in SdFat/SdFatConfig.h
#endif  // SD_CHIP_SELECT_MODE == 1 || SD_CHIP_SELECT_MODE == 2
