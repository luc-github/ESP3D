// Test of time-stamp callback.
// Set the callback with this statement.
// FsDateTime::setCallback(dateTime);
#include "SdFat.h"
// https://github.com/adafruit/RTClib
#include "RTClib.h"
// Set RTC_TYPE for file timestamps.
// 0 - millis()
// 1 - DS1307
// 2 - DS3231
// 3 - PCF8523
#define RTC_TYPE 3

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


#if RTC_TYPE == 0
RTC_Millis rtc;
#elif RTC_TYPE == 1
RTC_DS1307 rtc;
#elif RTC_TYPE == 2
RTC_DS3231 rtc;
#elif RTC_TYPE == 3
RTC_PCF8523 rtc;
#else  // RTC_TYPE == type
#error RTC_TYPE type not implemented.
#endif  // RTC_TYPE == type

//------------------------------------------------------------------------------
// Call back for file timestamps.  Only called for file create and sync().
void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  DateTime now = rtc.now();

  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(now.year(), now.month(), now.day());

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(now.hour(), now.minute(), now.second());

  // Return low time bits in units of 10 ms, 0 <= ms10 <= 199.
  *ms10 = now.second() & 1 ? 100 : 0;
}
//------------------------------------------------------------------------------
#define error(msg) (Serial.println(F("error " msg)), false)
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
void getLine(char* line, size_t size) {
  size_t i = 0;
  uint32_t t;
  line[0] = '\0';
  while (!Serial.available()) {
    yield();
  }
  while (true) {
    t = millis() + 10;
    while (!Serial.available()) {
      if (millis() > t){
        return;
      }
    }
    int c = Serial.read();
    if (i >= (size - 1) || c == '\r' || c == '\n' ) {
      return;
    }
    line[i++] = c;
    line[i] = '\0';
  }
}
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
  DateTime now = rtc.now();
  pr->print(now.year());
  printField(pr, '-',now.month());
  printField(pr, '-',now.day());
  printField(pr, ' ',now.hour());
  printField(pr, ':',now.minute());
  printField(pr, ':',now.second());
}
//------------------------------------------------------------------------------
bool setRtc() {
  uint16_t y;
  uint8_t m, d, hh, mm, ss;
  char line[30];
  char* ptr;

  clearSerialInput();
  Serial.println(F("Enter: YYYY-MM-DD hh:mm:ss"));
  getLine(line, sizeof(line));
  Serial.print(F("Input: "));
  Serial.println(line);

  y = strtol(line, &ptr, 10);
  if (*ptr++ != '-' || y < 2000 || y > 2099) return error("year");
  m = strtol(ptr, &ptr, 10);
  if (*ptr++ != '-' || m < 1 || m > 12) return error("month");
  d = strtol(ptr, &ptr, 10);
  if (d < 1 || d > 31) return error("day");
  hh = strtol(ptr, &ptr, 10);
  if (*ptr++ != ':' || hh > 23) return error("hour");
  mm = strtol(ptr, &ptr, 10);
  if (*ptr++ != ':' || mm > 59) return error("minute");
  ss = strtol(ptr, &ptr, 10);
  if (ss > 59) return error("second");

  rtc.adjust(DateTime(y, m, d, hh, mm, ss));
  Serial.print(F("RTC set to "));
  printNow(&Serial);
  Serial.println();
  return true;
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    yield();
  }
#if RTC_TYPE == 0
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
#else  // RTC_TYPE
  if (!rtc.begin()) {
    Serial.println(F("rtc.begin failed"));
    return;
  }
  if (!rtc.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    return;
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
#endif  // RTC_TYPE
  while (true) {
    Serial.print(F("DateTime::now "));
    printNow(&Serial);
    Serial.println();
    clearSerialInput();
    Serial.println(F("Type Y to set RTC, any other character to continue"));
    while (!Serial.available()) {}
    if (Serial.read() != 'Y') break;
    if (setRtc()) break;
  }
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
void loop() {
}