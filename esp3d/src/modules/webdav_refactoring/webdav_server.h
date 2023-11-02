/*
 webdav_server.h - webdav service functions class

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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

#ifndef _WEBDAV_SERVER_H
#define _WEBDAV_SERVER_H
#include "stdint.h"
class WiFiServer;
class WiFiClient;

class WebdavServer {
 public:
  WebdavServer();
  ~WebdavServer();
  bool begin();
  void end();
  void handle();
  bool started();
  bool isConnected();
  const char* clientIPAddress();
  uint16_t port() { return _port; }
  void closeClient();

 private:
  bool _started;
  WiFiServer* _tcpServer;
  WiFiClient _client;
  uint16_t _port;
};

extern WebdavServer esp3d_webdav_server;

#endif
