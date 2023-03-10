// Simple test of Unicode filename.
// Unicode is supported as UTF-8 encoded strings.
#include "SdFat.h"

// USE_UTF8_LONG_NAMES must be non-zero in SdFat/src/SdFatCongfig.h
#if USE_UTF8_LONG_NAMES

#define UTF8_FOLDER u8"😀"
const char* names[] = {u8"россиянин", u8"très élégant", u8"狗.txt", nullptr};

// Remove files if non-zero.
#define REMOVE_UTF8_FILES 1

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16))
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(16))
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
  if (!sd.exists(UTF8_FOLDER)) {
    if (!sd.mkdir(UTF8_FOLDER)) {
      Serial.println("sd.mkdir failed");
      return;
    }
  }
  if (!sd.chdir(UTF8_FOLDER)) {
    Serial.println("sd.chdir failed");
    return;
  }
  for (uint8_t i = 0; names[i]; i++) {
    if (!file.open(names[i], O_WRONLY | O_CREAT)) {
      Serial.println("file.open failed");
      return;
    }
    file.println(names[i]);
    file.close();
  }
  Serial.println("ls:");
  sd.ls("/", LS_SIZE | LS_R);
#if REMOVE_UTF8_FILES  // For debug test of remove and rmdir.
  for (uint8_t i = 0; names[i]; i++) {
    sd.remove(names[i]);
  }
  sd.chdir();
  sd.rmdir(UTF8_FOLDER);
  Serial.println("After remove and rmdir");
  sd.ls(LS_SIZE | LS_R);
#endif  // REMOVE_UTF8_FILES
  Serial.println("Done!");
}
void loop() {
}
#else  // USE_UTF8_LONG_NAMES
#error USE_UTF8_LONG_NAMES must be non-zero in SdFat/src/SdFatCongfig.h
#endif  // USE_UTF8_LONG_NAMES