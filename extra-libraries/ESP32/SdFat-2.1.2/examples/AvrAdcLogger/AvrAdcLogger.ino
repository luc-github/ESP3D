/**
 * This program logs data from the Arduino ADC to a binary file.
 *
 * Samples are logged at regular intervals. Each Sample consists of the ADC
 * values for the analog pins defined in the PIN_LIST array.  The pins numbers
 * may be in any order.
 *
 * Edit the configuration constants below to set the sample pins, sample rate,
 * and other configuration values.
 *
 * If your SD card has a long write latency, it may be necessary to use
 * slower sample rates.  Using a Mega Arduino helps overcome latency
 * problems since more 64 byte buffer blocks will be used.
 *
 * Each 64 byte data block in the file has a four byte header followed by up
 * to 60 bytes of data. (60 values in 8-bit mode or 30 values in 10-bit mode)
 * Each block contains an integral number of samples with unused space at the
 * end of the block.
 *
 */
#ifdef __AVR__
#include <SPI.h>
#include "SdFat.h"
#include "BufferedPrint.h"
#include "FreeStack.h"
#include "AvrAdcLogger.h"

// Save SRAM if 328.
#ifdef __AVR_ATmega328P__
#include "MinimumSerial.h"
MinimumSerial MinSerial;
#define Serial MinSerial
#endif  // __AVR_ATmega328P__
//------------------------------------------------------------------------------
// This example was designed for exFAT but will support FAT16/FAT32.
//
// Note: Uno will not support SD_FAT_TYPE = 3.
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
//------------------------------------------------------------------------------
// Set USE_RTC nonzero for file timestamps.
// RAM use will be marginal on Uno with RTClib.
// Set USE_RTC nonzero for file timestamps.
// RAM use will be marginal on Uno with RTClib.
// 0 - RTC not used
// 1 - DS1307
// 2 - DS3231
// 3 - PCF8523
#define USE_RTC 0
#if USE_RTC
#include "RTClib.h"
#endif  // USE_RTC
//------------------------------------------------------------------------------
// Pin definitions.
//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for SD write
// overrun errors and logging continues.
const int8_t ERROR_LED_PIN = -1;

// SD chip select pin.
const uint8_t SD_CS_PIN = SS;
//------------------------------------------------------------------------------
// Analog pin number list for a sample.  Pins may be in any order and pin
// numbers may be repeated.
const uint8_t PIN_LIST[] = {0, 1, 2, 3, 4};
//------------------------------------------------------------------------------
// Sample rate in samples per second.
const float SAMPLE_RATE = 5000;  // Must be 0.25 or greater.

// The interval between samples in seconds, SAMPLE_INTERVAL, may be set to a
// constant instead of being calculated from SAMPLE_RATE.  SAMPLE_RATE is not
// used in the code below.  For example, setting SAMPLE_INTERVAL = 2.0e-4
// will result in a 200 microsecond sample interval.
const float SAMPLE_INTERVAL = 1.0/SAMPLE_RATE;

// Setting ROUND_SAMPLE_INTERVAL non-zero will cause the sample interval to
// be rounded to a a multiple of the ADC clock period and will reduce sample
// time jitter.
#define ROUND_SAMPLE_INTERVAL 1
//------------------------------------------------------------------------------
// Reference voltage.  See the processor data-sheet for reference details.
// uint8_t const ADC_REF = 0; // External Reference AREF pin.
uint8_t const ADC_REF = (1 << REFS0);  // Vcc Reference.
// uint8_t const ADC_REF = (1 << REFS1);  // Internal 1.1 (only 644 1284P Mega)
// uint8_t const ADC_REF = (1 << REFS1) | (1 << REFS0);  // Internal 1.1 or 2.56
//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in bytes.
// The program creates a contiguous file with MAX_FILE_SIZE_MiB bytes.
// The file will be truncated if logging is stopped early.
const uint32_t MAX_FILE_SIZE_MiB = 100;  // 100 MiB file.

// log file name.  Integer field before dot will be incremented.
#define LOG_FILE_NAME "AvrAdc00.bin"

