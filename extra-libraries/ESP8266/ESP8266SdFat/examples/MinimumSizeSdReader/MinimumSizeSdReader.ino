// Create a text file on the SD with this path using short 8.3 names.
#define SFN_PATH "/DIR/TEST.TXT"

// Modify CS_PIN for your chip select pin.
#define CS_PIN SS

// Set USE_SD_H to one for SD.h or zero for SdFat.
#define USE_SD_H 0

#if USE_SD_H
#include "SD.h"
File file;
#else
#include "SdFat.h"
// Setting ENABLE_DEDICATED_SPI to zero saves over 200 more bytes.
#if ENABLE_DEDICATED_SPI
#warning \
    "Set ENABLE_DEDICATED_SPI zero in SdFat/src/SdFatConfig.h for minimum size"
#endif  // ENABLE_DEDICATED_SPI
// Insure FAT16/FAT32 only.
SdFat32 SD;
// FatFile does not support Stream functions, just simple read/write.
FatFile file;
#endif

void error(const char* msg) {
  Serial.println(msg);
  while (true) {
  }
}
void setup() {
  int n;
  char buf[4];

  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Type any character to begin");
  while (!Serial.available()) {
  }

  if (!SD.begin(CS_PIN)) error("SD.begin");

#if USE_SD_H
  file = SD.open(SFN_PATH);
  if (!file) error("open");
#else
  // Open existing file with a path of 8.3 names.
  // Directories will be opened O_RDONLY files O_RDWR.
  if (!file.openExistingSFN(SFN_PATH)) error("open");
#endif
  while ((n = file.read(buf, sizeof(buf)))) {
    Serial.write(buf, n);
  }
  // close() is only needed if you write to the file. For example, read
  // config data, modify the data, rewind the file and write the data.
  // file.close();
}

void loop() {}
