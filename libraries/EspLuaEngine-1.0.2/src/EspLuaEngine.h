/*
  EspLuaEngine project

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

#pragma once
#define LUA_USE_C89
#include <Arduino.h>

#include <atomic>
#include <functional>

#include "lua-5.4.7/src/lua.hpp"
#if defined(ARDUINO_ARCH_ESP32)
#define ESP_LUA_CHECK_INTERVAL pdMS_TO_TICKS(10)
#endif  // defined(ARDUINO_ARCH_ESP32)

#if defined(ARDUINO_ARCH_ESP8266)
#define ESP_LUA_CHECK_INTERVAL 10
#endif  // defined(ARDUINO_ARCH_ESP8266)



class EspLuaEngine {
 public:
  enum class Status {
    Idle,
    Running,
    Paused,
  };
  EspLuaEngine();
  ~EspLuaEngine();

  using PauseFunction = std::function<void(void)>;

  bool executeScript(const char* script);
  bool registerFunction(const char* name, lua_CFunction function,
                        void* userData = nullptr);
  template <typename T>
  bool registerConstant(const char* name, T value);
  lua_State* getLuaState() { return _lua_state; }
  const char* getLastError() { return _lastError.c_str(); }
  void resetState();

  void setPauseFunction(PauseFunction func);

  static void pauseExecution();
  static void resumeExecution();
  static void stopExecution();
  bool isPaused();
  bool isRunning();
  Status getStatus();
  bool hasError();

 private:
  lua_State* _lua_state;
  static PauseFunction _pauseFunction;
  static String _lastError;
  static inline std::atomic<bool> _isPaused{false};
  static inline std::atomic<bool> _isRunning{false};
 
  static void hookFunction(lua_State* L, lua_Debug* ar);
  static void _defaultPauseFunction();
  void _loadLibraries();
  bool _checkPreconditions(const char* name);
  bool _verifyGlobal(const char* name, int type);
  void _cleanupOnFailure(const char* name, int top);
  bool _makeReadOnly(const char* name);
  bool _registerConstantImpl(const char* name, lua_Number value);
  bool _registerConstantImpl(const char* name, const char* value);
  bool _registerConstantImpl(const char* name, bool value);
  bool _registerConstantImpl(const char* name, int value);
};

using EspLuaStatus = EspLuaEngine::Status;
