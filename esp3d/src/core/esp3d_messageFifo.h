/*
  esp3d_messageFifo.h -  class for handeling message

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
#if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP8285)

#include "esp3d_message.h"
#include "../include/esp3d_config.h"
#include <Arduino.h>

#include <queue>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ESP3DMessageFIFO {
public:
    ESP3DMessageFIFO(size_t maxSize = 5) : maxSize(maxSize) {
        mutex = xSemaphoreCreateMutex();
    }

    ~ESP3DMessageFIFO() {
        clear();
        vSemaphoreDelete(mutex);
    }

    void push(ESP3DMessage* message) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (fifo.size() >= maxSize) {
            ESP3DMessage* oldestMessage = fifo.front();
            fifo.pop();
            ESP3DMessageManager::deleteMsg(oldestMessage);
        }
        fifo.push(message);
        xSemaphoreGive(mutex);
    }

    ESP3DMessage* pop() {
        ESP3DMessage* message = nullptr;
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (!fifo.empty()) {
            message = fifo.front();
            fifo.pop();
        }
        xSemaphoreGive(mutex);
        return message;
    }

    bool isEmpty() {
        bool empty;
        xSemaphoreTake(mutex, portMAX_DELAY);
        empty = fifo.empty();
        xSemaphoreGive(mutex);
        return empty;
    }

    size_t size() {
        size_t s;
        xSemaphoreTake(mutex, portMAX_DELAY);
        s = fifo.size();
        xSemaphoreGive(mutex);
        return s;
    }

    void clear() {
        xSemaphoreTake(mutex, portMAX_DELAY);
        while (!fifo.empty()) {
            ESP3DMessage* message = fifo.front();
            fifo.pop();
            ESP3DMessageManager::deleteMsg(message);
        }
        xSemaphoreGive(mutex);
    }

private:
    std::queue<ESP3DMessage*> fifo;
    SemaphoreHandle_t mutex;
    const size_t maxSize;
};

#endif // !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP8285)