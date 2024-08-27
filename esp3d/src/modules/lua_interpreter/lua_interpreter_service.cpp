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
#if defined(NOTIFICATION_FEATURE)
#include "../notifications/notifications_service.h"
#endif  // NOTIFICATION_FEATURE
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
#include "../filesystem/esp_sd.h"
#endif  // SD_DEVICE

#define ESP_LUA_MAX_SCRIPT_SIZE 2048

LuaInterpreter esp3d_lua_interpreter;

LuaInterpreter::LuaInterpreter() {
  _scriptTask = NULL;
  _isRunning = false;
  _isPaused = false;
  _pauseTime = 0;
  _pauseSemaphore = xSemaphoreCreateBinary();
  _luaFSType = Lua_Filesystem_Type::none;
  xSemaphoreGive(_pauseSemaphore);  // Initialize as available
  setupFunctions();
  registerConstants();
  _scriptBuffer = nullptr;
  _stateMutex = xSemaphoreCreateMutex();
  _messageInFIFO.setMaxSize(10);  // no limit
  _messageInFIFO.setId("in");
  _messageOutFIFO.setMaxSize(0);  // no limit
  _messageOutFIFO.setId("out");

}

LuaInterpreter::~LuaInterpreter() {
  deleteScriptTask();
  vSemaphoreDelete(_pauseSemaphore);
  vSemaphoreDelete(_stateMutex);
}

bool LuaInterpreter::dispatch(ESP3DMessage *message) {
  if (!message) {
    return false;
  }
  if (message->size > 0 && message->data) {
    _messageInFIFO.push(message);
    return true;
  }
  return false;
}

bool LuaInterpreter::createScriptTask() {
  if (_scriptTask != NULL) {
    deleteScriptTask();
  }

  BaseType_t xReturned = xTaskCreatePinnedToCore(
      scriptExecutionTask, /* Task function. */
      "LuaScriptTask",     /* name of task. */
      8192,                /* Stack size of task */
      this,                /* parameter of the task = is main or bridge*/
      1,                   /* priority of the task */
      &_scriptTask,        /* Task handle to keep track of created task */
      1                    /* Core to run the task */
  );

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
    if (_lastError == "") {
      _lastError = "Script aborted";
    }
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

void LuaInterpreter::resetLuaEnvironment() {
  _luaEngine.resetState();
  setupFunctions();
  registerConstants();
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
  if (_lastError != "") resetLuaEnvironment();
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
          esp3d_log_e("%s", self->_lastError.c_str());
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
              esp3d_log_e("%s", self->_lastError.c_str());
            } else {
              self->_luaFSType = Lua_Filesystem_Type::fLash;
            }
          } else {
            self->_lastError = "Failed to allocate memory for script";
            esp3d_log_e("%s", self->_lastError.c_str());
          }
        }
        FSfileHandle.close();
      } else {
        self->_lastError = "File is not open: " + scriptName;
        esp3d_log_e("%s", self->_lastError.c_str());
      }
    } else {
      self->_lastError = "File not found: " + scriptName;
      esp3d_log_e("%s", self->_lastError.c_str());
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
      esp3d_log_e("%s", self->_lastError.c_str());
    } else {
      if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
        self->_lastError = "SD card not present";
        esp3d_log_e("%s", self->_lastError.c_str());
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
              esp3d_log_e("%s", self->_lastError.c_str());
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
                  esp3d_log_e("%s", self->_lastError.c_str());
                } else {
                  self->_luaFSType = Lua_Filesystem_Type::sd;
                }
              } else {
                self->_lastError = "Failed to allocate memory for script";
                esp3d_log_e("%s", self->_lastError.c_str());
              }
            }
            // close the file
            SDfileHandle.close();
          } else {
            self->_lastError = "File is not open: " + scriptName;
            esp3d_log_e("%s", self->_lastError.c_str());
          }
        } else {
          self->_lastError = "File not found: " + scriptName;
          esp3d_log_e("%s", self->_lastError.c_str());
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
    esp3d_log_e("%s", self->_lastError.c_str());
  }
  // Execute the script
  if (self->_luaFSType != Lua_Filesystem_Type::none && self->_scriptBuffer) {
    if (!self->_luaEngine.executeScript(self->_scriptBuffer)) {
      self->_lastError = "Script execution failed";
      esp3d_log_e("%s", self->_lastError.c_str());
    }
  }
  self->deleteScriptTask();
}

