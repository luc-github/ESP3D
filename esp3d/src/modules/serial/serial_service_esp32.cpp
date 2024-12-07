/*
  esp3d_serial_service.cpp -  serial services functions class

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
#if defined(ARDUINO_ARCH_ESP32)
#include "../../include/esp3d_config.h"
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || \
    defined(ESP_SERIAL_BRIDGE_OUTPUT) || COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../authentication/authentication_service.h"
#include "serial_service.h"

#define SERIAL_COMMUNICATION_TIMEOUT 500

#if defined(CONFIG_IDF_TARGET_ESP32C3) || \
    defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32S2)
#define MAX_SERIAL 2
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1};
#else
#define MAX_SERIAL 3
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1, &Serial2};
#endif

// Serial Parameters

// Constructor
ESP3DSerialService::ESP3DSerialService(uint8_t id) {
  _buffer_size = 0;
  _mutex = NULL;
  _started = false;
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication = true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
  _id = id;
  switch (_id) {
    case MAIN_SERIAL:
      _rxPin = ESP_RX_PIN;
      _txPin = ESP_TX_PIN;
      _origin = ESP3DClientType::serial;
      break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      _rxPin = ESP_BRIDGE_RX_PIN;
      _txPin = ESP_BRIDGE_TX_PIN;
      _origin = ESP3DClientType::serial_bridge;
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      _rxPin = ESP_RX_PIN;
      _txPin = ESP_TX_PIN;
      _origin = ESP3DClientType::serial;
      break;
  }
  _messagesInFIFO.setId("in");
  _messagesInFIFO.setMaxSize(0);  // no limit
  _baudRate = 0;
}
void ESP3DSerialService::receiveSerialCb() { esp3d_serial_service.receiveCb(); }

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
void ESP3DSerialService::receiveBridgeSerialCb() {
  serial_bridge_service.receiveCb();
}
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

void ESP3DSerialService::receiveCb() {
  if (!started()) {
    return;
  }
  if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
    uint32_t now = millis();
    while ((millis() - now) < SERIAL_COMMUNICATION_TIMEOUT) {
      if (Serials[_serialIndex]->available()) {
        _buffer[_buffer_size] = Serials[_serialIndex]->read();
        now = millis();
        if (esp3d_string::isRealTimeCommand(_buffer[_buffer_size])) {
          flushChar(_buffer[_buffer_size]);
          _buffer[_buffer_size] = '\0'; //remove realtime command from buffer
        } else {
          _buffer_size++;
          if (_buffer_size > ESP3D_SERIAL_BUFFER_SIZE ||
              _buffer[_buffer_size - 1] == '\n') {
            flushBuffer();
          }
        }
      }
    }

    xSemaphoreGive(_mutex);
  } else {
    esp3d_log_e("Mutex not taken");
  }
}

// Setup Serial
bool ESP3DSerialService::begin(uint8_t serialIndex) {
  _mutex = xSemaphoreCreateMutex();
  if (_mutex == NULL) {
    esp3d_log_e("Mutex creation failed");
    return false;
  }
  _serialIndex = serialIndex - 1;
  esp3d_log("Serial %d begin for %d", _serialIndex, _id);
  if (_id == BRIDGE_SERIAL &&
      ESP3DSettings::readByte(ESP_SERIAL_BRIDGE_ON) == 0) {
    esp3d_log("Serial %d for %d is disabled", _serialIndex, _id);
    return true;
  }
  if (_serialIndex >= MAX_SERIAL) {
    esp3d_log_e("Serial %d begin for %d failed, index out of range",
                _serialIndex, _id);
    return false;
  }
  _lastflush = millis();
  // read from settings
  uint32_t br = 0;
  uint32_t defaultBr = 0;
  switch (_id) {
    case MAIN_SERIAL:
      br = ESP3DSettings::readUint32(ESP_BAUD_RATE);
      defaultBr = ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE);
      break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      br = ESP3DSettings::readUint32(ESP_SERIAL_BRIDGE_BAUD);
      defaultBr =
          ESP3DSettings::getDefaultIntegerSetting(ESP_SERIAL_BRIDGE_BAUD);
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      esp3d_log_e("Serial %d begin for %d failed, unknown id", _serialIndex,
                  _id);
      return false;
  }
  setParameters();
  esp3d_log("Baud rate is %d , default is %d", br, defaultBr);
  _buffer_size = 0;
  // change only if different from current
  if (br != baudRate() || (_rxPin != -1) || (_txPin != -1)) {
    if (!ESP3DSettings::isValidIntegerSetting(br, ESP_BAUD_RATE)) {
      br = defaultBr;
    }
    Serials[_serialIndex]->setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
    Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, _rxPin, _txPin);
    _baudRate = br;
    if (_id == MAIN_SERIAL) {
      Serials[_serialIndex]->onReceive(receiveSerialCb);
    }
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    if (_id == BRIDGE_SERIAL) {
      Serials[_serialIndex]->onReceive(receiveBridgeSerialCb);
    }
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
  }
  _started = true;
  esp3d_log("Serial %d for %d is started", _serialIndex, _id);
  return true;
}

// End serial
bool ESP3DSerialService::end() {
  flush();
  delay(100);
  if (_mutex != NULL) {
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
  }
  Serials[_serialIndex]->end();
  _buffer_size = 0;
  _started = false;
  initAuthentication();
  return true;
}

void ESP3DSerialService::flushData(const uint8_t *data, size_t size, ESP3DMessageType type) {
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      _origin,
      _id == MAIN_SERIAL ? ESP3DClientType::all_clients
                         : esp3d_commands.getOutputClient(),
      data, size, getAuthentication());

  if (message) {
    message->type = type;
    esp3d_log("Message sent to fifo list");
    _messagesInFIFO.push(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastflush = millis();
}

// Function which could be called in other loop
void ESP3DSerialService::handle() {
  // Do we have some data waiting
  size_t len = _messagesInFIFO.size();
  if (len > 0) {
    if (len > 10) {
      len = 10;
    }
    while (len > 0) {
      esp3d_log("Serial in fifo size %d", _messagesInFIFO.size());
      ESP3DMessage *message = _messagesInFIFO.pop();
      if (message) {
        esp3d_commands.process(message);
      } else {
        esp3d_log_e("Cannot create message");
      }
      len--;
    }
  }
}

// Reset Serial Setting (baud rate)
bool ESP3DSerialService::reset() {
  esp3d_log("Reset serial");
  bool res = false;
  switch (_id) {
    case MAIN_SERIAL:
      return ESP3DSettings::writeUint32(
          ESP_BAUD_RATE,
          ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE));
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      res = ESP3DSettings::writeByte(
          ESP_SERIAL_BRIDGE_ON,
          ESP3DSettings::getDefaultByteSetting(ESP_SERIAL_BRIDGE_ON));
      return res &&
             ESP3DSettings::writeUint32(ESP_SERIAL_BRIDGE_BAUD,
                                        ESP3DSettings::getDefaultIntegerSetting(
                                            ESP_SERIAL_BRIDGE_BAUD));
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      return res;
  }
}


#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL ||
        // defined(ESP_SERIAL_BRIDGE_OUTPUT)
#endif  // ARDUINO_ARCH_ESP32