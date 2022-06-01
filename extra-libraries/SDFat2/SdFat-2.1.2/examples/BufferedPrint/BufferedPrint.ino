// Test and benchmark of the fast bufferedPrint class.
//
// Mainly for AVR but may improve print performance with other CPUs.
#include "SdFat.h"
#include "BufferedPrint.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0
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

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
typedef File file_t;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
SdExFat sd;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
SdFs sd;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

// number of lines to print
const uint16_t N_PRINT = 20000;
//------------------------------------------------------------------------------
void benchmark() {
  file_t file;
  BufferedPrint<file_t, 64> bp;
  // do write test
  Serial.println();
  for (int test = 0; test < 6; test++) {
    char fileName[13] = "bench0.txt";
    fileName[5] = '0' + test;
    // open or create file - truncate existing file.
    if (!file.open(fileName, O_RDWR | O_CREAT | O_TRUNC)) {
      sd.errorHalt(&Serial, F("open failed"));
    }
    if (test & 1) {
      bp.begin(&file);
    }
    uint32_t t = millis();
    switch(test) {
    case 0:
      Serial.println(F("Test of println(uint16_t)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.println(i);
      }
      break;

    case 1:
      Serial.println(F("Test of printField(uint16_t, char)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        bp.printField(i, '\n');
      }
      break;

    case 2:
      Serial.println(F("Test of println(uint32_t)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.println(12345678UL + i);
      }
      break;

    case 3:
      Serial.println(F("Test of printField(uint32_t, char)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        bp.printField(12345678UL + i, '\n');
      }
      break;

    case 4:
      Serial.println(F("Test of println(double)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        file.println((double)0.01*i);
      }
      break;

    case 5:
      Serial.println(F("Test of printField(double, char)"));
      for (uint16_t i = 0; i < N_PRINT; i++) {
        bp.printField((double)0.01*i, '\n');
      }
      break;

    }
    if (test & 1) {
      bp.sync();
    }
    if (file.getWriteError()) {
      sd.errorHalt(&Serial, F("write failed"));
    }
    double s = file.fileSize();
    file.close();
    t = millis() - t;
    Serial.print(F("Time "));
    Serial.print(0.001*t, 3);
    Serial.println(F(" sec"));
    Serial.print(F("File size "));
    Serial.print(0.001*s);
    Serial.println(F(" KB"));
    Serial.print(F("Write "));
    Serial.print(s/t);
    Serial.println(F(" KB/sec"));
    Serial.println();
  }
}
//------------------------------------------------------------------------------
void testMemberFunctions() {
  BufferedPrint<Print, 32> bp(&Serial);
  char c = 'c';    // char
//#define BASIC_TYPES
#ifdef BASIC_TYPES
  signed char sc = -1;   // signed 8-bit
  unsigned char uc = 1;  // unsiged 8-bit
  signed short ss = -2;  // signed 16-bit
  unsigned short us = 2; // unsigned 16-bit
  signed long sl = -4;   // signed 32-bit
  unsigned long ul = 4;  // unsigned 32-bit
#else  // BASIC_TYPES
  int8_t sc = -1;  // signed 8-bit
  uint8_t uc = 1;  // unsiged 8-bit
  int16_t ss = -2; // signed 16-bit
  uint16_t us = 2; // unsigned 16-bit
  int32_t sl = -4; // signed 32-bit
  uint32_t ul = 4; // unsigned 32-bit
#endif  // BASIC_TYPES
  float f = -1.234;
  double d = -5.678;
  bp.println();
  bp.println("Test print()");
  bp.print(c);
  bp.println();
  bp.print("string");
  bp.println();
  bp.print(F("flash"));
  bp.println();
  bp.print(sc);
  bp.println();
  bp.print(uc);
  bp.println();
  bp.print(ss);
  bp.println();
  bp.print(us);
  bp.println();
  bp.print(sl);
  bp.println();
  bp.print(ul);
  bp.println();
  bp.print(f);
  bp.println();
  bp.print(d);
  bp.println();
  bp.println();

  bp.println("Test println()");
  bp.println(c);
  bp.println("string");
  bp.println(F("flash"));
  bp.println(sc);
  bp.println(uc);
  bp.println(ss);
  bp.println(us);
  bp.println(sl);
  bp.println(ul);
  bp.println(f);
  bp.println(d);
  bp.println();

  bp.println("Test printField()");
  bp.printField(c, ',');
  bp.printField("string", ',');
  bp.printField(F("flash"), ',');
  bp.printField(sc, ',');
  bp.printField(uc, ',');
  bp.printField(ss, ',');
  bp.printField(us, ',');
  bp.printField(sl, ',');
  bp.printField(ul, ',');
  bp.printField(f, ',');
  bp.printField(d, '\n');

  bp.sync();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("Type any character to begin.");
  while(!Serial.available()) {}
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  Serial.println();
  Serial.println(F("Test member funcions:"));
  testMemberFunctions();
  Serial.println();
  Serial.println(F("Benchmark performance for uint16_t, uint32_t, and double:"));
  benchmark();
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void loop() {
}