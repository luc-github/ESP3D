/*
  webdav_server.cpp -  webdav server functions class

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

#include "../../include/esp3d_config.h"

#if defined(WEBDAV_FEATURE)
#include <WiFiClient.h>
#include <WiFiServer.h>

#include "../../core/hal.h"
#include "../../core/settings_esp3d.h"
#include "webdav_server.h"

WebdavServer esp3d_webdav_server;

#define TIMEOUT_WEBDAV_FLUSH 1500

void WebdavServer::closeClient() {
  if (_client) {
    _client.stop();
  }
}

bool WebdavServer::isConnected() {
  if (!_started || _tcpServer == NULL) {
    return false;
  }
  // check if there are any new clients
  if (_tcpServer->hasClient()) {
    // find free/disconnected spot
    if (!_client || !_client.connected()) {
      if (_client) {
        _client.stop();
      }
      _client = _tcpServer->accept();
      // new client
    }
  }
  if (_tcpServer->hasClient()) {
    // no free/disconnected spot so reject
    _tcpServer->accept().stop();
  }
  return _client.connected();
}

const char *WebdavServer::clientIPAddress() {
  static String res;
  res = "0.0.0.0";
  if (_client && _client.connected()) {
    res = _client.remoteIP().toString();
  }
  return res.c_str();
}

WebdavServer::WebdavServer() {
  _started = false;
  _port = 0;
  _tcpServer = nullptr;
}
WebdavServer::~WebdavServer() { end(); }

/**
 * begin Telnet setup
 */
bool WebdavServer::begin() {
  end();
  if (Settings_ESP3D::read_byte(ESP_WEBDAV_ON) != 1) {
    return true;
  }
  // Get webdav port
  _port = Settings_ESP3D::read_uint32(ESP_WEBDAV_PORT);

  // create instance
  _tcpServer = new WiFiServer(_port);
  if (!_tcpServer) {
    return false;
  }
  _tcpServer->setNoDelay(true);
  // start webdav server
  _tcpServer->begin();
  _started = true;
  return _started;
}
/**
 * End Telnet
 */
void WebdavServer::end() {
  _started = false;
  _port = 0;
  closeClient();
  if (_tcpServer) {
    delete _tcpServer;
    _tcpServer = nullptr;
  }
}

bool WebdavServer::started() { return _started; }

void WebdavServer::handle() {
  Hal::wait(0);
  if (isConnected()) {
    // TODO:Read data from client
    // TODO:Process data
    closeClient();
  }
}

#endif  // WEBDAV_FEATURE
