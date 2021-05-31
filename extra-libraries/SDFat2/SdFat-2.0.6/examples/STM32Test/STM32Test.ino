/*
 * Example use of two SPI ports on an STM32 board.
 * Note SPI speed is limited to 18 MHz.
 */
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"

// Chip select PA4, shared SPI, 18 MHz, port 1.
#define SD1_CONFIG SdSpiConfig(PA4, SHARED_SPI, SD_SCK_MHZ(18), &SPI)
SdFs sd1;
FsFile file1;

// Use mySPI2 since SPI2 is used in SPI.h as a different type.
static SPIClass mySPI2(2);
// Chip select PB21, dedicated SPI, 18 MHz, port 2.
#if ENABLE_DEDICATED_SPI
#define SD2_CONFIG SdSpiConfig(PB12, DEDICATED_SPI, SD_SCK_MHZ(18), &mySPI2)
#else  // ENABLE_DEDICATED_SPI
#define SD2_CONFIG SdSpiConfig(PB12, SHARED_SPI, SD_SCK_MHZ(18), &mySPI2)
#endif  // ENABLE_DEDICATED_SPI

SdFs sd2;
FsFile file2;

const uint8_t BUF_DIM = 100;
uint8_t buf[BUF_DIM];

const uint32_t FILE_SIZE = 1000000;
const uint32_t NWRITE = FILE_SIZE/BUF_DIM;
//------------------------------------------------------------------------------
// print error msg, any SD error codes, and halt.
// store messages in flash
#define error(msg) {Serial.println(msg); errorHalt();}
void errorHalt() {
  if (sd1.sdErrorCode()) {
    sd1.errorHalt();
  }
  sd2.errorHalt();
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  Serial.print(F("FreeStack: "));
  Serial.println(FreeStack());

  // fill buffer with known data
  for (size_t i = 0; i < sizeof(buf); i++) {
    buf[i] = i;
  }

  Serial.println(F("type any character to start"));
  while (!Serial.available()) {
    SysCall::yield();
  }

  // initialize the first card
  if (!sd1.begin(SD1_CONFIG)) {
    error("sd1.begin");
  }
  // create Dir1 on sd1 if it does not exist
  if (!sd1.exists("/Dir1")) {
    if (!sd1.mkdir("/Dir1")) {
      error("sd1.mkdir");
    }
  }
  // Make Dir1 the working directory on sd1.
  if (!sd1.chdir("Dir1")) {
     error("dsd1.chdir");
  }
  // initialize the second card
  if (!sd2.begin(SD2_CONFIG)) {
    error("sd2.begin");
  }
// create Dir2 on sd2 if it does not exist
  if (!sd2.exists("/Dir2")) {
    if (!sd2.mkdir("/Dir2")) {
      error("sd2.mkdir");
    }
  }
  // Make Dir2 the working directory on sd2.
  if (!sd2.chdir("Dir2")) {
     error("sd2.chdir");
  }
  // remove test.bin from /Dir1 directory of sd1
  if (sd1.exists("test.bin")) {
    if (!sd1.remove("test.bin")) {
      error("remove test.bin");
    }
  }
  // remove rename.bin from /Dir2 directory of sd2
  if (sd2.exists("rename.bin")) {
    if (!sd2.remove("rename.bin")) {
      error("remove rename.bin");
    }
  }
  // list directories.
  Serial.println(F("------sd1 Dir1-------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 Dir2-------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));

  // create or open /Dir1/test.bin and truncate it to zero length
  if (!file1.open(&sd1, "test.bin", O_RDWR | O_CREAT | O_TRUNC)) {
    error("file1.open");
  }
  Serial.println(F("Writing test.bin to sd1"));

  // write data to /Dir1/test.bin on sd1
  for (uint32_t i = 0; i < NWRITE; i++) {
    if (file1.write(buf, sizeof(buf)) != sizeof(buf)) {
      error("file1.write");
    }
  }

  // create or open /Dir2/copy.bin and truncate it to zero length
  if (!file2.open(&sd2, "copy.bin", O_WRONLY | O_CREAT | O_TRUNC)) {
    error("file2.open");
  }
  Serial.println(F("Copying test.bin to copy.bin"));

  // copy file1 to file2
  file1.rewind();
  uint32_t t = millis();

  while (1) {
    int n = file1.read(buf, sizeof(buf));
    if (n < 0) {
      error("file1.read");
    }
    if (n == 0) {
      break;
    }
    if ((int)file2.write(buf, n) != n) {
      error("file2.write");
    }
  }
  t = millis() - t;
  Serial.print(F("File size: "));
  Serial.println(file2.fileSize());
  Serial.print(F("Copy time: "));
  Serial.print(t);
  Serial.println(F(" millis"));
  // close test.bin
  file1.close();
  // sync copy.bin so ls works.
  file2.close();
  // list directories.
  Serial.println(F("------sd1 -------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 -------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));
  Serial.println(F("Renaming copy.bin"));
  // Rename copy.bin. The renamed file will be in Dir2.
  if (!sd2.rename("copy.bin", "rename.bin")) {
    error("rename copy.bin");
  }
  file2.close();
  // list directories.
  Serial.println(F("------sd1 -------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 -------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void loop() {}