/*
  BT_service.h -  Bluetooth service functions class

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
#ifndef _BT_SERVICE_H
#define _BT_SERVICE_H

#include "../../core/esp3d_message.h"

#define ESP3D_BT_BUFFER_SIZE 512

class BTService {
 public:
  BTService();
  ~BTService();
  static void BTEvent(uint8_t event);
  const char* hostname();
  static const char* macAddress();
  static const char* clientmacAddress();
  bool begin();
  void end();
  void handle();
  bool reset();
  bool started();
  static void setClientAddress(const char* saddress);
  bool isConnected();
  void flush();
  int availableForWrite();
  int available();
  size_t writeBytes(const uint8_t* buffer, size_t size);
  size_t readBytes(uint8_t* sbuf, size_t len);
  bool dispatch(ESP3DMessage* message);
  void initAuthentication();
  void setAuthentication(ESP3DAuthenticationLevel auth) { _auth = auth; }
  ESP3DAuthenticationLevel getAuthentication();

 private:
  ESP3DAuthenticationLevel _auth;
  static String _btname;
  static String _btclient;
  uint32_t _lastflush;
  uint8_t _buffer[ESP3D_BT_BUFFER_SIZE + 1];  // keep space of 0x0 terminal
  size_t _buffer_size;
  void push2buffer(uint8_t* sbuf, size_t len);
  void flushBuffer();
  void flushChar(char c);
  void flushData(const uint8_t* data, size_t size, ESP3DMessageType type);
  bool _started;
};

extern BTService bt_service;

#endif  //_BT_SERVICE_H
