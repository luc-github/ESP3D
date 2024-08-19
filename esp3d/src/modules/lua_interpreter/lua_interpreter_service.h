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

#include <EspLuaEngine.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

class LuaInterpreter {
public:
     LuaInterpreter();
    ~LuaInterpreter();

    bool executeScriptAsync(const char* script);
    void abortCurrentScript();
    bool pauseScript();
    bool resumeScript();
    const char* getCurrentScriptName() const;
    unsigned long getExecutionTime() const;
    bool isScriptRunning() const;
    bool isScriptPaused() const;
    const char* getLastError() const;

private:
    EspLuaEngine _luaEngine;
    TaskHandle_t _scriptTask;
    QueueHandle_t _scriptQueue;
    SemaphoreHandle_t _pauseSemaphore;
    
    char _currentScriptName[64];
    unsigned long _startTime;
    unsigned long _pauseTime;
    bool _isRunning;
    bool _isPaused;
    char _lastError[128];


    static void scriptExecutionTask(void* parameter);
    void setupFunctions();
    void registerConstants();
    bool createScriptTask();
    void deleteScriptTask();
    void checkPause();

    // Wrappers
    static int l_print(lua_State* L);
    static int l_pinMode(lua_State* L);
    static int l_digitalWrite(lua_State* L);
    static int l_digitalRead(lua_State* L);
    static int l_analogWrite(lua_State* L);
    static int l_analogRead(lua_State* L);
};

extern LuaInterpreter esp3d_lua_interpreter;

