#include "SdFat.h"

#ifdef __AVR__
const uint32_t FILE_SIZE_MiB = 10UL;
#else  // __AVR__
const uint32_t FILE_SIZE_MiB = 100UL;
#endif

bool waitBusy = true;

#define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI)
//#define SD_CONFIG SdSpiConfig(SS, SHARED_SPI)
// Config for Teensy 3.5/3.6 buit-in SD.
//#define SD_CONFIG SdSpiConfig(SDCARD_SS_PIN, DEDICATED_SPI)
//#define SD_CONFIG SdioConfig(FIFO_SDIO)

//------------------------------------------------------------------------------
const uint64_t FILE_SIZE = (uint64_t)FILE_SIZE_MiB  << 20;

SdFs sd;
FsFile file;

uint8_t buf[512];

#define error(s) sd.errorHalt(&Serial, F(s))
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
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  delay(1000);
//------------------------------------------------------------------------------
}
void loop() {
  clearSerialInput();
  Serial.println(F("\nType any character to start\n"));
  while (!Serial.available()) {
    yield();
  }
  // Initialize the SD card.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }
  if (!file.open("SdBusyTest.bin", O_RDWR | O_CREAT |O_TRUNC)) {
    error("file open failed");
  }
  if (!file.preAllocate(FILE_SIZE)) {
    error("preallocate failed");
  }
  Serial.print(F("Starting write of "));
  Serial.print(FILE_SIZE_MiB);
  Serial.println(F(" MiB."));
  uint32_t maxWrite = 0;
  uint32_t minWrite = 99999999;
  uint32_t ms = millis();
  uint32_t maxBusy = 0;
  uint32_t minBusy = UINT32_MAX;

  // Write a dummy sector to start a multi-sector write.
  if(file.write(buf, sizeof(buf)) != sizeof(buf)) {
    error("write failed for first sector");
  }

  while (file.position() < FILE_SIZE) {
    uint32_t m = micros();
    if (waitBusy) {
      m = micros();
      while (sd.card()->isBusy()) {}
      m = micros() - m;
      if (m < minBusy) {
        minBusy = m;
      }
      if (m > maxBusy) {
        maxBusy = m;
      }
    }
    m = micros();
    if (file.write(buf, sizeof(buf)) != sizeof(buf)) {
      error("write failed");
    }
    m = micros() - m;
    if (m < minWrite) {
      minWrite = m;
    }
    if (m > maxWrite) {
      maxWrite = m;
    }
  }
  file.close();
  ms = millis() - ms;
  Serial.println(F("\nTimes in micros"));
  if (waitBusy) {
    Serial.print(F("minBusy: "));
    Serial.println(minBusy);
    Serial.print(F("maxBusy: "));
    Serial.println(maxBusy);
  }
  Serial.print(F("minWrite: "));
  Serial.println(minWrite);
  Serial.print(F("maxWrite: "));
  Serial.println(maxWrite);
  Serial.print(1e-3*ms);
  Serial.println(F(" Seconds"));
  Serial.print(1.0*FILE_SIZE/ms);
  Serial.println(F(" KB/sec"));

}