/*
  EspLuaEngine project - example for file usage

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

// Choose your file system here
#define USE_LITTLEFS
// #define USE_SPIFFS

#ifdef USE_LITTLEFS
#include "LittleFS.h"
#define FILESYSTEM LittleFS
#elif defined(USE_SPIFFS)
#include "SPIFFS.h"
#define FILESYSTEM SPIFFS
#else
#error "No file system selected"
#endif

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

// Initialize file system
int l_initializeFileSystem(lua_State* L) {
    bool success = FILESYSTEM.begin(true);
    lua_pushboolean(L, success);
    if (!success) {
        Serial.println("Error initializing file system");
    }
    return 1;
}

// Write to file
int l_writeFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* message = luaL_checkstring(L, 2);
    File file = FILESYSTEM.open(path, FILE_WRITE);
    if (!file) {
        lua_pushboolean(L, false);
        return 1;
    }
    size_t bytesWritten = file.print(message);
    file.close();
    lua_pushboolean(L, bytesWritten > 0);
    return 1;
}

// Read from file
int l_readFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    File file = FILESYSTEM.open(path);
    if (!file) {
        lua_pushnil(L);
        return 1;
    }
    String content = file.readString();
    file.close();
    lua_pushstring(L, content.c_str());
    return 1;
}

// List files
int l_listFiles(lua_State* L) {
    File root = FILESYSTEM.open("/");
    if (!root || !root.isDirectory()) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;
    File file = root.openNextFile();
    while (file) {
        lua_pushstring(L, file.name());
        lua_rawseti(L, -2, index++);
        file = root.openNextFile();
    }
    return 1;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial connection to be established
    }
    
    // Create a new Lua environment
    luaEsp = new EspLuaEngine();
    
    // Register functions
    luaEsp->registerFunction("print", l_print);
    luaEsp->registerFunction("initializeFileSystem", l_initializeFileSystem);
    luaEsp->registerFunction("writeFile", l_writeFile);
    luaEsp->registerFunction("readFile", l_readFile);
    luaEsp->registerFunction("listFiles", l_listFiles);

    // Lua script to demonstrate file system operations
    const char* script = R"(
        print("Initializing file system...")
        if initializeFileSystem() then
            print("File system initialized successfully")

            print("Writing to file...")
            local success = writeFile("/test.txt", "Hello from Lua on ESP32!")
            if success then
                print("File written successfully")

                print("Reading file content:")
                local content = readFile("/test.txt")
                print(content)

                print("Listing all files:")
                local files = listFiles()
                for i, file in ipairs(files) do
                    print(i, file)
                end
            else
                print("Error writing to file")
            end
        else
            print("Error initializing file system")
        end

        print("File system demonstration completed.")
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
