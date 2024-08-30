// Quick hardware test for SPI card access.
//
#include <SPI.h>

#include "SdFat.h"
#include "sdios.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
//
// Set DISABLE_CHIP_SELECT to disable a second SPI device.
// For example, with the Ethernet shield, set DISABLE_CHIP_SELECT
// to 10 to disable the Ethernet controller.
const int8_t DISABLE_CHIP_SELECT = -1;
//
// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(4)
//------------------------------------------------------------------------------
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
// Serial streams
ArduinoOutStream cout(Serial);

// input buffer for line
char cinBuf[40];
ArduinoInStream cin(Serial, cinBuf, sizeof(cinBuf));

// SD card chip select
int chipSelect;

void cardOrSpeed() {
  cout << F("Try another SD card or reduce the SPI bus speed.\n");
  cout << F("Edit SPI_SPEED in this program to change it.\n");
}

void clearSerialInput() {
  uint32_t m = micros();
  do {
    if (Serial.read() >= 0) {
      m = micros();
    }
  } while (micros() - m < 10000);
}

void reformatMsg() {
  cout << F("Try reformatting the card.  For best results use\n");
  cout << F("the SdFormatter program in SdFat/examples or download\n");
  cout << F("and use SDFormatter from www.sdcard.org/downloads.\n");
}

void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  cout << F("\nSPI pins:\n");
  cout << F("MISO: ") << int(MISO) << endl;
  cout << F("MOSI: ") << int(MOSI) << endl;
  cout << F("SCK:  ") << int(SCK) << endl;
  cout << F("SS:   ") << int(SS) << endl;
#ifdef SDCARD_SS_PIN
  cout << F("SDCARD_SS_PIN:   ") << int(SDCARD_SS_PIN) << endl;
#endif  // SDCARD_SS_PIN

  if (DISABLE_CHIP_SELECT < 0) {
    cout << F(
        "\nBe sure to edit DISABLE_CHIP_SELECT if you have\n"
        "a second SPI device.  For example, with the Ethernet\n"
        "shield, DISABLE_CHIP_SELECT should be set to 10\n"
        "to disable the Ethernet controller.\n");
  }
  cout << F(
      "\nSD chip select is the key hardware option.\n"
      "Common values are:\n"
      "Arduino Ethernet shield, pin 4\n"
      "Sparkfun SD shield, pin 8\n"
      "Adafruit SD shields and modules, pin 10\n");
}

bool firstTry = true;
void loop() {
  // Read any existing Serial data.
  clearSerialInput();

  if (!firstTry) {
    cout << F("\nRestarting\n");
  }
  firstTry = false;

  cout << F("\nEnter the chip select pin number: ");
  while (!Serial.available()) {
    yield();
  }
  cin.readline();
  if (cin >> chipSelect) {
    cout << chipSelect << endl;
  } else {
    cout << F("\nInvalid pin number\n");
    return;
  }
  if (DISABLE_CHIP_SELECT < 0) {
    cout << F(
        "\nAssuming the SD is the only SPI device.\n"
        "Edit DISABLE_CHIP_SELECT to disable another device.\n");
  } else {
    cout << F("\nDisabling SPI device on pin ");
    cout << int(DISABLE_CHIP_SELECT) << endl;
    pinMode(DISABLE_CHIP_SELECT, OUTPUT);
    digitalWrite(DISABLE_CHIP_SELECT, HIGH);
  }
  if (!sd.begin(chipSelect, SPI_SPEED)) {
    if (sd.card()->errorCode()) {
      cout << F(
          "\nSD initialization failed.\n"
          "Do not reformat the card!\n"
          "Is the card correctly inserted?\n"
          "Is chipSelect set to the correct value?\n"
          "Does another SPI device need to be disabled?\n"
          "Is there a wiring/soldering problem?\n");
      cout << F("\nerrorCode: ") << hex << showbase;
      cout << int(sd.card()->errorCode());
      cout << F(", errorData: ") << int(sd.card()->errorData());
      cout << dec << noshowbase << endl;
      return;
    }
    cout << F("\nCard successfully initialized.\n");
    if (sd.vol()->fatType() == 0) {
      cout << F("Can't find a valid FAT16/FAT32/exFAT partition.\n");
      reformatMsg();
      return;
    }
    cout << F("Can't determine error type\n");
    return;
  }
  cout << F("\nCard successfully initialized.\n");
  cout << endl;

  uint32_t size = sd.card()->sectorCount();
  if (size == 0) {
    cout << F("Can't determine the card size.\n");
    cardOrSpeed();
    return;
  }
  uint32_t sizeMB = 0.000512 * size + 0.5;
  cout << F("Card size: ") << sizeMB;
  cout << F(" MB (MB = 1,000,000 bytes)\n");
  cout << endl;
  if (sd.fatType() <= 32) {
    cout << F("\nVolume is FAT") << int(sd.fatType());
  } else {
    cout << F("\nVolume is exFAT");
  }
  cout << F(", Cluster size (bytes): ") << sd.vol()->bytesPerCluster();
  cout << endl << endl;

  cout << F("Files found (date time size name):\n");
  sd.ls(LS_R | LS_DATE | LS_SIZE);

  if ((sizeMB > 1100 && sd.vol()->sectorsPerCluster() < 64) ||
      (sizeMB < 2200 && sd.vol()->fatType() == 32)) {
    cout << F("\nThis card should be reformatted for best performance.\n");
    cout << F("Use a cluster size of 32 KB for cards larger than 1 GB.\n");
    cout << F("Only cards larger than 2 GB should be formatted FAT32.\n");
    reformatMsg();
    return;
  }
  // Read any extra Serial data.
  clearSerialInput();

  cout << F("\nSuccess!  Type any character to restart.\n");
  while (!Serial.available()) {
    yield();
  }
}