// Maximum length name including zero byte.
const size_t NAME_DIM = 40;

// Set RECORD_EIGHT_BITS non-zero to record only the high 8-bits of the ADC.
#define RECORD_EIGHT_BITS 0
//------------------------------------------------------------------------------
// FIFO size definition. Use a multiple of 512 bytes for best performance.
//
#if RAMEND < 0X8FF
#error SRAM too small
#elif RAMEND < 0X10FF
const size_t FIFO_SIZE_BYTES = 512;
#elif RAMEND < 0X20FF
const size_t FIFO_SIZE_BYTES = 4*512;
#elif RAMEND < 0X40FF
const size_t FIFO_SIZE_BYTES = 12*512;
#else  // RAMEND
const size_t FIFO_SIZE_BYTES = 16*512;
#endif  // RAMEND
//------------------------------------------------------------------------------
// ADC clock rate.
// The ADC clock rate is normally calculated from the pin count and sample
// interval.  The calculation attempts to use the lowest possible ADC clock
// rate.
//
// You can select an ADC clock rate by defining the symbol ADC_PRESCALER to
// one of these values.  You must choose an appropriate ADC clock rate for
// your sample interval.
// #define ADC_PRESCALER 7 // F_CPU/128 125 kHz on an Uno
// #define ADC_PRESCALER 6 // F_CPU/64  250 kHz on an Uno
// #define ADC_PRESCALER 5 // F_CPU/32  500 kHz on an Uno
// #define ADC_PRESCALER 4 // F_CPU/16 1000 kHz on an Uno
// #define ADC_PRESCALER 3 // F_CPU/8  2000 kHz on an Uno (8-bit mode only)
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME "tmp_adc.bin"

// Number of analog pins to log.
const uint8_t PIN_COUNT = sizeof(PIN_LIST)/sizeof(PIN_LIST[0]);

// Minimum ADC clock cycles per sample interval
const uint16_t MIN_ADC_CYCLES = 15;

// Extra cpu cycles to setup ADC with more than one pin per sample.
const uint16_t ISR_SETUP_ADC = PIN_COUNT > 1 ? 100 : 0;

// Maximum cycles for timer0 system interrupt.
const uint16_t ISR_TIMER0 = 160;
//==============================================================================
const uint32_t MAX_FILE_SIZE = MAX_FILE_SIZE_MiB << 20;

// Max SPI rate for AVR is 10 MHz for F_CPU 20 MHz, 8 MHz for F_CPU 16 MHz.
#define SPI_CLOCK SD_SCK_MHZ(10)
// Select fastest interface.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // ENABLE_DEDICATED_SPI

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

file_t binFile;
file_t csvFile;

char binName[] = LOG_FILE_NAME;

#if RECORD_EIGHT_BITS
const size_t BLOCK_MAX_COUNT = PIN_COUNT*(DATA_DIM8/PIN_COUNT);
typedef block8_t block_t;
#else  // RECORD_EIGHT_BITS
const size_t BLOCK_MAX_COUNT = PIN_COUNT*(DATA_DIM16/PIN_COUNT);
typedef block16_t block_t;
#endif // RECORD_EIGHT_BITS

// Size of FIFO in blocks.
size_t const FIFO_DIM = FIFO_SIZE_BYTES/sizeof(block_t);
block_t* fifoData;
volatile size_t fifoCount = 0; // volatile - shared, ISR and background.
size_t fifoHead = 0;  // Only accessed by ISR during logging.
size_t fifoTail = 0;  // Only accessed by writer during logging.
//==============================================================================
// Interrupt Service Routines

// Disable ADC interrupt if true.
volatile bool isrStop = false;

// Pointer to current buffer.
block_t* isrBuf = nullptr;
// overrun count
uint16_t isrOver = 0;

// ADC configuration for each pin.
uint8_t adcmux[PIN_COUNT];
uint8_t adcsra[PIN_COUNT];
uint8_t adcsrb[PIN_COUNT];
uint8_t adcindex = 1;

