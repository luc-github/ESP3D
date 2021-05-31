// Test of Teensy exFAT DMA ADC logger.
// This is mainly to test use of RingBuf in an ISR.
// You should modify it for serious use as a data logger.
//
#include "DMAChannel.h"
#include "SdFat.h"
#include "FreeStack.h"
#include "RingBuf.h"

// 400 sector RingBuf - could be larger on Teensy 4.1.
const size_t RING_BUF_SIZE = 400*512;

// Preallocate 8GiB file.
const uint64_t PRE_ALLOCATE_SIZE = 8ULL << 30;

// Use FIFO SDIO.
#define SD_CONFIG SdioConfig(FIFO_SDIO)

DMAChannel dma(true);

SdFs sd;

FsFile file;
//------------------------------------------------------------------------------
// Ping-pong DMA buffer.
DMAMEM static uint16_t __attribute__((aligned(32))) dmaBuf[2][256];
size_t dmaCount;

// RingBuf for 512 byte sectors.
RingBuf<FsFile, RING_BUF_SIZE> rb;

// Shared between ISR and background.
volatile size_t maxBytesUsed;

volatile bool overrun;
//------------------------------------------------------------------------------
//ISR.
static void isr() {
  if (rb.bytesFreeIsr() >= 512 && !overrun) {
    rb.memcpyIn(dmaBuf[dmaCount & 1], 512);
    dmaCount++;
    if (rb.bytesUsed() > maxBytesUsed) {
      maxBytesUsed = rb.bytesUsed();
    }
  } else {
    overrun = true;
  }
  dma.clearComplete();
  dma.clearInterrupt();
#if defined(__IMXRT1062__)
  // Handle clear interrupt glitch in Teensy 4.x!
  asm("DSB");
#endif  // defined(__IMXRT1062__)
}
//------------------------------------------------------------------------------
// Over-clocking will degrade quality - use only for stress testing.
void overclock() {
#if defined(__IMXRT1062__) // Teensy 4.0
  ADC1_CFG  =
    // High Speed Configuration
    ADC_CFG_ADHSC |
    // Sample period 3 clocks
    ADC_CFG_ADSTS(0) |
    // Input clock
    ADC_CFG_ADIV(0) |
    // Not selected - Long Sample Time Configuration
    // ADC_CFG_ADLSMP |
    // 12-bit
    ADC_CFG_MODE(2) |
    // Asynchronous clock
    ADC_CFG_ADICLK(3);
#else // defined(__IMXRT1062__)
  // Set 12 bit mode and max over-clock
  ADC0_CFG1 =
    // Clock divide select, 0=direct, 1=div2, 2=div4, 3=div8
    ADC_CFG1_ADIV(0) |
    // Sample time configuration, 0=Short, 1=Long
    // ADC_CFG1_ADLSMP |
    // Conversion mode, 0=8 bit, 1=12 bit, 2=10 bit, 3=16 bit
    ADC_CFG1_MODE(1) |
    // Input clock, 0=bus, 1=bus/2, 2=OSCERCLK, 3=async
    ADC_CFG1_ADICLK(0);

  ADC0_CFG2 = ADC_CFG2_MUXSEL | ADC_CFG2_ADLSTS(3);
#endif  // defined(__IMXRT1062__)
}
//------------------------------------------------------------------------------
#if defined(__IMXRT1062__) // Teensy 4.0
#define SOURCE_SADDR ADC1_R0
#define SOURCE_EVENT DMAMUX_SOURCE_ADC1
#else
#define SOURCE_SADDR ADC0_RA
#define SOURCE_EVENT DMAMUX_SOURCE_ADC0
#endif
//------------------------------------------------------------------------------
// Should replace ADC stuff with calls to Teensy ADC library.
// https://github.com/pedvide/ADC
static void init(uint8_t pin) {
  uint32_t adch;
	uint32_t i, sum = 0;
	// Actually, do many normal reads, to start with a nice DC level
	for (i=0; i < 1024; i++) {
		sum += analogRead(pin);
	}
#if defined(__IMXRT1062__) // Teensy 4.0
  // save channel
  adch = ADC1_HC0 & 0x1F;
  // Continuous conversion , DMA enable
  ADC1_GC = ADC_GC_ADCO | ADC_GC_DMAEN;
  // start conversion
  ADC1_HC0 = adch;
#else  // defined(__IMXRT1062__) // Teensy 4.0
  // save channel
  adch = ADC0_SC1A & 0x1F;
  // DMA enable
  ADC0_SC2 |= ADC_SC2_DMAEN;
  // Continuous conversion enable
  ADC0_SC3 = ADC_SC3_ADCO;
  // Start ADC
  ADC0_SC1A = adch;
 #endif  // defined(__IMXRT1062__) // Teensy 4.0
	// set up a DMA channel to store the ADC data
 	dma.attachInterrupt(isr);
	dma.begin();
  dma.source((volatile const signed short &)SOURCE_SADDR);
  dma.destinationBuffer((volatile uint16_t*)dmaBuf, sizeof(dmaBuf));
  dma.interruptAtHalf();
  dma.interruptAtCompletion();
	dma.triggerAtHardwareEvent(SOURCE_EVENT);
	dma.enable();
}
//------------------------------------------------------------------------------
void stopDma() {
#if defined(__IMXRT1062__) // Teensy 4.0
  ADC1_GC = 0;
#else  // defined(__IMXRT1062__)
  ADC0_SC3 = 0;
#endif  // defined(__IMXRT1062__)
  dma.disable();
}
//------------------------------------------------------------------------------
void printTest(Print* pr) {
  if (file.fileSize() < 1024*2) {
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
    rb.memcpyOut(&data, 2);
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
  samplingTime = (micros() - samplingTime);
  if (!file.truncate()) {
    sd.errorHalt("truncate failed");
  }
  if (overrun) {
    Serial.println("Overrun ERROR!!");
  }
  Serial.print("RingBufSize ");
  Serial.println(RING_BUF_SIZE);
  Serial.print("maxBytesUsed ");
  Serial.println(maxBytesUsed);
  Serial.print("fileSize ");
  Serial.println((uint32_t)file.fileSize());
  Serial.print(0.000001*samplingTime);
  Serial.println(" seconds");
  Serial.print(1.0*file.fileSize()/samplingTime, 3);
  Serial.println(" MB/sec\n");
  printTest(&Serial);
  file.close();
}
//------------------------------------------------------------------------------
void waitSerial(const char* msg) {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
  Serial.println(msg);
  while (!Serial.available()) {}
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
//analogReadAveraging(1);
//analogReadResolution(12);
//overclock(); // 3 Msps on Teensy 3.6 - requires high quality card.
  runTest(A0);
  waitSerial("Type any character to run test again");
}
