/*
  EspLuaEngine project - Hello world example

  Copyright (c) 2024 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "EspLuaEngine.h"
#include <Arduino.h>
EspLuaEngine* luaEsp;

// If you need to print something in the console, you must add this function
int l_print(lua_State* L) {
    int nargs = lua_gettop(L);
    for (int i = 1; i <= nargs; i++) {
        if (lua_isstring(L, i)) {
            const char* s = lua_tostring(L, i);
            Serial.print(s);
            if (i < nargs) Serial.print(" ");
        }
    }
    Serial.println();
    return 0;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial connection to be established
    }
    // Create a new Lua environment
    luaEsp = new EspLuaEngine();
    // Register the print function
    luaEsp->registerFunction("print", l_print);
    // Simple usage example
    luaEsp->executeScript("print('Hello from Lua on ESP32!')");

    // Example using basic functions and string manipulation
    const char* script = R"(
        local t = {10, 20, 30, 40, 50}
        print("Table length: " .. #t)
        print("Concatenated string: " .. table.concat(t, ", "))
        print("Uppercase: " .. string.upper("lua on esp32"))
    )";
    luaEsp->executeScript(script);
}

void loop() {
    // You can add here code to execute Lua scripts in response to events or regular intervals
    delay(1000);
}
