// Test of Teensy exFAT DMA ADC logger.
// This is mainly to test use of RingBuf in an ISR.
// This example only supports pins on the first ADC.
// it has only been tested on Teensy 3.6 and 4.1.
// You should modify it for serious use as a data logger.
//
#include "ADC.h"
#include "DMAChannel.h"
#include "FreeStack.h"
#include "RingBuf.h"
#include "SdFat.h"

// Pin must be on first ADC.
#define ADC_PIN A0

// 400 sector RingBuf - could be larger on Teensy 4.1.
const size_t RING_BUF_SIZE = 400 * 512;

// Preallocate 8GiB file.
const uint64_t PRE_ALLOCATE_SIZE = 8ULL << 30;

// Use FIFO SDIO.
#define SD_CONFIG SdioConfig(FIFO_SDIO)

ADC adc;

DMAChannel dma(true);

SdFs sd;

FsFile file;

// Ping-pong DMA buffer.
DMAMEM static uint16_t __attribute__((aligned(32))) dmaBuf[2][256];

// Count of DMA interrupts.
volatile size_t dmaCount;

// RingBuf for 512 byte sectors.
RingBuf<FsFile, RING_BUF_SIZE> rb;

// Shared between ISR and background.
volatile size_t maxBytesUsed;

// Overrun error for write to RingBuf.
volatile bool overrun;
//------------------------------------------------------------------------------
// ISR for DMA.
static void isr() {
  if (!overrun) {
    // Clear cache for buffer filled by DMA to insure read from DMA memory.
    arm_dcache_delete((void*)dmaBuf[dmaCount & 1], 512);
    // Enable RingBuf functions to be called in ISR.
    rb.beginISR();
    if (rb.write(dmaBuf[dmaCount & 1], 512) == 512) {
      dmaCount++;
      if (rb.bytesUsed() > maxBytesUsed) {
        maxBytesUsed = rb.bytesUsed();
      }
    } else {
      overrun = true;
    }
    // End use of RingBuf functions in ISR.
    rb.endISR();
  }
  dma.clearComplete();
  dma.clearInterrupt();
#if defined(__IMXRT1062__)
  // Handle clear interrupt glitch in Teensy 4.x!
  asm("DSB");
#endif  // defined(__IMXRT1062__)
}
//------------------------------------------------------------------------------
#if defined(__IMXRT1062__)  // Teensy 4.x
#define SOURCE_SADDR ADC1_R0
#define SOURCE_EVENT DMAMUX_SOURCE_ADC1
#else
#define SOURCE_SADDR ADC0_RA
#define SOURCE_EVENT DMAMUX_SOURCE_ADC0
#endif
//------------------------------------------------------------------------------
static void init(uint8_t pin) {
  dma.begin();
  dma.attachInterrupt(isr);
  dma.source((volatile const signed short&)SOURCE_SADDR);
  dma.destinationBuffer((volatile uint16_t*)dmaBuf, sizeof(dmaBuf));
  dma.interruptAtHalf();
  dma.interruptAtCompletion();
  dma.triggerAtHardwareEvent(SOURCE_EVENT);
  dma.enable();
  adc.adc0->enableDMA();
  adc.adc0->startContinuous(pin);
}
//------------------------------------------------------------------------------
void stopDma() {
  adc.adc0->disableDMA();
  dma.disable();
}
//------------------------------------------------------------------------------
void printTest(Print* pr) {
  if (file.fileSize() < 1024 * 2) {
    return;
  }
  file.rewind();
  rb.begin(&file);
  // Could readIn RING_BUF_SIZE bytes and write to a csv file in a loop.
  if (rb.readIn(2048) != 2048) {
    sd.errorHalt("rb.readIn failed");
  }
  uint16_t data;
  for (size_t i = 0; i < 1024; i++) {
    pr->print(i);
    pr->print(',');
    // Test read with: template <typename Type>bool read(Type* data).
    rb.read(&data);
    pr->println(data);
  }
}
//------------------------------------------------------------------------------
void runTest(uint8_t pin) {
  dmaCount = 0;
  maxBytesUsed = 0;
  overrun = false;
  do {
    delay(10);
  } while (Serial.read() >= 0);

  if (!file.open("IsrLoggerTest.bin", O_CREAT | O_TRUNC | O_RDWR)) {
    sd.errorHalt("file.open failed");
  }
  if (!file.preAllocate(PRE_ALLOCATE_SIZE)) {
    sd.errorHalt("file.preAllocate failed");
  }
  rb.begin(&file);
  Serial.println("Type any character to stop\n");

  init(pin);
  uint32_t samplingTime = micros();
  while (!overrun && !Serial.available()) {
    size_t n = rb.bytesUsed();
    if ((n + file.curPosition()) >= (PRE_ALLOCATE_SIZE - 512)) {
      Serial.println("File full - stopping");
      break;
    }
    if (n >= 512) {
      if (rb.writeOut(512) != 512) {
        Serial.println("writeOut() failed");
        file.close();
        return;
      }
    }
  }
  stopDma();
  samplingTime = micros() - samplingTime;
  if (!rb.sync()) {
    Serial.println("sync() failed");
    file.close();
    return;
  }
  if (!file.truncate()) {
    sd.errorHalt("truncate failed");
  }
  if (overrun) {
    Serial.println("Overrun ERROR!!");
  }
  Serial.print("dmsCount ");
  Serial.println(dmaCount);
  Serial.print("RingBufSize ");
  Serial.println(RING_BUF_SIZE);
  Serial.print("maxBytesUsed ");
  Serial.println(maxBytesUsed);
  Serial.print("fileSize ");
  file.printFileSize(&Serial);
  Serial.println();
  Serial.print(0.000001 * samplingTime);
  Serial.println(" seconds");
  Serial.print(1.0 * file.fileSize() / samplingTime, 3);
  Serial.println(" MB/sec\n");
  printTest(&Serial);
  file.close();
}
//------------------------------------------------------------------------------
void waitSerial(const char* msg) {
  do {
    delay(10);
  } while (Serial.read() >= 0);
  Serial.println(msg);
  while (!Serial.available()) {
  }
  Serial.println();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
  waitSerial("Type any character to begin");
  Serial.print("FreeStack: ");
  Serial.println(FreeStack());
}
//------------------------------------------------------------------------------
void loop() {
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  // Try for max speed.
  adc.adc0->setAveraging(1);
  adc.adc0->setResolution(10);
  adc.adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
  adc.adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED);
  runTest(ADC_PIN);
  waitSerial("Type any character to run test again");
}
