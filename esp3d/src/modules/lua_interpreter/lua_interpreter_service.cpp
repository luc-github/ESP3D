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

bool LuaInterpreter::executeScriptAsync(const char* script) {
    if (_isRunning) {
        strncpy(_lastError, "A script is already running", sizeof(_lastError));
        return false;
    }

    if (strlen(script) >= sizeof(_currentScriptName)) {
        strncpy(_lastError, "Script name too long", sizeof(_lastError));
        return false;
    }

    if (!createScriptTask()) {
        strncpy(_lastError, "Failed to create script task", sizeof(_lastError));
        return false;
    }

    strncpy(_currentScriptName, script, sizeof(_currentScriptName));
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
        strncpy(_lastError, "Script aborted", sizeof(_lastError));
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

    if (xQueueReceive(self->_scriptQueue, &script, portMAX_DELAY) == pdPASS) {
        if (!self->_luaEngine.executeScript(script)) {
            strncpy(self->_lastError, "Script execution failed", sizeof(self->_lastError));
        }
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

bool LuaInterpreter::isScriptRunning() const {
    return _isRunning;
}

bool LuaInterpreter::isScriptPaused() const {
    return _isPaused;
}

void LuaInterpreter::setupFunctions() {
    _luaEngine.registerFunction("pinMode", l_pinMode);
    _luaEngine.registerFunction("digitalWrite", l_digitalWrite);
    _luaEngine.registerFunction("digitalRead", l_digitalRead);
    _luaEngine.registerFunction("analogWrite", l_analogWrite);
    _luaEngine.registerFunction("analogRead", l_analogRead);
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
#endif  // ESP_LUA_INTERPRETER_FEATURE
