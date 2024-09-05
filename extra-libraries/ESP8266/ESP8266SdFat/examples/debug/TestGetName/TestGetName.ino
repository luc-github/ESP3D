#include "SdFat.h"
SdFs sd;
FsFile file;
const char* name[] = {
  "SFN.TXT", 
  "LongFilename.txt",
#if USE_UTF8_LONG_NAMES
  u8"très élégant.txt",
#endif  // USE_UTF8_LONG_NAMES
  nullptr};
char buf[32];
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("Type any character to begin");
  while (!Serial.available()) {}
  if (!sd.begin(SS)) {
    sd.initErrorHalt();
  }
  for (uint8_t i = 0; name[i]; i++) {
    if (!file.open(name[i], O_CREAT |O_RDWR)) {
      sd.errorHalt("open");
    }
    size_t len = strlen(name[i]);
    size_t rtn = file.getName(buf, len);
    if (rtn != 0) {
      Serial.println("fail len");
    }
    rtn = file.getName(buf, len + 1);
    if (rtn != len) {
      Serial.println("fail len + 1");
    }
    Serial.print(rtn);
    Serial.print(' ');
    Serial.println(buf);
    if (!file.remove()) {
      sd.errorHalt("remove");
    }
  }
  Serial.println("Done");
}

void loop() {}
