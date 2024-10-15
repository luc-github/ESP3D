/*
  gcode_script_fifo.h -  class for managing script list, thread safe

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
#include <Arduino.h>
#include <queue>

#include "../../include/esp3d_config.h"
#include "../authentication/authentication_service.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif  // ARDUINO_ARCH_ESP32

#if defined(ARDUINO_ARCH_ESP8266)
// Définitions pour ESP8266 comme dans votre code original
#ifndef pdTRUE
#define pdTRUE true
#define xSemaphoreTake(A, B) true
#define xSemaphoreGive(A)
#define xSemaphoreCreateMutex(A) 0
#define vSemaphoreDelete(A)
#define SemaphoreHandle_t void*
#endif  // pdTRUE
#endif  // ESP8266

struct ScriptEntry {
    String script;
    ESP3DAuthenticationLevel auth_type;

    ScriptEntry(String s, ESP3DAuthenticationLevel a) : script(std::move(s)), auth_type(a) {}
};

class ESP3DScriptFIFO {
 public:
  ESP3DScriptFIFO(size_t maxSize = 5) : _maxSize(maxSize) {
    _mutex = xSemaphoreCreateMutex();
  }

  ~ESP3DScriptFIFO() {
    clear();
    vSemaphoreDelete(_mutex);
  }

  void setMaxSize(size_t maxSize) { _maxSize = maxSize; }
  size_t getMaxSize() const { return _maxSize; }

  void push(String script, ESP3DAuthenticationLevel auth_type) {
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      esp3d_log("push to list [%s] auth: %d, size: %d", script.c_str(), auth_type, fifo.size());
      if (fifo.size() >= _maxSize && _maxSize != 0) {
        esp3d_log("remove oldest script to make room for new one");
        fifo.pop();
        esp3d_log("oldest message removed, list size: %d", fifo.size());
      }
      fifo.emplace(std::move(script), auth_type);
      esp3d_log("push to list size: %d", fifo.size());
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("push to list [%s] id: %d failed, list size: %d", script.c_str(), auth_type, fifo.size());
    }
  }

  ScriptEntry pop() {
    ScriptEntry entry{String(""), ESP3DAuthenticationLevel::guest};
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      if (!fifo.empty()) {
        esp3d_log("pop from list size: %d", fifo.size());
        entry = std::move(fifo.front());
        fifo.pop();
        esp3d_log("Now list size: %d", fifo.size());
        esp3d_log("Script: %s, ID: %d", entry.script.c_str(), entry.auth_type);
      }
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("pop from list failed, list size: %d", fifo.size());
    }
    return entry;
  }

  bool isEmpty() const {
    bool empty = true;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      empty = fifo.empty();
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
    return empty;
  }

  size_t size() const {
    size_t s = 0;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      s = fifo.size();
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
    return s;
  }

  void clear() {
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      while (!fifo.empty()) {
        fifo.pop();
      }
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
  }

 private:
  std::queue<ScriptEntry> fifo;
  SemaphoreHandle_t _mutex;
  size_t _maxSize;
};
