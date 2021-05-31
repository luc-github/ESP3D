/*
 * This program demonstrates use of rename().
 */
#include "SdFat.h"
#include "sdios.h"

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


// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
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

// Serial print stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  cout << F("Insert an empty SD.  Type any character to start.") << endl;
  while (!Serial.available()) {
    SysCall::yield();
  }

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  // Remove file/dirs from previous run.
  if (sd.exists("dir2/DIR3/NAME3.txt")) {
    cout << F("Removing /dir2/DIR3/NAME3.txt") << endl;
    if (!sd.remove("dir2/DIR3/NAME3.txt") ||
        !sd.rmdir("dir2/DIR3/") ||
        !sd.rmdir("dir2/")) {
      error("remove/rmdir failed");
    }
  }
  // create a file and write one line to the file
  if (!file.open("Name1.txt", O_WRONLY | O_CREAT)) {
    error("Name1.txt");
  }
  file.println("A test line for Name1.txt");

  // rename the file name2.txt and add a line.
  if (!file.rename("name2.txt")) {
    error("name2.txt");
  }
  file.println("A test line for name2.txt");

  // list files
  cout << F("------") << endl;
  sd.ls(LS_R);

  // make a new directory - "Dir1"
  if (!sd.mkdir("Dir1")) {
    error("Dir1");
  }

  // move file into Dir1, rename it NAME3.txt and add a line
  if (!file.rename("Dir1/NAME3.txt")) {
    error("NAME3.txt");
  }
  file.println("A line for Dir1/NAME3.txt");

  // list files
  cout << F("------") << endl;
  sd.ls(LS_R);

  // make directory "dir2"
  if (!sd.mkdir("dir2")) {
    error("dir2");
  }

  // close file before rename(oldPath, newPath)
  file.close();

  // move Dir1 into dir2 and rename it DIR3
  if (!sd.rename("Dir1", "dir2/DIR3")) {
    error("dir2/DIR3");
  }

  // open file for append in new location and add a line
  if (!file.open("dir2/DIR3/NAME3.txt", O_WRONLY | O_APPEND)) {
    error("dir2/DIR3/NAME3.txt");
  }
  file.println("A line for dir2/DIR3/NAME3.txt");
  file.close();

  // list files
  cout << F("------") << endl;
  sd.ls(LS_R);

  cout << F("Done") << endl;
}
void loop() {}