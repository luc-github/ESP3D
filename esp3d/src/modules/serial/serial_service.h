/*
  serial_service.h -  serial services functions class

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

#ifndef _SERIAL_SERVICES_H
#define _SERIAL_SERVICES_H

#include "../../core/esp3d_client_types.h"
#include "../../core/esp3d_message.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "../../core/esp3d_messageFifo.h"
#endif  // ARDUINO_ARCH_ESP32

#define ESP3D_SERIAL_BUFFER_SIZE 1024
#define ESP_SERIAL_PARAM SERIAL_8N1

extern const uint32_t SupportedBaudList[];
extern const size_t SupportedBaudListSize;

class ESP3DSerialService final {
 public:
  ESP3DSerialService(uint8_t id);
  ~ESP3DSerialService();
  void setParameters();
  bool begin(uint8_t serialIndex);
  bool end();
  void updateBaudRate(uint32_t br);
  void handle();
  bool reset();
  uint32_t baudRate();
  uint8_t serialIndex() { return _serialIndex; }
  const uint32_t *get_baudratelist(uint8_t *count);
  void flush();
  #if defined(ARDUINO_ARCH_ESP8266)
  void swap();
  #endif // ARDUINO_ARCH_ESP8266
  size_t writeBytes(const uint8_t *buffer, size_t size);
  size_t readBytes(uint8_t *sbuf, size_t len);
  inline bool started() { return _started; }
  bool dispatch(ESP3DMessage *message);
  void initAuthentication();
  void setAuthentication(ESP3DAuthenticationLevel auth) { _auth = auth; }
  ESP3DAuthenticationLevel getAuthentication();
#if defined(ARDUINO_ARCH_ESP32)
  void receiveCb();
  static void receiveSerialCb();
  static void receiveBridgeSerialCb();
#endif  // ARDUINO_ARCH_ESP32
 private:
  uint32_t _baudRate;
  ESP3DAuthenticationLevel _auth;
  uint8_t _serialIndex;
  ESP3DClientType _origin;
  uint8_t _id;
  int8_t _rxPin;
  int8_t _txPin;
  bool _started;
  bool _needauthentication;
  uint32_t _lastflush;
  uint8_t _buffer[ESP3D_SERIAL_BUFFER_SIZE + 1];  // keep space of 0x0 terminal
  size_t _buffer_size;
#if defined(ARDUINO_ARCH_ESP32)
  SemaphoreHandle_t _mutex;
  ESP3DMessageFIFO _messagesInFIFO;
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  void push2buffer(uint8_t *sbuf, size_t len);
#endif // ARDUINO_ARCH_ESP8266
  void flushBuffer();
  void flushChar(char c);
  void flushData(const uint8_t* data, size_t size, ESP3DMessageType type);
};

extern ESP3DSerialService esp3d_serial_service;

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
extern ESP3DSerialService serial_bridge_service;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#endif  //_SERIAL_SERVICES_H
