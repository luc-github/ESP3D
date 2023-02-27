/*
 * Example use of chdir(), ls(), mkdir(), and  rmdir().
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
//------------------------------------------------------------------------------

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#endif  // SD_FAT_TYPE

// Create a Serial output stream.
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  delay(1000);
  cout << F("Type any character to start\n");
  while (!Serial.available()) {
    yield();
  }

  // Initialize the SD card.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  if (sd.exists("Folder1")
    || sd.exists("Folder1/file1.txt")
    || sd.exists("Folder1/File2.txt")) {
    error("Please remove existing Folder1, file1.txt, and File2.txt");
  }

  int rootFileCount = 0;
  if (!root.open("/")) {
    error("open root");
  }
  while (file.openNext(&root, O_RDONLY)) {
    if (!file.isHidden()) {
      rootFileCount++;
    }
    file.close();
    if (rootFileCount > 10) {
      error("Too many files in root. Please use an empty SD.");
    }
  }
  if (rootFileCount) {
    cout << F("\nPlease use an empty SD for best results.\n\n");
    delay(1000);
  }
  // Create a new folder.
  if (!sd.mkdir("Folder1")) {
    error("Create Folder1 failed");
  }
  cout << F("Created Folder1\n");

  // Create a file in Folder1 using a path.
  if (!file.open("Folder1/file1.txt", O_WRONLY | O_CREAT)) {
    error("create Folder1/file1.txt failed");
  }
  file.close();
  cout << F("Created Folder1/file1.txt\n");

  // Change volume working directory to Folder1.
  if (!sd.chdir("Folder1")) {
    error("chdir failed for Folder1.\n");
  }
  cout << F("chdir to Folder1\n");

  // Create File2.txt in current directory.
  if (!file.open("File2.txt", O_WRONLY | O_CREAT)) {
    error("create File2.txt failed");
  }
  file.close();
  cout << F("Created File2.txt in current directory\n");

  cout << F("\nList of files on the SD.\n");
  sd.ls("/", LS_R);

  // Remove files from current directory.
  if (!sd.remove("file1.txt") || !sd.remove("File2.txt")) {
    error("remove failed");
  }
  cout << F("\nfile1.txt and File2.txt removed.\n");

  // Change current directory to root.
  if (!sd.chdir()) {
    error("chdir to root failed.\n");
  }

  cout << F("\nList of files on the SD.\n");
  sd.ls(LS_R);

  // Remove Folder1.
  if (!sd.rmdir("Folder1")) {
    error("rmdir for Folder1 failed\n");
  }
  cout << F("\nFolder1 removed.\n");
  cout << F("\nList of files on the SD.\n");
  sd.ls(LS_R);
  cout << F("Done!\n");
}
//------------------------------------------------------------------------------
// Nothing happens in loop.
void loop() {}
