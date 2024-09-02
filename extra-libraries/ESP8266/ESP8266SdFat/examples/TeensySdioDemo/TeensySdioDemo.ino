// Simple performance test for Teensy 3.5/3.6 4.0 SDHC.
// Demonstrates yield() efficiency for SDIO modes.
#include "SdFat.h"

// Use built-in SD for SPI modes on Teensy 3.5/3.6.
// Teensy 4.0 use first SPI port.
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else   // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3

// 32 KiB buffer.
const size_t BUF_DIM = 32768;

// 8 MiB file.
const uint32_t FILE_SIZE = 256UL * BUF_DIM;

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

uint8_t buf[BUF_DIM];

// buffer as uint32_t
uint32_t* buf32 = (uint32_t*)buf;

// Total usec in read/write calls.
uint32_t totalMicros = 0;
// Time in yield() function.
uint32_t yieldMicros = 0;
// Number of yield calls.
uint32_t yieldCalls = 0;
// Max busy time for single yield call.
uint32_t yieldMaxUsec = 0;
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
void errorHalt(const char* msg) {
  Serial.print("Error: ");
  Serial.println(msg);
  if (sd.sdErrorCode()) {
    if (sd.sdErrorCode() == SD_CARD_ERROR_ACMD41) {
      Serial.println("Try power cycling the SD card.");
    }
    printSdErrorSymbol(&Serial, sd.sdErrorCode());
    Serial.print(", ErrorData: 0X");
    Serial.println(sd.sdErrorData(), HEX);
  }
  while (true) {
  }
}
bool ready = false;
//------------------------------------------------------------------------------
bool sdBusy() { return ready ? sd.card()->isBusy() : false; }
//------------------------------------------------------------------------------
// Replace "weak" system yield() function.
void yield() {
  // Only count cardBusy time.
  if (!sdBusy()) {
    return;
  }
  uint32_t m = micros();
  yieldCalls++;
  while (sdBusy()) {
    // Do something here.
  }
  m = micros() - m;
  if (m > yieldMaxUsec) {
    yieldMaxUsec = m;
  }
  yieldMicros += m;
}
//------------------------------------------------------------------------------
void runTest() {
  // Zero Stats
  totalMicros = 0;
  yieldMicros = 0;
  yieldCalls = 0;
  yieldMaxUsec = 0;
  if (!file.open("TeensyDemo.bin", O_RDWR | O_CREAT)) {
    errorHalt("open failed");
  }
  Serial.println("\nsize,write,read");
  Serial.println("bytes,KB/sec,KB/sec");
  for (size_t nb = 512; nb <= BUF_DIM; nb *= 2) {
    uint32_t nRdWr = FILE_SIZE / nb;
    if (!file.truncate(0)) {
      errorHalt("truncate failed");
    }

    Serial.print(nb);
    Serial.print(',');
    uint32_t t = micros();
    for (uint32_t n = 0; n < nRdWr; n++) {
      // Set start and end of buffer.
      buf32[0] = n;
      buf32[nb / 4 - 1] = n;
      if (nb != file.write(buf, nb)) {
        errorHalt("write failed");
      }
    }
    t = micros() - t;
    totalMicros += t;
    Serial.print(1000.0 * FILE_SIZE / t);
    Serial.print(',');
    file.rewind();
    t = micros();

    for (uint32_t n = 0; n < nRdWr; n++) {
      if ((int)nb != file.read(buf, nb)) {
        errorHalt("read failed");
      }
      // crude check of data.
      if (buf32[0] != n || buf32[nb / 4 - 1] != n) {
        errorHalt("data check");
      }
    }
    t = micros() - t;
    totalMicros += t;
    Serial.println(1000.0 * FILE_SIZE / t);
  }
  file.close();
  Serial.print("\ntotalMicros  ");
  Serial.println(totalMicros);
  Serial.print("yieldMicros  ");
  Serial.println(yieldMicros);
  Serial.print("yieldCalls   ");
  Serial.println(yieldCalls);
  Serial.print("yieldMaxUsec ");
  Serial.println(yieldMaxUsec);
  //  Serial.print("kHzSdClk     ");
  //  Serial.println(kHzSdClk());
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
}
//------------------------------------------------------------------------------
void loop() {
  static bool warn = true;
  if (warn) {
    warn = false;
    Serial.println(
        "SD cards must be power cycled to leave\n"
        "SPI mode so do SDIO tests first.\n"
        "\nCycle power on the card if an error occurs.");
  }
  clearSerialInput();

  Serial.println(
      "\nType '1' for FIFO SDIO"
      "\n     '2' for DMA SDIO"
      "\n     '3' for Dedicated SPI"
      "\n     '4' for Shared SPI");
  while (!Serial.available()) {
  }
  char c = Serial.read();

  if (c == '1') {
    if (!sd.begin(SdioConfig(FIFO_SDIO))) {
      errorHalt("begin failed");
    }
    Serial.println("\nFIFO SDIO mode.");
  } else if (c == '2') {
    if (!sd.begin(SdioConfig(DMA_SDIO))) {
      errorHalt("begin failed");
    }
    Serial.println("\nDMA SDIO mode - slow for small transfers.");
  } else if (c == '3') {
#if ENABLE_DEDICATED_SPI
    if (!sd.begin(SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50)))) {
      errorHalt("begin failed");
    }
    Serial.println("\nDedicated SPI mode.");
#else   // ENABLE_DEDICATED_SPI
    Serial.println("ENABLE_DEDICATED_SPI must be non-zero.");
    return;
#endif  // ENABLE_DEDICATED_SPI
  } else if (c == '4') {
    if (!sd.begin(SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50)))) {
      errorHalt("begin failed");
    }
    Serial.println("\nShared SPI mode - slow for small transfers.");
  } else {
    Serial.println("Invalid input");
    return;
  }
  ready = true;
  runTest();
  ready = false;
}