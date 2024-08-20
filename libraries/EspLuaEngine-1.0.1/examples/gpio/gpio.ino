/*
  EspLuaEngine project - example for gpio usage

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

// Function to print in the console
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

// Arduino pinMode function
int l_pinMode(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);
    pinMode(pin, mode);
    return 0;
}

// Arduino digitalWrite function
int l_digitalWrite(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    digitalWrite(pin, value);
    return 0;
}

// Arduino digitalRead function
int l_digitalRead(lua_State* L) {
    int pin = luaL_checkinteger(L, 1);
    int value = digitalRead(pin);
    lua_pushinteger(L, value);
    return 1;
}

// Arduino delay function
int l_delay(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    delay(ms);
    return 0;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial connection to be established
    }
    
    // Create a new Lua environment
    luaEsp = new EspLuaEngine();
    
    // Register Arduino functions
    luaEsp->registerFunction("print", l_print);
    luaEsp->registerFunction("pinMode", l_pinMode);
    luaEsp->registerFunction("digitalWrite", l_digitalWrite);
    luaEsp->registerFunction("digitalRead", l_digitalRead);
    luaEsp->registerFunction("delay", l_delay);
    
    // Register Arduino constants
    luaEsp->registerConstant("INPUT", INPUT);
    luaEsp->registerConstant("OUTPUT", OUTPUT);
    luaEsp->registerConstant("INPUT_PULLUP", INPUT_PULLUP);
    luaEsp->registerConstant("HIGH", HIGH);
    luaEsp->registerConstant("LOW", LOW);

    // Define pins (adjust these according to your ESP32 board)
    luaEsp->registerConstant("LED_PIN", LED_BUILTIN);  // Built-in LED on most ESP32 boards
    luaEsp->registerConstant("BUTTON_PIN", 0);  // Boot button on most ESP32 boards

    // Lua script to demonstrate GPIO operations with delay
    const char* script = R"(
        -- Setup
        print("Configuring GPIO pins...")
        pinMode(LED_PIN, OUTPUT)
        pinMode(BUTTON_PIN, INPUT_PULLUP)

        -- Function to read button, control LED, and use delay
        function checkButtonAndLED()
            local buttonState = digitalRead(BUTTON_PIN)
            if buttonState == LOW then
                print("Button pressed!")
                digitalWrite(LED_PIN, HIGH)
                delay(500)  -- Keep LED on for 500ms
                digitalWrite(LED_PIN, LOW)
                print("LED turned off")
            else
                print("Button not pressed")
            end
            delay(200)  -- Small delay to debounce and not flood the serial output
        end

        -- Main loop
        print("Starting main loop. Press the button to light up the LED.")
        for i = 1, 50 do
            print("Iteration " .. i)
            checkButtonAndLED()
        end

        print("GPIO demonstration completed.")
    )";

    // Execute the Lua script
    if (!luaEsp->executeScript(script)) {
        Serial.println("Error executing Lua script");
    }
}

void loop() {
    // The main logic is in the Lua script, so we don't need anything here
    delay(1000);
}
