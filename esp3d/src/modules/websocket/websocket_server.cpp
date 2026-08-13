/*
  websocket_server.cpp -  websocket functions class

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

#include "../../include/esp3d_config.h"

#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE) || \
    (defined(ESP_LOG_FEATURE) && ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET)

#include <WebSocketsServer.h>

#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../authentication/authentication_service.h"
#include "../printer_link/printer_link_service.h"
#include "websocket_server.h"

WebSocket_Server websocket_terminal_server("webui-v3",
                                           ESP3DClientType::webui_websocket);
#if defined(WS_DATA_FEATURE)
WebSocket_Server websocket_data_server("esp3d-v1", ESP3DClientType::websocket);
#endif  // WS_DATA_FEATURE
bool WebSocket_Server::pushMSG(const char *data) {
  if (_websocket_server) {
    esp3d_log_d("[%u]Broadcast %s", _current_id, data);
    return _websocket_server->broadcastTXT(data);
  }
  return false;
}

void WebSocket_Server::enableOnly(uint num) {
  // some sanity check
  if (_websocket_server) {
    for (uint8_t i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++)
      if (i != num && _websocket_server->clientIsConnected(i)) {
        _websocket_server->disconnect(i);
      }
  }
}

bool WebSocket_Server::dispatch(ESP3DMessage *message) {
  if (!message || !_started) {
    return false;
  }
  if (message->size > 0 && message->data) {
    size_t sentcnt = writeBytes(message->data, message->size);
    if (sentcnt != message->size) {
      return false;
    }
    esp3d_message_manager.deleteMsg(message);
    return true;
  }
  return false;
}

bool WebSocket_Server::pushMSG(uint num, const char *data) {
  if (_websocket_server) {
    esp3d_log_d("[%u]Send %s", num, data);
    return _websocket_server->sendTXT(num, data);
  }
  return false;
}

bool WebSocket_Server::isConnected() {
  if (_websocket_server) {
    return _websocket_server->connectedClients() > 0;
  }
  return false;
}

void WebSocket_Server::closeClients() {
  if (_websocket_server) {
    _websocket_server->disconnect();
  }
}
#if defined(WS_DATA_FEATURE)
// Events for Websocket bridge
void handle_Websocket_Server_Event(uint8_t num, uint8_t type, uint8_t *payload,
                                   size_t length) {
  (void)num;
  switch (type) {
    case WStype_DISCONNECTED:
      esp3d_log_d("[%u] Disconnected! port %d", num,
                websocket_data_server.port());
      break;
    case WStype_CONNECTED: {
      websocket_data_server.initAuthentication();
      esp3d_log_d("[%u] Connected! port %d, %s", num,
                websocket_data_server.port(), payload);
      websocket_data_server.pushMSG(num, "Welcome to ESP3D-X V1\n");
    } break;
    case WStype_TEXT:
      esp3d_log_d("[%u] get Text: %s port %d", num, payload,
                websocket_data_server.port());
      websocket_data_server.push2RXbuffer(payload, length);
      break;
    case WStype_BIN:
      esp3d_log_d("[%u] get binary length: %u port %d", num, length,
                websocket_data_server.port());
      websocket_data_server.handleV1Binary(num, payload, length);
      break;
    default:
      break;
  }
}
#endif  // WS_DATA_FEATURE
#if defined(HTTP_FEATURE)
// Events for Websocket used in WebUI for events and serial bridge
void handle_Websocket_Terminal_Event(uint8_t num, uint8_t type,
                                     uint8_t *payload, size_t length) {
  (void)payload;
  (void)length;
  String msg;
  switch (type) {
    case WStype_DISCONNECTED:
      esp3d_log_d("[%u] Socket Disconnected port %d!", num,
                websocket_terminal_server.port());
      break;
    case WStype_CONNECTED: {
      esp3d_log_d("[%u] Connected! port %d, %s", num,
                websocket_terminal_server.port(), (const char *)payload);
      msg = "currentID:" + String(num);
      // send message to client
      websocket_terminal_server.set_currentID(num);
      websocket_terminal_server.pushMSG(num, msg.c_str());
      msg = "activeID:" + String(num);
      websocket_terminal_server.pushMSG(msg.c_str());
      printer_link_service.notifyWebUi(num);
      websocket_terminal_server.enableOnly(num);
      esp3d_log_d("[%u] Socket connected port %d", num,
                websocket_terminal_server.port());
    } break;
    case WStype_TEXT:
#if defined(AUTHENTICATION_FEATURE)
      // we do not expect any input but ping to get session timeout if any
      if (AuthenticationService::getSessionTimeout() != 0) {
        msg = (const char *)payload;
        if (msg.startsWith("PING:")) {
          String session = msg.substring(5);
          String response =
              "PING:" + String(AuthenticationService::getSessionRemaining(
                            session.c_str()));
          response += ":" + String(AuthenticationService::getSessionTimeout());
          websocket_terminal_server.pushMSG(num, response.c_str());
        }
      }
#endif  // AUTHENTICATION_FEATURE
        // esp3d_log_d("[IGNORED][%u] get Text: %s  port %d", num, payload,
        // websocket_terminal_server.port());
      break;
    case WStype_BIN:
      // we do not expect any input
      // esp3d_log_d("[IGNORED][%u] get binary length: %u  port %d", num,
      // length, websocket_terminal_server.port());
      break;
    default:
      break;
  }
}
#endif  // HTTP_FEATURE

int WebSocket_Server::available() { return _RXbufferSize; }
int WebSocket_Server::availableForWrite() {
  return TXBUFFERSIZE - _TXbufferSize;
}
WebSocket_Server::WebSocket_Server(const char *protocol, ESP3DClientType type) {
  _websocket_server = nullptr;
  _started = false;
  _port = 0;
  _current_id = 0;
  _RXbuffer = nullptr;
  _RXbufferSize = 0;
  _protocol = protocol;
  _type = type;
#if defined(WS_DATA_FEATURE) && defined(FILESYSTEM_FEATURE)
  _transferState = 'O';
  _transferExpectedSize = 0;
  _transferProcessedSize = 0;
  _transferLastPacketId = 0;
#endif
  initAuthentication();
}
WebSocket_Server::~WebSocket_Server() { end(); }
bool WebSocket_Server::begin(uint16_t port) {
  end();
  if (port == 0) {
    _port = ESP3DSettings::readUint32(ESP_HTTP_PORT) + 1;
  } else {
    _port = port;
    if (ESP3DSettings::readByte(ESP_WEBSOCKET_ON) == 0) {
      return true;
    }
  }
  _websocket_server = new WebSocketsServer(_port, "", _protocol.c_str());
  if (_websocket_server) {
    _websocket_server->begin();
#if defined(HTTP_FEATURE)  // terminal websocket for HTTP
    if (port == 0) {
      _websocket_server->onEvent(handle_Websocket_Terminal_Event);
    }
#endif                        // HTTP_FEATURE
#if defined(WS_DATA_FEATURE)  // terminal websocket for HTTP
    if ((port != 0) && _protocol != "log") {
      _websocket_server->onEvent(handle_Websocket_Server_Event);
      _RXbuffer = (uint8_t *)malloc(RXBUFFERSIZE + 1);
      if (!_RXbuffer) {
        return false;
      }
    }
#endif  // WS_DATA_FEATURE
    _started = true;
  } else {
    end();
  }
  return _started;
}

void WebSocket_Server::end() {
  _current_id = 0;
  _TXbufferSize = 0;
  if (_RXbuffer) {
    free(_RXbuffer);
    _RXbuffer = nullptr;
  }
  _RXbufferSize = 0;
  if (_websocket_server) {
    _websocket_server->close();
    delete _websocket_server;
    _websocket_server = nullptr;
    _port = 0;
  }
  _started = false;
#if defined(WS_DATA_FEATURE) && (defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE))
  if (_transferState != 'O') {
#if defined(FILESYSTEM_FEATURE)
    if (_transferTargetFS == 1) {
      if (_transferFileFS) _transferFileFS.close();
      ESP_FileSystem::releaseFS();
    }
#endif
#if defined(SD_DEVICE)
    if (_transferTargetFS == 2) {
      if (_transferFileSD) _transferFileSD.close();
      ESP_SD::setState(ESP_SDCARD_IDLE);
      ESP_SD::releaseFS();
    }
#endif
  }
  _transferState = 'O';
  _transferTargetFS = 0;
#endif
  initAuthentication();
}

WebSocket_Server::operator bool() const { return _started; }

void WebSocket_Server::set_currentID(uint8_t current_id) {
  _current_id = current_id;
}
uint8_t WebSocket_Server::get_currentID() { return _current_id; }

// size_t WebSocket_Server::write(uint8_t c) { return write(&c, 1); }

size_t WebSocket_Server::writeBytes(const uint8_t *buffer, size_t size) {
  if (_started) {
    if ((buffer == nullptr) || (!_websocket_server) || (size == 0)) {
      return 0;
    }
    if (_TXbufferSize == 0) {
      _lastTXflush = millis();
    }
    // send full line
    if (_TXbufferSize + size > TXBUFFERSIZE) {
      flushTXbuffer();
    }
    if (_websocket_server->connectedClients() == 0) {
      return 0;
    }
    // need periodic check to force to flush in case of no end
    for (uint i = 0; i < size; i++) {
      // add a sanity check to avoid buffer overflow
      if (_TXbufferSize >= TXBUFFERSIZE) {
        flushTXbuffer();
      }
      _TXbuffer[_TXbufferSize] = buffer[i];
      _TXbufferSize++;
    }
    return size;
  }
  return 0;
}

void WebSocket_Server::push2RXbuffer(uint8_t *sbuf, size_t len) {
   if (!_RXbuffer || !_started || !sbuf) {
    return;
  }
  for (size_t i = 0; i < len; i++) {
    _lastRXflush = millis();
    if (esp3d_string::isRealTimeCommand(sbuf[i])) {
      flushRXChar(sbuf[i]);
    } else {
      _RXbuffer[_RXbufferSize] = sbuf[i];
      _RXbufferSize++;
      if (_RXbufferSize > RXBUFFERSIZE ||
          _RXbuffer[_RXbufferSize - 1] == '\n') {
        flushRXbuffer();
      }
    }
  }
}

void WebSocket_Server::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}
ESP3DAuthenticationLevel WebSocket_Server::getAuthentication() { return _auth; }

void WebSocket_Server::flushRXChar(char c) {
  flushRXData((uint8_t *)&c, 1, ESP3DMessageType::realtimecmd);
}

void WebSocket_Server::flushRXbuffer() {
  _RXbuffer[_RXbufferSize] = 0x0;
  flushRXData((uint8_t *)_RXbuffer, _RXbufferSize, ESP3DMessageType::unique);
  _RXbufferSize = 0;
}

void WebSocket_Server::flushRXData(const uint8_t *data, size_t size,
                                   ESP3DMessageType type) {
  if (!data || !_started) {
    return;
  }
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      _type, esp3d_commands.getOutputClient(), 
      data, size, _auth);

  if (message) {
    message->type = type;
    esp3d_log_d("Process Message");
    esp3d_commands.process(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastRXflush = millis();
}

void WebSocket_Server::handle() {
  ESP3DHal::wait(0);
  if (_started) {
    if (_TXbufferSize > 0) {
      if ((_TXbufferSize >= TXBUFFERSIZE) ||
          ((millis() - _lastTXflush) > FLUSHTIMEOUT)) {
        flushTXbuffer();
      }
    }
    if (_RXbufferSize > 0) {
      if ((_RXbufferSize >= RXBUFFERSIZE) ||
          ((millis() - _lastRXflush) > FLUSHTIMEOUT)) {
        flushRXbuffer();
      }
    }
    if (_websocket_server) {
      _websocket_server->loop();
    }
  }
}

void WebSocket_Server::flush(void) {
  flushTXbuffer();
  flushRXbuffer();
}

void WebSocket_Server::flushTXbuffer(void) {
  if (_started) {
    if ((_TXbufferSize > 0) && (_websocket_server->connectedClients() > 0)) {
      if (_websocket_server) {
        _websocket_server->broadcastBIN(_TXbuffer, _TXbufferSize);
        esp3d_log_d("WS Broadcast bin port %d: %d bytes", port(), _TXbufferSize);
      }
      // refresh timout
      _lastTXflush = millis();
    }
  }
  // reset buffer
  _TXbufferSize = 0;
}

void WebSocket_Server::handleV1Binary(uint8_t num, uint8_t *payload, size_t length) {
#if defined(WS_DATA_FEATURE) && (defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE))
  if (length < 4) return;
  
  char op[3] = {(char)payload[0], (char)payload[1], 0};
  uint16_t plen = payload[2] | (payload[3] << 8);
  if (length < 4U + plen) return;
  
  uint8_t *pdata = payload + 4;
  esp3d_log_d("V1 Binary: length=%u, op=%s, plen=%u", length, op, plen);
  
  auto close_transfer = [&]() {
    if (_transferState == 'O') return;
#if defined(FILESYSTEM_FEATURE)
    if (_transferTargetFS == 1) {
      if (_transferFileFS) _transferFileFS.close();
      ESP_FileSystem::releaseFS();
    }
#endif
#if defined(SD_DEVICE)
    if (_transferTargetFS == 2) {
      if (_transferFileSD) _transferFileSD.close();
      ESP_SD::setState(ESP_SDCARD_IDLE);
      ESP_SD::releaseFS();
    }
#endif
    _transferState = 'O';
    _transferTargetFS = 0;
  };

  if (strcmp(op, "SR") == 0) {
    uint8_t resp[6] = {'R', 'S', 2, 0, _transferState, 1};
    if (_websocket_server) _websocket_server->sendBIN(num, resp, 6);
  } else if (strcmp(op, "SU") == 0) {
    if (_transferState != 'O') {
      uint8_t resp[5] = {'U', 'S', 1, 0, 'B'};
      if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
      return;
    }
    if (plen < 1) return;
    uint8_t path_len = pdata[0];
    if (plen < 1U + path_len + 1U) return;
    uint8_t name_len = pdata[1 + path_len];
    if (plen < 1U + path_len + 1U + name_len + 4U) return;
    
    String path = "";
    for (int i = 0; i < path_len; i++) path += (char)pdata[1 + i];
    String name = "";
    for (int i = 0; i < name_len; i++) name += (char)pdata[1 + path_len + 1 + i];
    
    _transferExpectedSize = pdata[1 + path_len + 1 + name_len] | 
                           (pdata[1 + path_len + 1 + name_len + 1] << 8) | 
                           (pdata[1 + path_len + 1 + name_len + 2] << 16) | 
                           (pdata[1 + path_len + 1 + name_len + 3] << 24);
    _transferProcessedSize = 0;
    
    esp3d_log_d("SU: path='%s' name='%s' size=%u", path.c_str(), name.c_str(), _transferExpectedSize);
    
    String fullpath = path;
    if (fullpath.length() > 0 && fullpath[fullpath.length()-1] != '/' && name.length() > 0 && name[0] != '/') {
      fullpath += "/";
    }
    fullpath += name;
    
    _transferTargetFS = 0;
#if defined(FILESYSTEM_FEATURE)
    if (fullpath.startsWith("/fs/")) {
      fullpath.remove(0, 3);
      _transferTargetFS = 1;
    } else if (fullpath.startsWith("/fs")) {
      fullpath.remove(0, 3);
      if (fullpath.length() == 0) fullpath = "/";
      _transferTargetFS = 1;
    }
#endif
#if defined(SD_DEVICE)
    if (fullpath.startsWith("/sd/")) {
      fullpath.remove(0, 3);
      _transferTargetFS = 2;
    } else if (fullpath.startsWith("/sd")) {
      fullpath.remove(0, 3);
      if (fullpath.length() == 0) fullpath = "/";
      _transferTargetFS = 2;
    }
#endif

    if (_transferTargetFS == 0) {
      esp3d_log_e("SU: Invalid or unsupported FS prefix");
      uint8_t resp[5] = {'U', 'S', 1, 0, 'E'};
      if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
      return;
    }

    esp3d_log_d("SU: targetFS=%d fullpath='%s'", _transferTargetFS, fullpath.c_str());

#if defined(FILESYSTEM_FEATURE)
    if (_transferTargetFS == 1) {
      if (ESP_FileSystem::accessFS()) {
        if (ESP_FileSystem::exists(fullpath.c_str())) {
          ESP_FileSystem::remove(fullpath.c_str());
        }
        _transferFileFS = ESP_FileSystem::open(fullpath.c_str(), ESP_FILE_WRITE);
        if (_transferFileFS) {
          esp3d_log_d("SU: File open success (FS)");
          _transferState = 'U';
          uint8_t resp[5] = {'U', 'S', 1, 0, 'O'};
          if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
        } else {
          esp3d_log_e("SU: File open error for '%s'", fullpath.c_str());
          ESP_FileSystem::releaseFS();
          uint8_t resp[5] = {'U', 'S', 1, 0, 'E'};
          if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
        }
      } else {
        esp3d_log_e("SU: Cannot access FS");
        uint8_t resp[5] = {'U', 'S', 1, 0, 'B'};
        if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
      }
    }
#endif

#if defined(SD_DEVICE)
    if (_transferTargetFS == 2) {
      if (ESP_SD::accessFS()) {
        if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
          ESP_SD::releaseFS();
          esp3d_log_e("SU: SD card not present");
          uint8_t resp[5] = {'U', 'S', 1, 0, 'E'};
          if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
        } else {
          ESP_SD::setState(ESP_SDCARD_BUSY);
          if (ESP_SD::exists(fullpath.c_str())) {
            ESP_SD::remove(fullpath.c_str());
          }
          _transferFileSD = ESP_SD::open(fullpath.c_str(), ESP_FILE_WRITE);
          if (_transferFileSD) {
            esp3d_log_d("SU: File open success (SD)");
            _transferState = 'U';
            uint8_t resp[5] = {'U', 'S', 1, 0, 'O'};
            if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
          } else {
            esp3d_log_e("SU: File open error for '%s'", fullpath.c_str());
            ESP_SD::setState(ESP_SDCARD_IDLE);
            ESP_SD::releaseFS();
            uint8_t resp[5] = {'U', 'S', 1, 0, 'E'};
            if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
          }
        }
      } else {
        esp3d_log_e("SU: Cannot access SD");
        uint8_t resp[5] = {'U', 'S', 1, 0, 'B'};
        if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
      }
    }
#endif
  } else if (strcmp(op, "UP") == 0) {
    if (_transferState != 'U' || plen < 4) {
      esp3d_log_e("UP: error, state=%c, plen=%u", _transferState, plen);
      uint8_t resp[9] = {'P', 'U', 5, 0, 'E', 0xFF, 0xFF, 0xFF, 0xFF};
      if (plen >= 4) {
        resp[5] = pdata[0]; resp[6] = pdata[1]; resp[7] = pdata[2]; resp[8] = pdata[3];
      }
      if (_websocket_server) _websocket_server->sendBIN(num, resp, 9);
      return;
    }
    uint32_t pktid = pdata[0] | (pdata[1] << 8) | (pdata[2] << 16) | (pdata[3] << 24);
    size_t chunk_size = plen - 4;
    
    if (chunk_size > 0) {
      size_t written = 0;
#if defined(FILESYSTEM_FEATURE)
      if (_transferTargetFS == 1 && _transferFileFS) {
        written = _transferFileFS.write(pdata + 4, chunk_size);
      }
#endif
#if defined(SD_DEVICE)
      if (_transferTargetFS == 2 && _transferFileSD) {
        written = _transferFileSD.write(pdata + 4, chunk_size);
      }
#endif
      if (written == chunk_size) {
        _transferProcessedSize += written;
        uint8_t resp[9] = {'P', 'U', 5, 0, 'O', pdata[0], pdata[1], pdata[2], pdata[3]};
        if (_websocket_server) _websocket_server->sendBIN(num, resp, 9);
      } else {
        esp3d_log_e("UP: File write error, written %u / %u", written, chunk_size);
        uint8_t resp[9] = {'P', 'U', 5, 0, 'E', pdata[0], pdata[1], pdata[2], pdata[3]};
        if (_websocket_server) _websocket_server->sendBIN(num, resp, 9);
      }
    } else {
      uint8_t resp[9] = {'P', 'U', 5, 0, 'O', pdata[0], pdata[1], pdata[2], pdata[3]};
      if (_websocket_server) _websocket_server->sendBIN(num, resp, 9);
    }
  } else if (strcmp(op, "EU") == 0) {
    close_transfer();
    uint8_t resp[5] = {'U', 'E', 1, 0, 'O'};
    if (_websocket_server) _websocket_server->sendBIN(num, resp, 5);
  } else if (strcmp(op, "CM") == 0) {
    if (plen >= 1 && pdata[0] == 'A') {
      close_transfer();
    }
  }
#else
  push2RXbuffer(payload, length);
#endif
}

#endif  // HTTP_FEATURE || WS_DATA_FEATURE
