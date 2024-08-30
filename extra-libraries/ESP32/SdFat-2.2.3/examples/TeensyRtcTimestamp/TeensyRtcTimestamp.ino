// Test of time-stamp callback with Teensy 3/4.
// The upload time will be used to set the RTC.
// You must arrange for syncing the RTC.
#include <TimeLib.h>

#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
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
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

//------------------------------------------------------------------------------
// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(year(), month(), day());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(hour(), minute(), second());

  // Return low time bits in units of 10 ms.
  *ms10 = second() & 1 ? 100 : 0;
}
//------------------------------------------------------------------------------
time_t getTeensy3Time() { return Teensy3Clock.get(); }
//------------------------------------------------------------------------------
void printField(Print* pr, char sep, uint8_t v) {
  if (sep) {
    pr->write(sep);
  }
  if (v < 10) {
    pr->write('0');
  }
  pr->print(v);
}
//------------------------------------------------------------------------------
void printNow(Print* pr) {
  pr->print(year());
  printField(pr, '-', month());
  printField(pr, '-', day());
  printField(pr, ' ', hour());
  printField(pr, ':', minute());
  printField(pr, ':', second());
}
//------------------------------------------------------------------------------
void setup() {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  Serial.println(F("Type any character to begin"));
  while (!Serial.available()) {
    yield();
  }
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
    return;
  }
  Serial.print(F("DateTime::now "));
  printNow(&Serial);
  Serial.println();

  // Set callback
  FsDateTime::setCallback(dateTime);

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  // Remove old version to set create time.
  if (sd.exists("RtcTest.txt")) {
    sd.remove("RtcTest.txt");
  }
  if (!file.open("RtcTest.txt", FILE_WRITE)) {
    Serial.println(F("file.open failed"));
    return;
  }
  // Print current date time to file.
  file.print(F("Test file at: "));
  printNow(&file);
  file.println();

  file.close();
  // List files in SD root.
  sd.ls(LS_DATE | LS_SIZE);
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void loop() {}