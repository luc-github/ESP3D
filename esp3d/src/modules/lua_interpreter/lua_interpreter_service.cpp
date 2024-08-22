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
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
#include "../filesystem/esp_sd.h"
#endif  // SD_DEVICE

#define ESP_LUA_MAX_SCRIPT_SIZE 2048

LuaInterpreter esp3d_lua_interpreter;

LuaInterpreter::LuaInterpreter()
    : _scriptTask(NULL), _isRunning(false), _isPaused(false), _pauseTime(0) {
  _pauseSemaphore = xSemaphoreCreateBinary();
  _luaFSType = Lua_Filesystem_Type::none;
  xSemaphoreGive(_pauseSemaphore);  // Initialize as available
  setupFunctions();
  registerConstants();
  _scriptBuffer = nullptr;
}

LuaInterpreter::~LuaInterpreter() {
  deleteScriptTask();
  vSemaphoreDelete(_pauseSemaphore);
}

bool LuaInterpreter::dispatch(ESP3DMessage *message) {
  if (!message) {
    return false;
  }
  if (message->size > 0 && message->data) {
    _messageFIFO.push(message);
    return true;
  }
  return false;
}

bool LuaInterpreter::createScriptTask() {
  if (_scriptTask != NULL) {
    deleteScriptTask();
  }

  BaseType_t xReturned = xTaskCreate(scriptExecutionTask, "LuaScriptTask", 8192,
                                     this, tskIDLE_PRIORITY + 1, &_scriptTask);

  if (xReturned != pdPASS) {
    _lastError = "Failed to create script task";
    return false;
  }
  _isRunning = true;
  _isPaused = false;
  _startTime = millis();
  return true;
}

bool LuaInterpreter::executeScriptAsync(const char *script) {
  bool result = true;
  if (_isRunning) {
    _lastError = "A script is already running";
    return false;
  }
  _currentScriptName = script;
  if (!createScriptTask()) {
    _lastError = "Failed to create script task";
    result = false;
  }

  return result;
}

