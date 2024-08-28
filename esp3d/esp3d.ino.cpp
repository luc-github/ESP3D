# 1 "C:\\Users\\luc\\AppData\\Local\\Temp\\tmpqiqpm2uv"
#include <Arduino.h>
# 1 "C:/Users/luc/Documents/GitHub/ESP3D/esp3d/esp3d.ino"
# 21 "C:/Users/luc/Documents/GitHub/ESP3D/esp3d/esp3d.ino"
#include "src/core/esp3d.h"

Esp3D myesp3d;
void setup();
void loop();
#line 26 "C:/Users/luc/Documents/GitHub/ESP3D/esp3d/esp3d.ino"
void setup() { myesp3d.begin(); }


void loop() { myesp3d.handle(); }