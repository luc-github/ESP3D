// This is a simple SPI loop-back test.
//
// Connect SD_MISO to SD_MOSI
//
// Modify these defines for your configuration.
#define SD_SPI SPI
#define SD_MISO MISO
#define SD_MOSI MOSI

#include "SPI.h"
void setup() {
  uint8_t rx, tx;
  Serial.begin(9600);
  while (!Serial) {
    yield();    
  }
  Serial.println(F("\nType any character to start"));
  while (!Serial.available()) {
    yield();
  }
  Serial.print("Begin, SD_MISO: ");
  Serial.print(SD_MISO), Serial.print(", SD_MOSI: ");
  Serial.println(SD_MOSI);
  pinMode(SD_MISO, INPUT_PULLUP);
  pinMode(SD_MOSI, OUTPUT);
  digitalWrite(SD_MOSI, HIGH);
  if (!digitalRead(SD_MISO)) {
    Serial.println("Error: SD_MISO not HIGH");
    goto fail;
  }
  digitalWrite(SD_MOSI, LOW);
  if (digitalRead(SD_MISO)) {
    Serial.println("Error: SD_MISO not LOW");
    goto fail;
  }
  pinMode(SD_MISO, INPUT);
  pinMode(SD_MOSI, INPUT);

  // Modify if SD_SPI.begin has arguments and use this style SdFat begin call:
  // sd.begin(SdSpiConfig(CS_PIN, USER_SPI_BEGIN | <other options>, &SD_SPI));
  SD_SPI.begin();

  // Start with a 400 kHz clock.  Try full speed if success for 400 kHz.
  SD_SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  tx = 0;
  do {
    rx = SD_SPI.transfer(tx);
    if (tx != rx) {
      Serial.print("Error rx: 0x");
      Serial.print(rx, HEX);
      Serial.print(" != tx: 0x");
      Serial.println(tx, HEX);
      SD_SPI.endTransaction();
      goto fail;
    }
  } while (tx++ < 255);
  SD_SPI.endTransaction();
  Serial.println("Success!");
  return;

fail:
  SD_SPI.endTransaction();
  Serial.println("Is SD_MISO connected to SD_MOSI?");
  Serial.println("Are SD_MISO and SD_MOSI correct?");
}
void loop() {}