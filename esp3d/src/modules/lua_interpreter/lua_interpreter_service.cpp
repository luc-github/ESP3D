/*
  lua_interpreter_service.cpp - ESP3D lua interpreter service class

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

#include "../../include/esp3d_config.h"

#ifdef ESP_LUA_INTERPRETER_FEATURE

#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_hal.h"
#include "../../core/esp3d_settings.h"
#include "lua_interpreter_service.h"

LuaInterpreter esp3d_lua_interpreter;

LuaInterpreter::LuaInterpreter()
    : _scriptTask(NULL), _isRunning(false), _isPaused(false), _pauseTime(0) {
  _scriptQueue = xQueueCreate(1, sizeof(char*));
  _pauseSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(_pauseSemaphore);  // Initialize as available
  setupFunctions();
  registerConstants();
}

LuaInterpreter::~LuaInterpreter() {
  deleteScriptTask();
  vQueueDelete(_scriptQueue);
  vSemaphoreDelete(_pauseSemaphore);
}

bool LuaInterpreter::dispatch(ESP3DMessage* message) {
  if (!message) {
    return false;
  }
  if (message->size > 0 && message->data) {
    _messageFIFO.push(message);
    return true;
  }
  return false;
}

bool LuaInterpreter::executeScriptAsync(const char* script) {
  if (_isRunning) {
    _lastError = "A script is already running";
    return false;
  }

  if (!createScriptTask()) {
    _lastError ="Failed to create script task";
    return false;
  }
  _currentScriptName = script;
  _startTime = millis();
  _isRunning = true;
  _isPaused = false;
  return xQueueSend(_scriptQueue, (void*)&script, (TickType_t)10) == pdPASS;
}

void LuaInterpreter::abortCurrentScript() {
  if (_isRunning && _scriptTask != NULL) {
    vTaskDelete(_scriptTask);
    _scriptTask = NULL;
    _isRunning = false;
    _isPaused = false;
    _lastError= "Script aborted";
  }
}

bool LuaInterpreter::pauseScript() {
  if (_isRunning && !_isPaused) {
    _isPaused = true;
    _pauseTime = millis();
    xSemaphoreTake(_pauseSemaphore, 0);
    return true;
  }
  return false;
}

bool LuaInterpreter::resumeScript() {
  if (_isRunning && _isPaused) {
    _isPaused = false;
    _startTime += (millis() - _pauseTime);
    xSemaphoreGive(_pauseSemaphore);
    return true;
  }
  return false;
}

void LuaInterpreter::deleteScriptTask() {
  if (_scriptTask != NULL) {
    vTaskDelete(_scriptTask);
    _scriptTask = NULL;
  }
}

void LuaInterpreter::checkPause() {
  if (_isPaused) {
    xSemaphoreTake(_pauseSemaphore, portMAX_DELAY);
    xSemaphoreGive(_pauseSemaphore);
  }
}

void LuaInterpreter::scriptExecutionTask(void* parameter) {
  LuaInterpreter* self = static_cast<LuaInterpreter*>(parameter);
  char* script;
  char * buffer = nullptr;
  if (xQueueReceive(self->_scriptQueue, &script, portMAX_DELAY) == pdPASS) {
    //TODO:
    //Check script name exists
    //Check script file size is under 2048 bytes
    // Create a buffer to hold the script
    //Load the script from flash / SD into char * buffer
   
    if (!self->_luaEngine.executeScript(script)) {
      self->_lastError = "Script execution failed";
    }
  }
  if (buffer) {
    free(buffer);
  }
  self->_isRunning = false;
  self->_isPaused = false;
  self->deleteScriptTask();
  vTaskDelete(NULL);
}

unsigned long LuaInterpreter::getExecutionTime() const {
  if (!_isRunning) return 0;
  if (_isPaused) return _pauseTime - _startTime;
  return millis() - _startTime;
}

bool LuaInterpreter::isScriptRunning() const { return _isRunning; }

bool LuaInterpreter::isScriptPaused() const { return _isPaused; }

void LuaInterpreter::setupFunctions() {
  _luaEngine.registerFunction("pinMode", l_pinMode);
  _luaEngine.registerFunction("digitalWrite", l_digitalWrite);
  _luaEngine.registerFunction("digitalRead", l_digitalRead);
  _luaEngine.registerFunction("analogWrite", l_analogWrite);
  _luaEngine.registerFunction("analogRead", l_analogRead);
  _luaEngine.registerFunction("available", l_available, this);
  _luaEngine.registerFunction("readData", l_readData, this);
}

void LuaInterpreter::registerConstants() {
  _luaEngine.registerConstant("INPUT", INPUT);
  _luaEngine.registerConstant("OUTPUT", OUTPUT);
  _luaEngine.registerConstant("INPUT_PULLUP", INPUT_PULLUP);
  _luaEngine.registerConstant("INPUT_PULLDOWN", INPUT_PULLDOWN);
  _luaEngine.registerConstant("HIGH", HIGH);
  _luaEngine.registerConstant("LOW", LOW);
}

// Wrapper
int LuaInterpreter::l_pinMode(lua_State* L) {
  int pin = luaL_checkinteger(L, 1);
  int mode = luaL_checkinteger(L, 2);
  pinMode(pin, mode);
  return 0;
}

int LuaInterpreter::l_digitalWrite(lua_State* L) {
  int pin = luaL_checkinteger(L, 1);
  int value = luaL_checkinteger(L, 2);
  digitalWrite(pin, value);
  return 0;
}

int LuaInterpreter::l_digitalRead(lua_State* L) {
  int pin = luaL_checkinteger(L, 1);
  int value = digitalRead(pin);
  lua_pushinteger(L, value);
  return 1;
}

int LuaInterpreter::l_analogWrite(lua_State* L) {
  int pin = luaL_checkinteger(L, 1);
  int value = luaL_checkinteger(L, 2);
  analogWrite(pin, value);
  return 0;
}

int LuaInterpreter::l_analogRead(lua_State* L) {
  int pin = luaL_checkinteger(L, 1);
  int value = analogRead(pin);
  lua_pushinteger(L, value);
  return 1;
}

int l_print(lua_State* L) {
  String dataString;
  int nargs = lua_gettop(L);
  for (int i = 1; i <= nargs; i++) {
    if (lua_isstring(L, i)) {
      dataString += lua_tostring(L, i);
    }
  }
  ESP3DMessage* msg = ESP3DMessageManager::newMsg(
      ESP3DClientType::lua_script, esp3d_commands.getOutputClient(),
      (uint8_t*)dataString.c_str(), dataString.length(),
      ESP3DAuthenticationLevel::admin);
  if (msg) {
    // process command
    esp3d_commands.process(msg);
  } else {
    esp3d_log_e("Cannot create message");
  }
  return 0;
}

int LuaInterpreter::l_available(lua_State* L) {
  LuaInterpreter* self =
      (LuaInterpreter*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushinteger(L, self->_messageFIFO.size());
  return 1;
}

int LuaInterpreter::l_readData(lua_State* L) {
  LuaInterpreter* self =
      (LuaInterpreter*)lua_touserdata(L, lua_upvalueindex(1));
  ESP3DMessage* message = self->_messageFIFO.pop();
  if (message) {
    lua_pushlstring(L, (const char*)message->data, message->size);
    ESP3DMessageManager::deleteMsg(message);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

#endif  // ESP_LUA_INTERPRETER_FEATURE
