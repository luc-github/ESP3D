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
#include "../../include/version.h"
#include "../network/netconfig.h"
#include "webdav_server.h"

WebdavServer webdav_server;

#define TIMEOUT_WEBDAV_FLUSH 1500
#define TIMEOUT_WEBDAV_REQUEST 5000

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

const char* WebdavServer::clientIPAddress() {
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
    parseRequest();
    closeClient();
  }
}

void WebdavServer::parseRequest() {
  String line = "";
  _headers_sent = false;
  _is_chunked = false;
  _response_code_sent = false;
  _headers.clear();
  // read the first line of the request to get the request method, URL and
  // HTTP
  log_esp3d_d("Parsing request\n\n");
  if (_client.connected() && _client.available()) {
    bool hasError = false;
    bool lineIsRead = false;
    size_t has_r = 0;
    Esp3dTimout timeout(TIMEOUT_WEBDAV_REQUEST);
    while (!lineIsRead) {
      int c = _client.read();
      if (c == -1) {
        if (timeout.isTimeout()) {
          hasError = true;
          log_esp3d_e("Timeout");
          lineIsRead = true;
        }
      } else {
        if (c == '\n') {
          lineIsRead = true;
          if (has_r != 1) {
            hasError = true;
            log_esp3d_e("Bad line ending");
          }
        } else if (c != '\r') {
          line += (char)c;
        } else {
          has_r++;
        }
      }
    }
    if (hasError) {
      send_response_code(400);
      send_webdav_headers();
      _client.write("\r\n");  // empty line
      log_esp3d_e("Bad request line: %s", line.c_str());
      return;
    }
    line.toUpperCase();
    log_esp3d_d("Request: %s", line.c_str());
    size_t pos1 = line.indexOf(' ');
    size_t pos2 = line.indexOf(' ', pos1 + 1);
    if (pos1 == -1 || pos2 == -1) {
      send_response_code(400);
      log_esp3d_e("Bad request line: %s", line.c_str());
      send_webdav_headers();
      _client.write("\r\n");  // empty line
      return;
    }
    static String method = line.substring(0, pos1);
    static String url = line.substring(pos1 + 1, pos2);
    // Now list all headers
    bool headers_read = false;
    while (!headers_read) {
      lineIsRead = false;
      has_r = 0;
      line = "";
      timeout.reset();
      while (!lineIsRead && !hasError) {
        int c = _client.read();
        if (c == -1) {
          if (timeout.isTimeout()) {
            hasError = true;
            log_esp3d_e("Timeout");
            lineIsRead = true;
          }
        } else {
          if (c == '\n') {
            lineIsRead = true;
            if (has_r != 1) {
              hasError = true;
              log_esp3d_e("Bad line ending %d r, %s", has_r, line.c_str());
            }
          } else if (c != '\r') {
            line += (char)c;
          } else {
            has_r++;
          }
        }
      }
      log_esp3d_d("Line: %s", line.c_str());
      if (hasError) {
        send_response_code(400);
        send_webdav_headers();
        _client.write("\r\n");  // empty line
        log_esp3d_e("Bad request line: %s", line.c_str());
        return;
      }
      if (line.length() == 0) {
        headers_read = true;
      } else {
        pos1 = line.indexOf(':');
        static String header_name = line.substring(0, pos1);
        static String header_value = line.substring(pos1 + 1);
        header_name.trim();
        header_value.trim();
        log_esp3d_d("Header: %s = %s", header_name.c_str(),
                    header_value.c_str());
        _headers.push_back(std::make_pair(header_name, header_value));
      }
    }

    selectHandler(method.c_str(), url.c_str());
  }
}

bool WebdavServer::send_response_code(int code) {
  if (_headers_sent) {
    log_esp3d_e("Headers already sent");
    return false;
  }
  if (_response_code_sent) {
    log_esp3d_e("Response code already sent");
    return false;
  }

  _client.write("HTTP/1.1 ");
  _client.write(code);
  _client.write(" ");
  _response_code_sent = true;
  switch (code) {
    case 200:
      _client.write("OK");
      break;
    case 201:
      _client.write("Created");
      break;
    case 204:
      _client.write("No Content");
      break;
    case 207:
      _client.write("Multi-Status");
      break;
    case 400:
      _client.write("Bad Request");
      break;
    case 401:
      _client.write("Unauthorized");
      break;
    case 403:
      _client.write("Forbidden");
      break;
    case 404:
      _client.write("Not Found");
      break;
    case 405:
      _client.write("Method Not Allowed");
      break;
    case 409:
      _client.write("Conflict");
      break;
    case 412:
      _client.write("Precondition Failed");
      break;
    case 423:
      _client.write("Locked");
      break;
    case 424:
      _client.write("Failed Dependency");
      break;
    case 500:
      _client.write("Internal Server Error");
      break;
    case 501:
      _client.write("Not Implemented");
      break;
    case 507:
      _client.write("Insufficient Storage");
      break;
    default:
      _client.write("Unknown");
      _client.write("\r\n\r\n");
      log_esp3d_e("Unknown code %d", code);
      return false;
      break;
  }
  _client.write("\r\n");
  return true;
}

