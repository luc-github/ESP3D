// Print a list of error codes, symbols, and comments.
#include "SdFat.h"
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  delay(1000);
  Serial.println();
  Serial.println(F("Code,Symbol - failed operation"));
  for (uint8_t code = 0; code <= SD_CARD_ERROR_UNKNOWN; code++) {
    Serial.print(code < 16 ? "0X0" : "0X");
    Serial.print(code, HEX);
    Serial.print(",");
    printSdErrorSymbol(&Serial, code);
    Serial.print(" - ");
    printSdErrorText(&Serial, code);
    Serial.println();
  }
}
void loop() {}