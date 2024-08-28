/*
  lua_interpreter_service.h - ESP3D lua interpreter service class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#pragma once
#if defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <EspLuaEngine.h>

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_messageFifo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum class Lua_Filesystem_Type : uint8_t {
  none = 0,
  fLash = 1,
  sd = 2,
};

class LuaInterpreter {
 public:
  LuaInterpreter();
  ~LuaInterpreter();

  bool executeScriptAsync(const char* script);
  void abortScript();
  bool pauseScript();
  bool resumeScript();
  const char* getCurrentScriptName() { return _currentScriptName.c_str(); }
  uint64_t getExecutionTime();
  bool isScriptRunning();
  bool isScriptPaused();
  const char* getLastError(); 
  bool dispatch(ESP3DMessage* message);
  bool begin();
  void end();
  void handle();

 private:
  EspLuaEngine _luaEngine;
  TaskHandle_t _scriptTask;
  char* _scriptBuffer;
  SemaphoreHandle_t _stateMutex;
  ESP3DMessageFIFO _messageInFIFO;
  ESP3DMessageFIFO _messageOutFIFO;
  Lua_Filesystem_Type _luaFSType;
  String _currentScriptName;
  unsigned long _startTime;
  unsigned long _pauseTime;
  String _lastError;

  static void scriptExecutionTask(void* parameter);
  void setupFunctions();
  void registerConstants();
  bool createScriptTask();
  void deleteScriptTask();
  void resetLuaEnvironment();

  // Wrappers
  static int l_print(lua_State* L);
  static int l_pinMode(lua_State* L);
  static int l_digitalWrite(lua_State* L);
  static int l_digitalRead(lua_State* L);
  static int l_analogWrite(lua_State* L);
  static int l_analogRead(lua_State* L);
  static int l_available(lua_State* L);
  static int l_readData(lua_State* L);
  static int l_delay(lua_State* L);
  static int l_yield(lua_State* L);
  static int l_millis(lua_State* L);
};

extern LuaInterpreter esp3d_lua_interpreter;

#endif  // defined(ARDUINO_ARCH_ESP32)