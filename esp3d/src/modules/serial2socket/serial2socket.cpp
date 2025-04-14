/*
  serial2socket.cpp -  serial 2 socket functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"

#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_message.h"
#include "serial2socket.h"

Serial_2_Socket Serial2Socket;

Serial_2_Socket::Serial_2_Socket() {
  _rxBufferMutex = (void*)xSemaphoreCreateMutex();
  if (_rxBufferMutex == NULL) {
    esp3d_log_e("Serial2Socket: Failed to create RX mutex");
  }
  
  _txBufferMutex = (void*)xSemaphoreCreateMutex();
  if (_txBufferMutex == NULL) {
    esp3d_log_e("Serial2Socket: Failed to create TX mutex");
  }
  
  end();
}

Serial_2_Socket::~Serial_2_Socket() {
  if (_rxBufferMutex != NULL) {
    vSemaphoreDelete((SemaphoreHandle_t)_rxBufferMutex);
    _rxBufferMutex = NULL;
  }
  
  if (_txBufferMutex != NULL) {
    vSemaphoreDelete((SemaphoreHandle_t)_txBufferMutex);
    _txBufferMutex = NULL;
  }
  
  end();
}

void Serial_2_Socket::begin(long speed) { end(); }

void Serial_2_Socket::enable(bool enable) { _started = enable; }

void Serial_2_Socket::pause(bool state) {
  _paused = state;
  if (_paused) {
    // Protect TX buffer access with mutex
    if (_txBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_txBufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      _TXbufferSize = 0;
      xSemaphoreGive((SemaphoreHandle_t)_txBufferMutex);
    } else {
      _TXbufferSize = 0;
    }
    
    // Protect RX buffer access with mutex
    if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      _RXbufferSize = 0;
      _RXbufferpos = 0;
      xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
    } else {
      _RXbufferSize = 0;
      _RXbufferpos = 0;
    }
  } else {
    _lastflush = millis();
  }
}

bool Serial_2_Socket::isPaused() { return _paused; }

void Serial_2_Socket::end() {
  // Protect TX buffer access with mutex
  if (_txBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_txBufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    _TXbufferSize = 0;
    xSemaphoreGive((SemaphoreHandle_t)_txBufferMutex);
  } else {
    _TXbufferSize = 0;
  }
  
  // Protect RX buffer access with mutex
  if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    _RXbufferSize = 0;
    _RXbufferpos = 0;
    xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
  } else {
    _RXbufferSize = 0;
    _RXbufferpos = 0;
  }
  
  _started = false;
  _paused = false;
  _lastflush = millis();
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}

long Serial_2_Socket::baudRate() { return 0; }

bool Serial_2_Socket::started() { return _started; }

Serial_2_Socket::operator bool() const { return true; }

int Serial_2_Socket::available() {
  if (_paused || !_started) {
    return 0;
  }
  if (_RXbufferSize == 0) {
    return 0;
  }

  // Protect RX buffer access with mutex
  if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    esp3d_log_e("Serial2Socket: Failed to take mutex for available");
    return 0;
  }

  int size = _RXbufferSize;

  if (_rxBufferMutex != NULL) {
    xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
  }
  return size;
}

size_t Serial_2_Socket::write(uint8_t c) {
  if (!_started || _paused) {
    return 1;
  }
  esp3d_log_d("Serial2Socket: write one char %c", c);
  return write(&c, 1);
}

size_t Serial_2_Socket::write(const uint8_t *buffer, size_t size) {
  if (buffer == NULL || size == 0 || !_started || _paused) {
    esp3d_log("Serial2Socket: no data, not started or paused");
    return size;
  }
  
  // Take TX buffer mutex once for the entire function
  if (_txBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_txBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    esp3d_log_e("Serial2Socket: Failed to take mutex for write");
    return 0;
  }
  
  if (_TXbufferSize == 0) {
    _lastflush = millis();
  }
  
  // Check if buffer is full and needs flushing
  if (_TXbufferSize + size > S2S_TXBUFFERSIZE) {
    esp3d_log_d("Serial2Socket: buffer full, flush it");
    flush(false); // Use flush without mutex since we already have it
  }
  
  // Add data to buffer and flush on newline
  for (int i = 0; i < size; i++) {
    _TXbuffer[_TXbufferSize] = buffer[i];
    _TXbufferSize++;
    if (buffer[i] == (const uint8_t)'\n') {
      esp3d_log_d("S2S: %s TXSize: %d", (const char *)_TXbuffer, _TXbufferSize);
      flush(false); // Use flush without mutex since we already have it
    }
  }
  
  // Check if we need to flush based on time
  if (_TXbufferSize > 0 && ((millis() - _lastflush) > S2S_FLUSHTIMEOUT)) {
    flush(false); // Use flush without mutex since we already have it
  }
  
  // Release the mutex at the end
  if (_txBufferMutex != NULL) {
    xSemaphoreGive((SemaphoreHandle_t)_txBufferMutex);
  }
  
  return size;
}

int Serial_2_Socket::peek(void) {
  esp3d_log_d("Serial2Socket: peek first of %d", _RXbufferSize);
  if (_RXbufferSize <= 0 || !_started) {
    return -1;
  }

  // Protect RX buffer access with mutex
  if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    esp3d_log_e("Serial2Socket: Failed to take mutex for peek");
    return -1;
  }

  uint8_t v = _RXbuffer[_RXbufferpos];

  if (_rxBufferMutex != NULL) {
    xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
  }
  return v;
}

// Send data to socket output buffer
bool Serial_2_Socket::push2RX(const uint8_t *buffer, size_t size) {
  if (buffer == NULL || size == 0 || !_started || _paused) {
    return false;
  }
  
  // Protect RX buffer access with mutex
  if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    esp3d_log_e("Serial2Socket: cannot take mutex for push2RX");
    return false;
  }
  
  int data_size = size;
  bool success = false;
  esp3d_log_d("Serial2Socket: pushing %d chars to buffer", data_size);
  if ((data_size + _RXbufferSize) <= S2S_RXBUFFERSIZE) {
    int current = _RXbufferpos + _RXbufferSize;
    if (current > S2S_RXBUFFERSIZE) {
      current = current - S2S_RXBUFFERSIZE;
    }
    for (int i = 0; i < data_size; i++) {
      if (current > (S2S_RXBUFFERSIZE - 1)) {
        current = 0;
      }
      _RXbuffer[current] = buffer[i];
      current++;
    }
    _RXbufferSize += size;
    if (current < S2S_RXBUFFERSIZE) {
      _RXbuffer[current] = 0;
    }
    success = true;
  }
  
  if (_rxBufferMutex != NULL) {
    xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
  }
  return success;
}

int Serial_2_Socket::read(void) {
  if (_RXbufferSize <= 0 || !_started || _paused) {
    return -1;
  }

  // Protect RX buffer access with mutex
  if (_rxBufferMutex != NULL && xSemaphoreTake((SemaphoreHandle_t)_rxBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    esp3d_log_e("Serial2Socket: Failed to take mutex for read");
    return -1;
  }

  uint8_t v = _RXbuffer[_RXbufferpos];
  _RXbufferpos++;
  if (_RXbufferpos > (S2S_RXBUFFERSIZE - 1)) {
    _RXbufferpos = 0;
  }
  _RXbufferSize--;
  esp3d_log_d("Serial2Socket: read one char %c", v);
  
  if (_rxBufferMutex != NULL) {
    xSemaphoreGive((SemaphoreHandle_t)_rxBufferMutex);
  }
  return v;
}

void Serial_2_Socket::handle() { handle_flush(); }

void Serial_2_Socket::handle_flush() {
  if (_TXbufferSize > 0 && _started && !_paused) {
    if ((_TXbufferSize >= S2S_TXBUFFERSIZE) ||
        ((millis() - _lastflush) > S2S_FLUSHTIMEOUT)) {
      esp3d_log("force socket flush");
      flush(true);
    }
  }
}

void Serial_2_Socket::flush(bool useMutex) {

  // Process buffer if there's data and we're not paused
  if (_TXbufferSize > 0 && _started && !_paused) {
     // Protect TX buffer access with mutex if requested
    if (useMutex && _txBufferMutex != NULL) {
    if (xSemaphoreTake((SemaphoreHandle_t)_txBufferMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
      esp3d_log_e("Serial2Socket: Failed to take mutex for flush");
      return;
    }
  }
    ESP3DMessage *msg = esp3d_message_manager.newMsg(
        ESP3DClientType::socket_serial, ESP3DClientType::all_clients, _TXbuffer,
        _TXbufferSize, _auth);
    
    // Reset buffer before processing
    _lastflush = millis();
    _TXbufferSize = 0;
    
    // Release mutex if we took it
    if (useMutex && _txBufferMutex != NULL) {
      xSemaphoreGive((SemaphoreHandle_t)_txBufferMutex);
    }
    
    if (msg) {
      // process command
      msg->type = ESP3DMessageType::unique;
      esp3d_commands.process(msg);
    } else {
      esp3d_log_e("Cannot create message");
    }
  }
}

bool Serial_2_Socket::dispatch(ESP3DMessage *message) {
  if (!message || !_started) {
    esp3d_log_e("Serial2Socket: no message or not started");
    return false;
  }
  if (message->size > 0 && message->data) {
    esp3d_log_d("Serial2Socket: dispatch message %d", message->size);
    if (!push2RX(message->data, message->size)) {
      esp3d_log_e("Serial2Socket: cannot push all data");
      return false;
    }
    esp3d_message_manager.deleteMsg(message);
    return true;
  }
  esp3d_log_e("Serial2Socket: no data in message");
  return false;
}

#endif  // ESP3DLIB_ENV