void LuaInterpreter::abortCurrentScript() {
  if (_isRunning && _scriptTask != NULL) {
    deleteScriptTask();
    _lastError = "Script aborted";
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
  if (_scriptBuffer) {
    free(_scriptBuffer);
    _scriptBuffer = nullptr;
  }
  _isRunning = false;
  _isPaused = false;
  _luaFSType = Lua_Filesystem_Type::none;
}

void LuaInterpreter::checkPause() {
  if (_isPaused) {
    xSemaphoreTake(_pauseSemaphore, portMAX_DELAY);
    xSemaphoreGive(_pauseSemaphore);
  }
}

void LuaInterpreter::scriptExecutionTask(void *parameter) {
  LuaInterpreter *self = static_cast<LuaInterpreter *>(parameter);
  String scriptName = self->_currentScriptName;
  if (self->_scriptBuffer) {
    free(self->_scriptBuffer);
  }
  self->_lastError = "";
  size_t fileSize = 0;
  self->_luaFSType = Lua_Filesystem_Type::none;
// define fstype
#if defined(FILESYSTEM_FEATURE)
  // Check if the script is in flash
  if (self->_currentScriptName.startsWith(ESP_FLASH_FS_HEADER) ||
      self->_currentScriptName.startsWith("/")) {
    // remove the header if it exists
    if (self->_currentScriptName.startsWith(ESP_FLASH_FS_HEADER)) {
      scriptName = self->_currentScriptName.substring(
          strlen(ESP_FLASH_FS_HEADER), self->_currentScriptName.length());
    }
    // check if the file exists
    if (ESP_FileSystem::exists(scriptName.c_str())) {
      // open the file
      ESP_File FSfileHandle = ESP_FileSystem::open(scriptName.c_str());
      if (FSfileHandle.isOpen()) {
        // Check the file size
        fileSize = FSfileHandle.size();
        esp3d_log("File %s opened, size is %d", scriptName.c_str(), fileSize);
        if (fileSize > ESP_LUA_MAX_SCRIPT_SIZE) {
          self->_lastError = "File size is too large";
          esp3d_log_e(% s, self->_lastError.c_str());
        } else {
          // allocate memory for the script
          self->_scriptBuffer = (char *)malloc(fileSize + 1);
          if (self->_scriptBuffer) {
            // read the file into the buffer
            size_t readSize =
                FSfileHandle.read((uint8_t *)self->_scriptBuffer, fileSize);
            self->_scriptBuffer[fileSize] = '\0';
            esp3d_log("File %s read into buffer", scriptName.c_str());
            // check if the read is ok
            if (readSize != fileSize) {
              self->_lastError = "Failed to read file";
              esp3d_log_e(% s, self->_lastError.c_str());
            } else {
              self->_luaFSType = Lua_Filesystem_Type::fLash;
            }
          } else {
            self->_lastError = "Failed to allocate memory for script";
            esp3d_log_e(% s, self->_lastError.c_str());
          }
        }
        FSfileHandle.close();
      } else {
        self->_lastError = "File is not open: " + scriptName;
        esp3d_log_e(% s, self->_lastError.c_str());
      }
    } else {
      self->_lastError = "File not found: " + scriptName;
      esp3d_log_e(% s, self->_lastError.c_str());
    }
  }
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
  if (self->_currentScriptName.startsWith(ESP_SD_FS_HEADER)) {
    scriptName = self->_currentScriptName.substring(
        strlen(ESP_SD_FS_HEADER), self->_currentScriptName.length());
    esp3d_log("Processing SD file %s", scriptName.c_str());
    // Check if the SD file system is available
    if (!ESP_SD::accessFS()) {
      self->_lastError = "SD file system not found";
      esp3d_log_e(% s, self->_lastError.c_str());
    } else {
      if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
        self->_lastError = "SD card not present";
        esp3d_log_e(% s, self->_lastError.c_str());
      } else {
        ESP_SD::setState(ESP_SDCARD_BUSY);
        // Check script name exists
        if (ESP_SD::exists(scriptName.c_str())) {
          // open SD file
          ESP_SDFile SDfileHandle = ESP_SD::open(scriptName.c_str());
          if (SDfileHandle.isOpen()) {
            // Check script file size is under 2048 bytes
            fileSize = SDfileHandle.size();
            if (fileSize > ESP_LUA_MAX_SCRIPT_SIZE) {
              self->_lastError = "File size is too large";
              esp3d_log_e(% s, self->_lastError.c_str());
            } else {
              // allocate memory for the script
              self->_scriptBuffer = (char *)malloc(fileSize + 1);
              if (self->_scriptBuffer) {
                // read the file into the buffer
                size_t readSize =
                    SDfileHandle.read((uint8_t *)self->_scriptBuffer, fileSize);
                self->_scriptBuffer[fileSize] = '\0';
                esp3d_log("File %s read into buffer", scriptName.c_str());
                // check if the read is ok
                if (readSize != fileSize) {
                  self->_lastError = "Failed to read file";
                  esp3d_log_e(% s, self->_lastError.c_str());
                } else {
                  self->_luaFSType = Lua_Filesystem_Type::sd;
                }
              } else {
                self->_lastError = "Failed to allocate memory for script";
                esp3d_log_e(% s, self->_lastError.c_str());
              }
            }
            // close the file
            SDfileHandle.close();
          } else {
            self->_lastError = "File is not open: " + scriptName;
            esp3d_log_e(% s, self->_lastError.c_str());
          }
        } else {
          self->_lastError = "File not found: " + scriptName;
          esp3d_log_e(% s, self->_lastError.c_str());
        }
      }
      ESP_SD::releaseFS();
    }
  }
#endif  // SD_DEVICE
  // Check if the file system type is not determined
  if (self->_luaFSType == Lua_Filesystem_Type::none &&
      self->_lastError.length() == 0) {
    self->_lastError = "Cannot determine file system type";
    esp3d_log_e(% s, self->_lastError.c_str());
  }
  // Execute the script
  if (self->_luaFSType != Lua_Filesystem_Type::none && self->_scriptBuffer) {
    if (!self->_luaEngine.executeScript(self->_scriptBuffer)) {
      self->_lastError = "Script execution failed";
      esp3d_log_e(% s, self->_lastError.c_str());
    }
  }
  self->deleteScriptTask();
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
  _luaEngine.registerFunction("delay", l_delay);
  _luaEngine.registerFunction("yield", l_yield);
  _luaEngine.registerFunction("millis", l_millis);
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
int LuaInterpreter::l_pinMode(lua_State *L) {
  int pin = luaL_checkinteger(L, 1);
  int mode = luaL_checkinteger(L, 2);
  pinMode(pin, mode);
  return 0;
}

int LuaInterpreter::l_digitalWrite(lua_State *L) {
  int pin = luaL_checkinteger(L, 1);
  int value = luaL_checkinteger(L, 2);
  digitalWrite(pin, value);
  return 0;
}

int LuaInterpreter::l_digitalRead(lua_State *L) {
  int pin = luaL_checkinteger(L, 1);
  int value = digitalRead(pin);
  lua_pushinteger(L, value);
  return 1;
}

int LuaInterpreter::l_analogWrite(lua_State *L) {
  int pin = luaL_checkinteger(L, 1);
  int value = luaL_checkinteger(L, 2);
  analogWrite(pin, value);
  return 0;
}

int LuaInterpreter::l_analogRead(lua_State *L) {
  int pin = luaL_checkinteger(L, 1);
  int value = analogRead(pin);
  lua_pushinteger(L, value);
  return 1;
}

int l_print(lua_State *L) {
  String dataString;
  int nargs = lua_gettop(L);
  for (int i = 1; i <= nargs; i++) {
    if (lua_isstring(L, i)) {
      dataString += lua_tostring(L, i);
    }
  }
  ESP3DMessage *msg = ESP3DMessageManager::newMsg(
      ESP3DClientType::lua_script, esp3d_commands.getOutputClient(),
      (uint8_t *)dataString.c_str(), dataString.length(),
      ESP3DAuthenticationLevel::admin);
  if (msg) {
    // process command
    esp3d_commands.process(msg);
  } else {
    esp3d_log_e("Cannot create message");
  }
  return 0;
}

int LuaInterpreter::l_available(lua_State *L) {
  LuaInterpreter *self =
      (LuaInterpreter *)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushinteger(L, self->_messageFIFO.size());
  return 1;
}

int LuaInterpreter::l_readData(lua_State *L) {
  LuaInterpreter *self =
      (LuaInterpreter *)lua_touserdata(L, lua_upvalueindex(1));
  ESP3DMessage *message = self->_messageFIFO.pop();
  if (message) {
    lua_pushlstring(L, (const char *)message->data, message->size);
    ESP3DMessageManager::deleteMsg(message);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

int LuaInterpreter::l_delay(lua_State *L) {
  int ms = luaL_checkinteger(L, 1);
  vTaskDelay(pdMS_TO_TICKS(ms));
  return 0;
}

int LuaInterpreter::l_yield(lua_State *L) {
  taskYIELD();
  return 0;
}

int LuaInterpreter::l_millis(lua_State *L) {
  lua_pushinteger(L, millis());
  return 1;
}

#endif  // ESP_LUA_INTERPRETER_FEATURE
