/*
  esp3d_messageFifo.h -  class for managing messages list, thread safe

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
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP8285)

#include <Arduino.h>

#include <queue>

#include "../include/esp3d_config.h"
#include "esp3d_message.h"
#if defined(ARDUINO_ARCH_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif  // ARDUINO_ARCH_ESP32

#if defined(ARDUINO_ARCH_ESP8266)
// To avoid compilation error on ESP8266
//  and to use many ifdefs
#ifndef pdTRUE
#define pdTRUE true
#define xSemaphoreTake(A, B) true
#define xSemaphoreGive(A)
#define xSemaphoreCreateMutex(A) 0
#define vSemaphoreDelete(A)
#define SemaphoreHandle_t void*
#endif  // pdTRUE
#endif  // ESP8266

class ESP3DMessageFIFO {
 public:
  ESP3DMessageFIFO(size_t maxSize = 5) {
    _mutex = xSemaphoreCreateMutex();
    _maxSize = maxSize;
  }

  ~ESP3DMessageFIFO() {
    clear();
    vSemaphoreDelete(_mutex);
  }
  void setId(String id) { _id = id; }
  String getId() { return _id; }

  void setMaxSize(size_t maxSize) { _maxSize = maxSize; }
  size_t getMaxSize() { return _maxSize; }

  void push(ESP3DMessage* message) {
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      esp3d_log("push to list [%s] size: %d", _id.c_str(), fifo.size());
      if (fifo.size() >= _maxSize && _maxSize != 0) {
        esp3d_log("remove oldest message to make room for new one");
        ESP3DMessage* oldestMessage = fifo.front();
        fifo.pop();
        esp3d_message_manager.deleteMsg(oldestMessage);
        esp3d_log("oldest message removed, list [%s] size: %d", _id.c_str(),
                  fifo.size());
      }
      fifo.push(message);
      esp3d_log("push to list [%s] size: %d", _id.c_str(), fifo.size());
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("push to list [%s] failed, list size: %d", _id.c_str(),
                  fifo.size());
      esp3d_log_e("Delete message");
      esp3d_message_manager.deleteMsg(message);
    }
  }

  ESP3DMessage* pop() {
    ESP3DMessage* message = nullptr;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      if (!fifo.empty()) {
        esp3d_log("pop from list [%s] size: %d", _id.c_str(), fifo.size());
        message = fifo.front();
        fifo.pop();
        esp3d_log("Now list [%s] size: %d", _id.c_str(), fifo.size());
        esp3d_log("Message: %s", (const char*)message->data);
      }
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("pop from list [%s] failed, list size: %d", _id.c_str(),
                  fifo.size());
    }
    return message;
  }

  bool isEmpty() {
    bool empty = true;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      empty = fifo.empty();
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
    return empty;
  }

  size_t size() {
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
        ESP3DMessage* message = fifo.front();
        fifo.pop();
        esp3d_message_manager.deleteMsg(message);
      }
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
  }

  bool applyToEach(std::function<bool(ESP3DMessage*)> fn,
                   bool stopOnFalse = true) {
    bool result = false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
      result = true;
      size_t size = fifo.size();
      for (size_t i = 0; i < size; ++i) {
        ESP3DMessage* message = fifo.front();
        bool result = fn(message);
        fifo.pop();
        if (!result && stopOnFalse) {
          result = false;
          break;
        }
      }
      xSemaphoreGive(_mutex);
    } else {
      esp3d_log_e("Mutex not taken");
    }
    return result;
  }

 private:
  std::queue<ESP3DMessage*> fifo;
  SemaphoreHandle_t _mutex;
  size_t _maxSize;
  String _id;
};

#endif  // !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP8285)