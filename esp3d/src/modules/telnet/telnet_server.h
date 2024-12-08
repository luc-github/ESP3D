/*
  telnet_server.h -  telnet service functions class

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

#ifndef _TELNET_SERVER_H
#define _TELNET_SERVER_H

#include <WiFiClient.h>
#include <WiFiServer.h>

#include "../../core/esp3d_message.h"

#define ESP3D_TELNET_BUFFER_SIZE 1200

class Telnet_Server {
 public:
  Telnet_Server();
  ~Telnet_Server();
  bool begin(uint16_t port = 0, bool debug = false);
  void end();
  void handle();
  bool reset();
  bool started();
  bool isConnected();
  const char* clientIPAddress();
  size_t writeBytes(const uint8_t* buffer, size_t size);
  bool dispatch(ESP3DMessage* message);
  int available();
  int availableForWrite();
  void flush();
  size_t readBytes(uint8_t* sbuf, size_t len);
  uint16_t port() { return _port; }
  void closeClient();
  void initAuthentication();
  void setAuthentication(ESP3DAuthenticationLevel auth) { _auth = auth; }
  ESP3DAuthenticationLevel getAuthentication();

 private:
  bool _started;
  WiFiServer* _telnetserver;
  WiFiClient _telnetClients;
  ESP3DAuthenticationLevel _auth;
  uint16_t _port;
  bool _isdebug;
  uint32_t _lastflush;
  uint8_t* _buffer;
  size_t _buffer_size;
  void push2buffer(uint8_t* sbuf, size_t len);
  void flushBuffer();
  void flushChar(char c);
  void flushData(const uint8_t* data, size_t size, ESP3DMessageType type);
};

extern Telnet_Server telnet_server;

#endif