bool WebdavServer::send_header(const char* name, const char* value) {
  if (_headers_sent) {
    log_esp3d_e("Headers already sent");
    return false;
  }
  if (!_response_code_sent) {
    send_response_code(200);
  }
  _client.write(name);
  _client.write(": ");
  _client.write(value);
  _client.write("\r\n");
  return true;
}

bool WebdavServer::send_webdav_headers() {
  if (_headers_sent) {
    log_esp3d_e("Headers already sent");
    return false;
  }
  if (!_response_code_sent) {
    send_response_code(200);
  }
  send_header("DAV", "1");
  send_header("Allow",
              "OPTIONS, GET, HEAD, PUT, DELETE, COPY, MOVE, MKCOL, PROPFIND");
  send_header("Connection", "close");
  send_header("Cache-Control", "no-cache");

  static String ua = "";
  static String host = "";
  if (ua.length() == 0) {
    ua = "ESP3D-WebdavServer/1.0 (";
    ua += Settings_ESP3D::TargetBoard();
    ua += "; Firmware/";
    ua += FW_VERSION;
    ua += "; Platform/arduino; Embedded; http://www.esp3d.io)";
  }
  if (host.length() == 0) {
    host = "Host: http://";
    host += NetConfig::localIP();
    if (_port != 80) {
      host += ":";
      host += String(_port);
    }
  }

  send_header("User-Agent", ua.c_str());
  send_header("Host", host.c_str());

  return true;
}

bool WebdavServer::send_content(const char* response) {
  if (send_header("Content-Length", String(strlen(response)).c_str())) {
    send_header("Content-Type", "text/html; charset=utf-8");
    _client.write("\r\n");
    _headers_sent = true;
    _client.write(response);
    _client.write("\r\n");
    return true;
  }
  return false;
}

bool WebdavServer::send_response(const char* response) {
  if (send_header("Content-Length", String(strlen(response)).c_str())) {
    send_header("Content-Type", "text/html; charset=utf-8");
    _client.write("\r\n");
    _headers_sent = true;
    _client.write(response);
    _client.write("\r\n");
    return true;
  }
  return false;
}

bool WebdavServer::selectHandler(const char* method, const char* url) {
  log_esp3d_d("Method: %s", method);
  log_esp3d_d("URL: %s", url);
  if (strcmp(method, "OPTIONS") == 0) {
    handler_options();
    return true;
  }
  if (strcmp(method, "GET") == 0) {
    handler_get();
    return true;
  }
  if (strcmp(method, "PUT") == 0) {
    handler_put();
    return true;
  }
  if (strcmp(method, "HEAD") == 0) {
    handler_head();
    return true;
  }
  if (strcmp(method, "COPY") == 0) {
    handler_copy();
    return true;
  }
  if (strcmp(method, "MOVE") == 0) {
    handler_move();
    return true;
  }
  if (strcmp(method, "MKCOL") == 0) {
    handler_mkcol();
    return true;
  }
  if (strcmp(method, "DELETE") == 0) {
    handler_delete();
    return true;
  }
  if (strcmp(method, "LOCK") == 0) {
    handler_lock();
    return true;
  }
  if (strcmp(method, "UNLOCK") == 0) {
    handler_unlock();
    return true;
  }
  if (strcmp(method, "PROPFIND") == 0) {
    handler_propfind();
    return true;
  }
  if (strcmp(method, "PROPPATCH") == 0) {
    handler_proppatch();
    return true;
  }
  send_response_code(405);
  send_webdav_headers();
  _client.write("\r\n");  // empty line
  return false;
}
size_t WebdavServer::clearPayload() {
  size_t res = 0;
  uint8_t chunk[50];
  if (_client) {
    while (_client.available()) {
      res += _client.read(chunk, sizeof(chunk));
    }
  }
  return res;
}
bool WebdavServer::hasHeader(const char* name) {
  for (auto it = _headers.begin(); it != _headers.end(); ++it) {
    // look for header with name
    std::pair<String, String> header = *it;
    // if present return true
    if (header.first == name) {
      return true;
    }
  }
  return false;
}
const char* WebdavServer::getHeader(const char* name) {
  for (auto it = _headers.begin(); it != _headers.end(); ++it) {
    // look for header with name
    std::pair<String, String> header = *it;
    // if present return true
    if (header.first == name) {
      return header.second.c_str();
    }
  }

  return "";
}

#endif  // WEBDAV_FEATURE
