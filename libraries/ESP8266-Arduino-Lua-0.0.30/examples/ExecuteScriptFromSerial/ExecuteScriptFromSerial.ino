#include <LuaWrapper.h>

LuaWrapper lua;

void setup() {
  Serial.begin(115200);
}

void loop() {
  String script = "";
  char c = 0;
  Serial.println();
  Serial.println("# Enter the lua script and press Control-D when finished:");
  while(1) {
    if(Serial.available() > 0) {
      c = Serial.read();
      if(c == 4) {
        break;
      }
      Serial.write(c);
      script += c;
      if(c == '\r') {
        Serial.write('\n');
        script += '\n';
      }
    }   
  }
  if(script.length() > 0) {
    Serial.println();
    Serial.println("# Executing script:");
    Serial.println(lua.Lua_dostring(&script));
  }
}
