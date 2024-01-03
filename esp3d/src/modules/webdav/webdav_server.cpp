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

#include "../../core/esp3d_hal.h"
#include "../../core/esp3d_settings.h"
#include "../../include/esp3d_version.h"
#include "../network/netconfig.h"
#include "webdav_server.h"

WebdavServer webdav_server;

#define TIMEOUT_WEBDAV_FLUSH 1500
#define TIMEOUT_WEBDAV_REQUEST 5000

void WebdavServer::closeClient() {
  if (_client) {
    if (!_headers_sent) {
      _client.print("\r\n");
      _headers_sent = true;
    }
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
  if (ESP3DSettings::readByte(ESP_WEBDAV_ON) != 1) {
    return true;
  }
  // Get webdav port
  _port = ESP3DSettings::readUint32(ESP_WEBDAV_PORT);

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
  ESP3DHal::wait(0);
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
  esp3d_log("Parsing new request:\n");
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
          esp3d_log_e("Timeout");
          lineIsRead = true;
        }
      } else {
        if (c == '\n') {
          lineIsRead = true;
          if (has_r != 1) {
            hasError = true;
            esp3d_log_e("Bad line ending");
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
      _client.print("\r\n");  // empty line
      esp3d_log_e("Bad request line: %s", line.c_str());
      return;
    }

    esp3d_log("Request: %s", line.c_str());
    int pos1 = line.indexOf(' ');
    int pos2 = line.indexOf(' ', pos1 + 1);
    if (pos1 == -1 || pos2 == -1) {
      send_response_code(400);
      esp3d_log_e("Bad request line: %s", line.c_str());
      send_webdav_headers();
      _client.print("\r\n");  // empty line
      return;
    }
    String method = line.substring(0, pos1);
    method.toUpperCase();
    String url = line.substring(pos1 + 1, pos2);
    // Do some sanity check
    url.trim();
    method.trim();
    // if empty
    if (url.length() == 0) {
      url = "/";
    }
    // if encoded
    url = urlDecode(url.c_str());
    esp3d_log("url ini: %s", url.c_str());
    if (url != "/" && url[url.length() - 1] == '/') {
      url = url.substring(0, url.length() - 1);
    }
    // if not starting with /
    if (url[0] != '/') {
      url = "/" + url;
    }
    esp3d_log("url clean: %s", url.c_str());

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
            esp3d_log_e("Timeout");
            lineIsRead = true;
          }
        } else {
          if (c == '\n') {
            lineIsRead = true;
            if (has_r != 1) {
              hasError = true;
              esp3d_log_e("Bad line ending %d r, %s", has_r, line.c_str());
            }
          } else if (c != '\r') {
            line += (char)c;
          } else {
            has_r++;
          }
        }
      }
      esp3d_log("Line: %s", line.c_str());
      if (hasError) {
        send_response_code(400);
        send_webdav_headers();
        _client.print("\r\n");  // empty line
        esp3d_log_e("Bad request line: %s", line.c_str());
        return;
      }
      if (line.length() == 0) {
        headers_read = true;
      } else {
        pos1 = line.indexOf(':');
        String header_name = line.substring(0, pos1);
        String header_value = line.substring(pos1 + 1);
        header_name.trim();
        header_value.trim();
        esp3d_log("Header: %s = %s", header_name.c_str(), header_value.c_str());
        _headers.push_back(std::make_pair(header_name, header_value));
      }
    }

    selectHandler(method.c_str(), url.c_str());
  }
}

bool WebdavServer::send_response_code(int code) {
  if (_headers_sent) {
    esp3d_log_e("Headers already sent");
    return false;
  }
  if (_response_code_sent) {
    esp3d_log_e("Response code already sent");
    return false;
  }

  _client.print("HTTP/1.1 ");
  _client.print(String(code).c_str());
  _client.print(" ");
  _response_code_sent = true;
  switch (code) {
    case 200:
      _client.print("OK");
      break;
    case 201:
      _client.print("Created");
      break;
    case 204:
      _client.print("No Content");
      break;
    case 207:
      _client.print("Multi-Status");
      break;
    case 400:
      _client.print("Bad Request");
      break;
    case 401:
      _client.print("Unauthorized");
      break;
    case 403:
      _client.print("Forbidden");
      break;
    case 404:
      _client.print("Not Found");
      break;
    case 405:
      _client.print("Method Not Allowed");
      break;
    case 409:
      _client.print("Conflict");
      break;
    case 412:
      _client.print("Precondition Failed");
      break;
    case 423:
      _client.print("Locked");
      break;
    case 424:
      _client.print("Failed Dependency");
      break;
    case 500:
      _client.print("Internal Server Error");
      break;
    case 501:
      _client.print("Not Implemented");
      break;
    case 507:
      _client.print("Insufficient Storage");
      break;
    default:
      _client.print("Unknown");
      _client.print("\r\n\r\n");
      esp3d_log_e("Unknown code %d", code);
      return false;
      break;
  }
  _client.print("\r\n");
  return true;
}

