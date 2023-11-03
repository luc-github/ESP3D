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
#include <Arduino.h>

#include <list>
#include <utility>

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
  void parseRequest();
  bool selectHandler(const char* method, const char* url);
  size_t clearPayload();
  bool hasHeader(const char* name);
  const char* getHeader(const char* name);
  void handler_options();
  void handler_get();
  void handler_put();
  void handler_head();
  void handler_copy();
  void handler_move();
  void handler_mkcol();
  void handler_delete();
  void handler_lock();
  void handler_unlock();
  void handler_propfind();
  void handler_proppatch();
  bool send_response_code(int code);
  bool send_header(const char* name, const char* value);
  bool send_webdav_headers();
  bool send_response(const char* response);
  bool send_content(const char* content);

 private:
  bool _started;
  bool _headers_sent;
  bool _is_chunked;
  bool _response_code_sent;
  std::list<std::pair<String, String>> _headers;
  WiFiServer* _tcpServer;
  WiFiClient _client;
  uint16_t _port;
};

extern WebdavServer webdav_server;

#endif