// Insure no timer events are missed.
volatile bool timerError = false;
volatile bool timerFlag = false;
//------------------------------------------------------------------------------
// ADC done interrupt.
ISR(ADC_vect) {
  // Read ADC data.
#if RECORD_EIGHT_BITS
  uint8_t d = ADCH;
#else  // RECORD_EIGHT_BITS
  // This will access ADCL first.
  uint16_t d = ADC;
#endif  // RECORD_EIGHT_BITS

  if (!isrBuf) {
    if (fifoCount < FIFO_DIM) {
      isrBuf = fifoData + fifoHead;
    } else {
      // no buffers - count overrun
      if (isrOver < 0XFFFF) {
        isrOver++;
      }
      // Avoid missed timer error.
      timerFlag = false;
      return;
    }
  }
  // Start ADC for next pin
  if (PIN_COUNT > 1) {
    ADMUX = adcmux[adcindex];
    ADCSRB = adcsrb[adcindex];
    ADCSRA = adcsra[adcindex];
    if (adcindex == 0) {
      timerFlag = false;
    }
    adcindex =  adcindex < (PIN_COUNT - 1) ? adcindex + 1 : 0;
  } else {
    timerFlag = false;
  }
  // Store ADC data.
  isrBuf->data[isrBuf->count++] = d;

  // Check for buffer full.
  if (isrBuf->count >= BLOCK_MAX_COUNT) {
    fifoHead = fifoHead < (FIFO_DIM - 1) ? fifoHead + 1 : 0;
    fifoCount++;
    // Check for end logging.
    if (isrStop) {
      adcStop();
      return;
    }
    // Set buffer needed and clear overruns.
    isrBuf = nullptr;
    isrOver = 0;
  }
}
//------------------------------------------------------------------------------
// timer1 interrupt to clear OCF1B
ISR(TIMER1_COMPB_vect) {
  // Make sure ADC ISR responded to timer event.
  if (timerFlag) {
    timerError = true;
  }
  timerFlag = true;
}
//==============================================================================
// Error messages stored in flash.
#define error(msg) (Serial.println(F(msg)),errorHalt())
#define assert(e) ((e) ? (void)0 : error("assert: " #e))
//------------------------------------------------------------------------------
//
void fatalBlink() {
  while (true) {
    if (ERROR_LED_PIN >= 0) {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(200);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(200);
    }
  }
}
//------------------------------------------------------------------------------
void errorHalt() {
  // Print minimal error data.
  // sd.errorPrint(&Serial);
  // Print extended error info - uses extra bytes of flash.
  sd.printSdError(&Serial);
  // Try to save data.
  binFile.close();
  fatalBlink();
}
//------------------------------------------------------------------------------
void printUnusedStack() {
  Serial.print(F("\nUnused stack: "));
  Serial.println(UnusedStack());
}
//------------------------------------------------------------------------------
#if USE_RTC
#if USE_RTC == 1
RTC_DS1307 rtc;
#elif USE_RTC == 2
RTC_DS3231 rtc;
#elif USE_RTC == 3
RTC_PCF8523 rtc;
#else  // USE_RTC == type
#error USE_RTC type not implemented.
#endif  // USE_RTC == type
// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  DateTime now = rtc.now();

  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(now.year(), now.month(), now.day());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(now.hour(), now.minute(), now.second());

  // Return low time bits in units of 10 ms.
  *ms10 = now.second() & 1 ? 100 : 0;
}
#endif  // USE_RTC
//==============================================================================
#if ADPS0 != 0 || ADPS1 != 1 || ADPS2 != 2
#error unexpected ADC prescaler bits
#endif
//------------------------------------------------------------------------------
inline bool adcActive() {return (1 << ADIE) & ADCSRA;}
//------------------------------------------------------------------------------
// initialize ADC and timer1
void adcInit(metadata_t* meta) {
  uint8_t adps;  // prescaler bits for ADCSRA
  uint32_t ticks = F_CPU*SAMPLE_INTERVAL + 0.5;  // Sample interval cpu cycles.

  if (ADC_REF & ~((1 << REFS0) | (1 << REFS1))) {
    error("Invalid ADC reference");
  }
#ifdef ADC_PRESCALER
  if (ADC_PRESCALER > 7 || ADC_PRESCALER < 2) {
    error("Invalid ADC prescaler");
  }
  adps = ADC_PRESCALER;
#else  // ADC_PRESCALER
  // Allow extra cpu cycles to change ADC settings if more than one pin.
  int32_t adcCycles = (ticks - ISR_TIMER0)/PIN_COUNT - ISR_SETUP_ADC;

  for (adps = 7; adps > 0; adps--) {
    if (adcCycles >= (MIN_ADC_CYCLES << adps)) {
      break;
    }
  }
#endif  // ADC_PRESCALER
  meta->adcFrequency = F_CPU >> adps;
  if (meta->adcFrequency > (RECORD_EIGHT_BITS ? 2000000 : 1000000)) {
    error("Sample Rate Too High");
  }
#if ROUND_SAMPLE_INTERVAL
  // Round so interval is multiple of ADC clock.
  ticks += 1 << (adps - 1);
  ticks >>= adps;
  ticks <<= adps;
#endif  // ROUND_SAMPLE_INTERVAL

  if (PIN_COUNT > BLOCK_MAX_COUNT || PIN_COUNT > PIN_NUM_DIM) {
    error("Too many pins");
  }
  meta->pinCount = PIN_COUNT;
  meta->recordEightBits = RECORD_EIGHT_BITS;

  for (int i = 0; i < PIN_COUNT; i++) {
    uint8_t pin = PIN_LIST[i];
    if (pin >= NUM_ANALOG_INPUTS) {
      error("Invalid Analog pin number");
    }
    meta->pinNumber[i] = pin;

    // Set ADC reference and low three bits of analog pin number.
    adcmux[i] = (pin & 7) | ADC_REF;
    if (RECORD_EIGHT_BITS) {
      adcmux[i] |= 1 << ADLAR;
    }

    // If this is the first pin, trigger on timer/counter 1 compare match B.
    adcsrb[i] = i == 0 ? (1 << ADTS2) | (1 << ADTS0) : 0;
#ifdef MUX5
    if (pin > 7) {
      adcsrb[i] |= (1 << MUX5);
    }
#endif  // MUX5
    adcsra[i] = (1 << ADEN) | (1 << ADIE) | adps;
    // First pin triggers on timer 1 compare match B rest are free running.
    adcsra[i] |= i == 0 ? 1 << ADATE : 1 << ADSC;
  }

  // Setup timer1
  TCCR1A = 0;
  uint8_t tshift;
  if (ticks < 0X10000) {
    // no prescale, CTC mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
    tshift = 0;
  } else if (ticks < 0X10000*8) {
    // prescale 8, CTC mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    tshift = 3;
  } else if (ticks < 0X10000*64) {
    // prescale 64, CTC mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
    tshift = 6;
  } else if (ticks < 0X10000*256) {
    // prescale 256, CTC mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12);
    tshift = 8;
  } else if (ticks < 0X10000*1024) {
    // prescale 1024, CTC mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);
    tshift = 10;
  } else {
    error("Sample Rate Too Slow");
  }
  // divide by prescaler
  ticks >>= tshift;
  // set TOP for timer reset
  ICR1 = ticks - 1;
  // compare for ADC start
  OCR1B = 0;

  // multiply by prescaler
  ticks <<= tshift;

  // Sample interval in CPU clock ticks.
  meta->sampleInterval = ticks;
  meta->cpuFrequency = F_CPU;
  float sampleRate = (float)meta->cpuFrequency/meta->sampleInterval;
  Serial.print(F("Sample pins:"));
  for (uint8_t i = 0; i < meta->pinCount; i++) {
    Serial.print(' ');
    Serial.print(meta->pinNumber[i], DEC);
  }
  Serial.println();
  Serial.print(F("ADC bits: "));
  Serial.println(meta->recordEightBits ? 8 : 10);
  Serial.print(F("ADC clock kHz: "));
  Serial.println(meta->adcFrequency/1000);
  Serial.print(F("Sample Rate: "));
  Serial.println(sampleRate);
  Serial.print(F("Sample interval usec: "));
  Serial.println(1000000.0/sampleRate);
}
//------------------------------------------------------------------------------
// enable ADC and timer1 interrupts
void adcStart() {
  // initialize ISR
  adcindex = 1;
  isrBuf = nullptr;
  isrOver = 0;
  isrStop = false;

  // Clear any pending interrupt.
  ADCSRA |= 1 << ADIF;

  // Setup for first pin.
  ADMUX = adcmux[0];
  ADCSRB = adcsrb[0];
  ADCSRA = adcsra[0];

  // Enable timer1 interrupts.
  timerError = false;
  timerFlag = false;
  TCNT1 = 0;
  TIFR1 = 1 << OCF1B;
  TIMSK1 = 1 << OCIE1B;
}
//------------------------------------------------------------------------------
inline void adcStop() {
  TIMSK1 = 0;
  ADCSRA = 0;
}
//------------------------------------------------------------------------------
// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  block_t* pd;
  metadata_t* pm;
  uint32_t t0 = millis();
  // Use fast buffered print class.
  BufferedPrint<file_t, 64> bp(&csvFile);
  block_t binBuffer[FIFO_DIM];

  assert(sizeof(block_t) == sizeof(metadata_t));
  binFile.rewind();
  uint32_t tPct = millis();
  bool doMeta = true;
  while (!Serial.available()) {
    pd = binBuffer;
    int nb = binFile.read(binBuffer, sizeof(binBuffer));
    if (nb < 0) {
      error("read binFile failed");
    }
    size_t nd = nb/sizeof(block_t);
    if (nd < 1) {
      break;
    }
    if (doMeta) {
      doMeta = false;
      pm = (metadata_t*)pd++;
      if (PIN_COUNT != pm->pinCount) {
        error("Invalid pinCount");
      }
      bp.print(F("Interval,"));
      float intervalMicros = 1.0e6*pm->sampleInterval/(float)pm->cpuFrequency;
      bp.print(intervalMicros, 4);
      bp.println(F(",usec"));
      for (uint8_t i = 0; i < PIN_COUNT; i++) {
        if (i) {
          bp.print(',');
        }
        bp.print(F("pin"));
        bp.print(pm->pinNumber[i]);
      }
      bp.println();
      if (nd-- == 1) {
        break;
      }
    }
    for (size_t i = 0; i < nd; i++, pd++) {
      if (pd->overrun) {
        bp.print(F("OVERRUN,"));
        bp.println(pd->overrun);
      }
      for (size_t j = 0; j < pd->count; j += PIN_COUNT) {
        for (size_t i = 0; i < PIN_COUNT; i++) {
          if (!bp.printField(pd->data[i + j], i == (PIN_COUNT-1) ? '\n' : ',')) {
            error("printField failed");
          }
        }
      }
    }
    if ((millis() - tPct) > 1000) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        Serial.print(pct, DEC);
        Serial.println('%');
      }
    }
  }
  if (!bp.sync() || !csvFile.close()) {
    error("close csvFile failed");
  }
  Serial.print(F("Done: "));
  Serial.print(0.001*(millis() - t0));
  Serial.println(F(" Seconds"));
}
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
void createBinFile() {
  binFile.close();
  while (sd.exists(binName)) {
    char* p = strchr(binName, '.');
    if (!p) {
      error("no dot in filename");
    }
    while (true) {
      p--;
      if (p < binName || *p < '0' || *p > '9') {
        error("Can't create file name");
      }
      if (p[0] != '9') {
        p[0]++;
        break;
      }
      p[0] = '0';
    }
  }
  Serial.print(F("Opening: "));
  Serial.println(binName);
  if (!binFile.open(binName, O_RDWR | O_CREAT)) {
    error("open binName failed");
  }
  Serial.print(F("Allocating: "));
  Serial.print(MAX_FILE_SIZE_MiB);
  Serial.println(F(" MiB"));
  if (!binFile.preAllocate(MAX_FILE_SIZE)) {
    error("preAllocate failed");
  }
}
//------------------------------------------------------------------------------
bool createCsvFile() {
  char csvName[NAME_DIM];

  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return false;
  }
  binFile.getName(csvName, sizeof(csvName));
  char* dot = strchr(csvName, '.');
  if (!dot) {
    error("no dot in binName");
  }
  strcpy(dot + 1, "csv");
  if (!csvFile.open(csvName, O_WRONLY|O_CREAT|O_TRUNC)) {
    error("open csvFile failed");
  }
  Serial.print(F("Writing: "));
  Serial.print(csvName);
  Serial.println(F(" - type any character to stop"));
  return true;
}
//------------------------------------------------------------------------------
// log data
void logData() {
  uint32_t t0;
  uint32_t t1;
  uint32_t overruns =0;
  uint32_t count = 0;
  uint32_t maxLatencyUsec = 0;
  size_t maxFifoUse = 0;
  block_t fifoBuffer[FIFO_DIM];

  adcInit((metadata_t*)fifoBuffer);
  // Write metadata.
  if (sizeof(metadata_t) != binFile.write(fifoBuffer, sizeof(metadata_t))) {
    error("Write metadata failed");
  }
  fifoCount = 0;
  fifoHead = 0;
  fifoTail = 0;
  fifoData = fifoBuffer;
  // Initialize all blocks to save ISR overhead.
  memset(fifoBuffer, 0, sizeof(fifoBuffer));

  Serial.println(F("Logging - type any character to stop"));
  // Wait for Serial Idle.
  Serial.flush();
  delay(10);

  t0 = millis();
  t1 = t0;
  // Start logging interrupts.
  adcStart();
  while (1) {
    uint32_t m;
    noInterrupts();
    size_t tmpFifoCount = fifoCount;
    interrupts();
    if (tmpFifoCount) {
      block_t* pBlock = fifoData + fifoTail;
      // Write block to SD.
      m = micros();
      if (sizeof(block_t) != binFile.write(pBlock, sizeof(block_t))) {
        error("write data failed");
      }
      m = micros() - m;
      t1 = millis();
      if (m > maxLatencyUsec) {
        maxLatencyUsec = m;
      }
      if (tmpFifoCount >maxFifoUse) {
        maxFifoUse = tmpFifoCount;
      }
      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun) {
        overruns += pBlock->overrun;
        if (ERROR_LED_PIN >= 0) {
          digitalWrite(ERROR_LED_PIN, HIGH);
        }
      }
      // Initialize empty block to save ISR overhead.
      pBlock->count = 0;
      pBlock->overrun = 0;
      fifoTail = fifoTail < (FIFO_DIM - 1) ? fifoTail + 1 : 0;

      noInterrupts();
      fifoCount--;
      interrupts();

      if (binFile.curPosition() >= MAX_FILE_SIZE) {
        // File full so stop ISR calls.
        adcStop();
        break;
      }
    }
    if (timerError) {
      error("Missed timer event - rate too high");
    }
    if (Serial.available()) {
      // Stop ISR interrupts.
      isrStop = true;
    }
    if (fifoCount == 0 && !adcActive()) {
       break;
    }
  }
  Serial.println();
  // Truncate file if recording stopped early.
  if (binFile.curPosition() < MAX_FILE_SIZE) {
    Serial.println(F("Truncating file"));
    Serial.flush();
    if (!binFile.truncate()) {
      error("Can't truncate file");
    }
  }
  Serial.print(F("Max write latency usec: "));
  Serial.println(maxLatencyUsec);
  Serial.print(F("Record time sec: "));
  Serial.println(0.001*(t1 - t0), 3);
  Serial.print(F("Sample count: "));
  Serial.println(count/PIN_COUNT);
  Serial.print(F("Overruns: "));
  Serial.println(overruns);
  Serial.print(F("FIFO_DIM: "));
  Serial.println(FIFO_DIM);
  Serial.print(F("maxFifoUse: "));
  Serial.println(maxFifoUse + 1);  // include ISR use.
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void openBinFile() {
  char name[NAME_DIM];
  clearSerialInput();
  Serial.println(F("Enter file name"));
  if (!serialReadLine(name, sizeof(name))) {
    return;
  }
  if (!sd.exists(name)) {
    Serial.println(name);
    Serial.println(F("File does not exist"));
    return;
  }
  binFile.close();
  if (!binFile.open(name, O_RDWR)) {
    Serial.println(name);
    Serial.println(F("open failed"));
    return;
  }
  Serial.println(F("File opened"));
}
//------------------------------------------------------------------------------
// Print data file to Serial
void printData() {
  block_t buf;
  if (!binFile.isOpen()) {
    Serial.println(F("No current binary file"));
    return;
  }
  binFile.rewind();
  if (binFile.read(&buf , sizeof(buf)) != sizeof(buf)) {
    error("Read metadata failed");
  }
  Serial.println(F("Type any character to stop"));
  delay(1000);
  while (!Serial.available() &&
         binFile.read(&buf , sizeof(buf)) == sizeof(buf)) {
    if (buf.count == 0) {
      break;
    }
    if (buf.overrun) {
      Serial.print(F("OVERRUN,"));
      Serial.println(buf.overrun);
    }
    for (size_t i = 0; i < buf.count; i++) {
      Serial.print(buf.data[i], DEC);
      if ((i+1)%PIN_COUNT) {
        Serial.print(',');
      } else {
        Serial.println();
      }
    }
  }
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
bool serialReadLine(char* str, size_t size) {
  size_t n = 0;
  while(!Serial.available()) {
  }
  while (true) {
    int c = Serial.read();
    if (c < ' ') break;
    str[n++] = c;
    if (n >= size) {
      Serial.println(F("input too long"));
      return false;
    }
    uint32_t m = millis();
    while (!Serial.available() && (millis() - m) < 100){}
    if (!Serial.available()) break;
  }
  str[n] = 0;
  return true;
}
//------------------------------------------------------------------------------
void setup(void) {
  if (ERROR_LED_PIN >= 0) {
    pinMode(ERROR_LED_PIN, OUTPUT);
  }
  Serial.begin(9600);
  while(!Serial) {}
  Serial.println(F("Type any character to begin."));
  while(!Serial.available()) {}

  FillStack();

  // Read the first sample pin to init the ADC.
  analogRead(PIN_LIST[0]);

#if !ENABLE_DEDICATED_SPI
  Serial.println(F(
    "\nFor best performance edit SdFatConfig.h\n"
    "and set ENABLE_DEDICATED_SPI nonzero"));
#endif  // !ENABLE_DEDICATED_SPI
  // Initialize SD.
  if (!sd.begin(SD_CONFIG)) {
    error("sd.begin failed");
  }
#if USE_RTC
  if (!rtc.begin()) {
    error("rtc.begin failed");
  }
  if (!rtc.isrunning()) {
    // Set RTC to sketch compile date & time.
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    error("RTC is NOT running!");
  } else {
    Serial.println(F("RTC is running"));
  }
  // Set callback
  FsDateTime::setCallback(dateTime);
#endif  // USE_RTC
}
//------------------------------------------------------------------------------
void loop(void) {
  printUnusedStack();
  // Read any Serial data.
  clearSerialInput();
  Serial.println();
  Serial.println(F("type:"));
  Serial.println(F("b - open existing bin file"));
  Serial.println(F("c - convert file to csv"));
  Serial.println(F("l - list files"));
  Serial.println(F("p - print data to Serial"));
  Serial.println(F("r - record ADC data"));

  while(!Serial.available()) {
    yield();
  }
  char c = tolower(Serial.read());
  Serial.println();
  if (ERROR_LED_PIN >= 0) {
    digitalWrite(ERROR_LED_PIN, LOW);
  }
  // Read any Serial data.
  clearSerialInput();

  if (c == 'b') {
    openBinFile();
  } else if (c == 'c') {
    if (createCsvFile()) {
      binaryToCsv();
    }
  } else if (c == 'l') {
    Serial.println(F("ls:"));
    sd.ls(&Serial, LS_DATE | LS_SIZE);
  } else if (c == 'p') {
    printData();
  } else if (c == 'r') {
    createBinFile();
    logData();
  } else {
    Serial.println(F("Invalid entry"));
  }
}
#else  // __AVR__
#error This program is only for AVR.
#endif  // __AVR__