const char* WebdavServer::urlDecode(const char* url) {
  static char* decoded = nullptr;
  if (decoded) {
    free(decoded);
  }
  char temp[] = "0x00";
  unsigned int len = strlen(url);
  unsigned int i = 0;
  unsigned int p = 0;
  decoded = (char*)malloc(len + 1);
  if (decoded) {
    while (i < len) {
      char decodedChar;
      char encodedChar = url[i++];
      if ((encodedChar == '%') && (i + 1 < len)) {
        temp[2] = url[i++];
        temp[3] = url[i++];
        decodedChar = strtol(temp, NULL, 16);
      } else {
        if (encodedChar == '+') {
          decodedChar = ' ';
        } else {
          decodedChar = encodedChar;  // normal ascii char
        }
      }
      decoded[p++] = decodedChar;
    }
    decoded[p] = 0x0;
    return decoded;
  } else {
    esp3d_log_e("Can't allocate memory for decoded url");
    return nullptr;
  }
}

bool WebdavServer::send_header(const char* name, const char* value) {
  if (_headers_sent) {
    esp3d_log_e("Headers already sent");
    return false;
  }
  if (!_response_code_sent) {
    send_response_code(200);
  }
  _client.print(name);
  _client.print(": ");
  _client.print(value);
  _client.print("\r\n");
  return true;
}

bool WebdavServer::send_webdav_headers() {
  if (_headers_sent) {
    esp3d_log_e("Headers already sent");
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
    ua += ESP3DSettings::TargetBoard();
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

bool WebdavServer::send_chunk_content(const char* response) {
  if (!_is_chunked) {
    _is_chunked = true;
    if (!send_header("Transfer-Encoding", "chunked")) {
      esp3d_log_e("Can't send chunked header");
      return false;
    }
    _headers_sent = true;
    _client.print("\r\n");
  }
  _client.printf("%X", strlen(response));
  _client.print("\r\n");
  _client.print(response);
  _client.print("\r\n");
  // end of chunk
  if (strlen(response) == 0) {
    // set it end of chunk
    _client.print("\r\n");
  }
  return true;
}

bool WebdavServer::send_response(const char* response) {
  if (send_header("Content-Length", String(strlen(response)).c_str())) {
    send_header("Content-Type", "text/html; charset=utf-8");
    _client.print("\r\n");
    _headers_sent = true;
    _client.print(response);
    _client.print("\r\n");
    return true;
  }
  return false;
}

bool WebdavServer::selectHandler(const char* method, const char* url) {
  esp3d_log("Method: %s", method);
  esp3d_log("URL: %s", url);
  if (strcasecmp(method, "OPTIONS") == 0) {
    handler_options(url);
    return true;
  }
  if (strcasecmp(method, "GET") == 0) {
    handler_get(url);
    return true;
  }
  if (strcasecmp(method, "PUT") == 0) {
    handler_put(url);
    return true;
  }
  if (strcasecmp(method, "HEAD") == 0) {
    handler_head(url);
    return true;
  }
  if (strcasecmp(method, "COPY") == 0) {
    handler_copy(url);
    return true;
  }
  if (strcasecmp(method, "MOVE") == 0) {
    handler_move(url);
    return true;
  }
  if (strcasecmp(method, "MKCOL") == 0) {
    handler_mkcol(url);
    return true;
  }
  if (strcasecmp(method, "DELETE") == 0) {
    handler_delete(url);
    return true;
  }
  if (strcasecmp(method, "LOCK") == 0) {
    handler_lock(url);
    return true;
  }
  if (strcasecmp(method, "UNLOCK") == 0) {
    handler_unlock(url);
    return true;
  }
  if (strcasecmp(method, "PROPFIND") == 0) {
    handler_propfind(url);
    return true;
  }
  if (strcasecmp(method, "PROPPATCH") == 0) {
    handler_proppatch(url);
    return true;
  }
  esp3d_log_e("Unknown method %s for %s", method, url);
  send_response_code(405);
  send_webdav_headers();
  _client.print("\r\n");  // empty line
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
    esp3d_log("Header: %s = %s", header.first.c_str(), header.second.c_str());
    // if present return true
    if (strcasecmp(header.first.c_str(), name) == 0) {
      return true;
    }
  }
  return false;
}
const char* WebdavServer::getHeader(const char* name) {
  static String res;
  res = "";
  for (auto it = _headers.begin(); it != _headers.end(); ++it) {
    // look for header with name
    std::pair<String, String> header = *it;
    // if present return true
    if (strcasecmp(header.first.c_str(), name) == 0) {
      esp3d_log("Header %s found value %s", name, header.second.c_str());
      res = header.second;
      return res.c_str();
    }
  }

  return res.c_str();
}

bool WebdavServer::isRoot(const char* url) {
  if (strcmp(url, "/") == 0) {
    return true;
  }
#if WEBDAV_FEATURE == FS_ROOT
#ifdef FILESYSTEM_FEATURE
  if (strcmp(url, ESP_FLASH_FS_HEADER) == 0) return true;
#endif  // FILESYSTEM_FEATURE
#if defined(SD_FEATURE)
  if (strcmp(url, ESP_SD_FS_HEADER) == 0) return true;
#endif  // SD_FEATURE

#endif  // WEBDAV_FEATURE == FS_ROOT
  return false;
}
#endif  // WEBDAV_FEATURE
