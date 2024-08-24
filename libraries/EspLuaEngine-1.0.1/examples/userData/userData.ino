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

class Counter {
public:
    Counter() : count(0) {}
    void increment() { count++; }
    int getCount() const { return count; }
private:
    int count;
};

Counter globalCounter;
EspLuaEngine* luaEsp;

// Lua function to increment the counter
int l_incrementCounter(lua_State* L) {
    Counter* counter = (Counter*)lua_touserdata(L, lua_upvalueindex(1));
    counter->increment();
    return 0;
}

// Lua function to get the counter value
int l_getCounterValue(lua_State* L) {
    Counter* counter = (Counter*)lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, counter->getCount());
    return 1;
}

// Print function for Lua
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
        ; // Wait for serial connection
    }

    luaEsp = new EspLuaEngine();
    
    // Register functions with userdata
    luaEsp->registerFunction("incrementCounter", l_incrementCounter, &globalCounter);
    luaEsp->registerFunction("getCounterValue", l_getCounterValue, &globalCounter);
    luaEsp->registerFunction("print", l_print);

    // Execute a Lua script that uses the counter
    const char* script = R"(
        print("Initial counter value: " .. getCounterValue())
        for i = 1, 5 do
            incrementCounter()
            print("Counter value after increment: " .. getCounterValue())
        end
    )";
    luaEsp->executeScript(script);
}

void loop() {
    delay(1000);
}