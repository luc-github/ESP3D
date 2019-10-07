#include <LuaWrapper.h>

LuaWrapper lua;

void setup() {
  Serial.begin(115200);
  String script = String("print('Hello world!')");
  Serial.println(lua.Lua_dostring(&script));
}

void loop() {

}
