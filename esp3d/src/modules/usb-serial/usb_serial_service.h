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

#pragma once

#include <esp32_usb_serial.h>

#include "../../core/esp3d_client_types.h"
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_messageFifo.h"


#define ESP3D_USB_SERIAL_BUFFER_SIZE 1024

extern const uint32_t SupportedUsbSerialBaudList[];
extern const size_t SupportedUsbSerialBaudListSize;

class ESP3DUsbSerialService final {
 public:
  ESP3DUsbSerialService();
  ~ESP3DUsbSerialService();
  void setParameters();
  bool begin();
  bool end();
  void updateBaudRate(uint32_t br);
  void handle();
  bool reset();
  uint32_t baudRate();
  const uint32_t *get_baudratelist(uint8_t *count);
  void flush();
  void swap();
  size_t writeBytes(const uint8_t *buffer, size_t size);
  size_t readBytes(uint8_t *sbuf, size_t len);
  inline bool started() { return _started; }
  bool dispatch(ESP3DMessage *message);
  void initAuthentication();
  void setAuthentication(ESP3DAuthenticationLevel auth) { _auth = auth; }
  ESP3DAuthenticationLevel getAuthentication();
  void connectDevice();
  void setConnected(bool connected);
  void receiveCb(const uint8_t *data, size_t data_len, void *arg = nullptr);
  bool isConnected() { return _is_connected; }
  const char * getVIDString();
  const char * getPIDString();
  uint16_t getVID();
  uint16_t getPID();

 private:
  uint32_t _baudRate;
  ESP3DAuthenticationLevel _auth;
  ESP3DClientType _origin;
  bool _started;
  bool _needauthentication;
  uint32_t _lastflush;
  uint8_t
      _buffer[ESP3D_USB_SERIAL_BUFFER_SIZE + 1];  // keep space of 0x0 terminal
  size_t _buffer_size;
  SemaphoreHandle_t _buffer_mutex;
  SemaphoreHandle_t _device_disconnected_mutex;
  bool _is_connected;
  std::unique_ptr<CdcAcmDevice> _vcp_ptr;

  TaskHandle_t _xHandle;
  ESP3DMessageFIFO _messagesInFIFO;
  void flushBuffer();
  void flushChar(char c);
  void flushData(const uint8_t* data, size_t size, ESP3DMessageType type);
};

extern ESP3DUsbSerialService esp3d_usb_serial_service;
