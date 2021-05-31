// A simple read/write example for SD.h.
// Mostly from the SD.h ReadWrite example.
//
// Your SD must be formatted FAT16/FAT32.
//
// Set USE_SD_H nonzero to use SD.h.
// Set USE_SD_H zero to use SdFat.h.
//
#define USE_SD_H 0
//
#if USE_SD_H
#include <SD.h>
#else  // USE_SD_H
#include "SdFat.h"
SdFat SD;
#endif  // USE_SD_H

// Modify SD_CS_PIN for your board.
// For Teensy 3.6 and SdFat.h use BUILTIN_SDCARD.
#define SD_CS_PIN SS

File myFile;

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

#if USE_SD_H
  Serial.println(F("Using SD.h. Set USE_SD_H zero to use SdFat.h."));
#else  // USE_SD_H
  Serial.println(F("Using SdFat.h. Set USE_SD_H nonzero to use SD.h."));
#endif  // USE_SD_H
  Serial.println(F("\nType any character to begin."));
  while (!Serial.available()) {
    yield();
  }
  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
void loop() {
  // nothing happens after setup
}