bool LuaInterpreter::begin() {
  end();
  return true;
}

void LuaInterpreter::end() { deleteScriptTask(); }

void LuaInterpreter::handle() {
  if (_isRunning) {
    if (xSemaphoreTake(_stateMutex, portMAX_DELAY) == pdTRUE) {
      // Check if the script is in error state and if still running
      if (_isRunning && _luaEngine.isInErrorState()) {
        _lastError = _luaEngine.getLastError();
#ifdef NOTIFICATION_FEATURE
        String errorMsg = "Error: " + _lastError;
        notificationsservice.sendAutoNotification(errorMsg.c_str());
#endif  // NOTIFICATION_FEATURE
      }
      if (_messageOutFIFO.size() > 0) {
        esp3d_log_d("lua_interpreter message size %d", _messageOutFIFO.size());
      }
      ESP3DMessage *msg = _messageOutFIFO.pop();
      if (msg) {
        esp3d_log_d("Processing message: %s", msg->data);
        esp3d_commands.process(msg);
      }
      xSemaphoreGive(_stateMutex);
    }
  }
}

uint64_t LuaInterpreter::getExecutionTime() {
  if (!_isRunning) return 0;
  if (_isPaused) return _pauseTime - _startTime;
  return millis() - _startTime;
}

bool LuaInterpreter::isScriptRunning() { return _isRunning; }

bool LuaInterpreter::isScriptPaused() { return _isPaused; }

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
  _luaEngine.registerFunction("print", l_print, this);
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

int LuaInterpreter::l_print(lua_State *L) {
  esp3d_log_d("lua_interpreter output");
  LuaInterpreter *self =
      (LuaInterpreter *)lua_touserdata(L, lua_upvalueindex(1));
  String dataString;
  dataString = "";
  int nargs = lua_gettop(L);
  //esp3d_log_d("lua_interpreter output args %d", nargs);
  for (int i = 1; i <= nargs; i++) {
    //esp3d_log_d("lua_interpreter output arg %d", i);
    if (lua_isstring(L, i)) {
      dataString += lua_tostring(L, i);
    } else if (lua_isnumber(L, i)) {
      dataString += String(lua_tonumber(L, i));
    } else if (lua_isboolean(L, i)) {
      dataString += lua_toboolean(L, i) ? "true" : "false";
    } else if (lua_isnil(L, i)) {
      dataString += "nil";
    } else {
      dataString += lua_typename(L, lua_type(L, i));
    }
  }
  if (!dataString.endsWith("\n")) {
    dataString += "\n";
  }
  esp3d_log_d("lua_interpreter output %s", dataString.c_str());
  esp3d_log_d("Message Creation");
  ESP3DMessage *msg = esp3d_message_manager.newMsg(
      ESP3DClientType::lua_script, esp3d_commands.getOutputClient(),
      (uint8_t *)dataString.c_str(), dataString.length(),
      ESP3DAuthenticationLevel::admin);
  esp3d_log_d("Message created");

  if (msg) {
    // process command
    msg->type = ESP3DMessageType::unique;
    esp3d_log_d("Message sent to fifo list");
    // push to FIFO
    self->_messageOutFIFO.push(msg);
  } else {
    esp3d_log_e("Cannot create message");
  }
  return 0;
}

int LuaInterpreter::l_available(lua_State *L) {
  LuaInterpreter *self =
      (LuaInterpreter *)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushinteger(L, self->_messageInFIFO.size());
  return 1;
}

int LuaInterpreter::l_readData(lua_State *L) {
  LuaInterpreter *self =
      (LuaInterpreter *)lua_touserdata(L, lua_upvalueindex(1));
  ESP3DMessage *message = self->_messageInFIFO.pop();
  if (message) {
    lua_pushlstring(L, (const char *)message->data, message->size);
    esp3d_message_manager.deleteMsg(message);
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
