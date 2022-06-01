// Test Teensy SDIO with write busy in a data logger demo.
//
// The driver writes to the uSDHC controller's FIFO then returns
// while the controller writes the data to the SD.  The first sector
// puts the controller in write mode and takes about 11 usec on a
// Teensy 4.1. About 5 usec is required to write a sector when the
// controller is in write mode.

#include "SdFat.h"
#include "RingBuf.h"

// Use Teensy SDIO
#define SD_CONFIG  SdioConfig(FIFO_SDIO)

// Interval between points for 25 ksps.
#define LOG_INTERVAL_USEC 40

// Size to log 10 byte lines at 25 kHz for more than ten minutes.
#define LOG_FILE_SIZE 10*25000*600  // 150,000,000 bytes.

// Space to hold more than 800 ms of data for 10 byte lines at 25 ksps.
#define RING_BUF_CAPACITY 400*512
#define LOG_FILENAME "SdioLogger.csv"

SdFs sd;
FsFile file;

// RingBuf for File type FsFile.
RingBuf<FsFile, RING_BUF_CAPACITY> rb;

void logData() {
  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  // Open or create file - truncate existing file.
  if (!file.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println("open failed\n");
    return;
  }
  // File must be pre-allocated to avoid huge
  // delays searching for free clusters.
  if (!file.preAllocate(LOG_FILE_SIZE)) {
     Serial.println("preAllocate failed\n");
     file.close();
     return;
  }
  // initialize the RingBuf.
  rb.begin(&file);
  Serial.println("Type any character to stop");

  // Max RingBuf used bytes. Useful to understand RingBuf overrun.
  size_t maxUsed = 0;

  // Min spare micros in loop.
  int32_t minSpareMicros = INT32_MAX;

  // Start time.
  uint32_t logTime = micros();
  // Log data until Serial input or file full.
  while (!Serial.available()) {
    // Amount of data in ringBuf.
    size_t n = rb.bytesUsed();
    if ((n + file.curPosition()) > (LOG_FILE_SIZE - 20)) {
      Serial.println("File full - quitting.");
      break;
    }
    if (n > maxUsed) {
      maxUsed = n;
    }
    if (n >= 512 && !file.isBusy()) {
      // Not busy only allows one sector before possible busy wait.
      // Write one sector from RingBuf to file.
      if (512 != rb.writeOut(512)) {
        Serial.println("writeOut failed");
        break;
      }
    }
    // Time for next point.
    logTime += LOG_INTERVAL_USEC;
    int32_t spareMicros = logTime - micros();
    if (spareMicros < minSpareMicros) {
      minSpareMicros = spareMicros;
    }
    if (spareMicros <= 0) {
      Serial.print("Rate too fast ");
      Serial.println(spareMicros);
      break;
    }
    // Wait until time to log data.
    while (micros() < logTime) {}

    // Read ADC0 - about 17 usec on Teensy 4, Teensy 3.6 is faster.
    uint16_t adc = analogRead(0);
    // Print spareMicros into the RingBuf as test data.
    rb.print(spareMicros);
    rb.write(',');
    // Print adc into RingBuf.
    rb.println(adc);
    if (rb.getWriteError()) {
      // Error caused by too few free bytes in RingBuf.
      Serial.println("WriteError");
      break;
    }
  }
  // Write any RingBuf data to file.
  rb.sync();
  file.truncate();
  file.rewind();
  // Print first twenty lines of file.
  Serial.println("spareMicros,ADC0");
  for (uint8_t n = 0; n < 20 && file.available();) {
    int c = file.read();
    if (c < 0) {
      break;
    }
    Serial.write(c);
    if (c == '\n') n++;
  }
  Serial.print("fileSize: ");
  Serial.println((uint32_t)file.fileSize());
  Serial.print("maxBytesUsed: ");
  Serial.println(maxUsed);
  Serial.print("minSpareMicros: ");
  Serial.println(minSpareMicros);
  file.close();
}
void clearSerialInput() {
  for (uint32_t m = micros(); micros() - m < 10000;) {
    if (Serial.read() >= 0) {
      m = micros();
    }
  }
}
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  // Go faster or log more channels.  ADC quality will suffer.
  // analogReadAveraging(1);
}

void loop() {
  clearSerialInput();
  Serial.println("Type any character to start");
  while (!Serial.available()) {};
  clearSerialInput();
  logData();
}