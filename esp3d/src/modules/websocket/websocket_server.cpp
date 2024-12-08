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
#include "websocket_server.h"

WebSocket_Server websocket_terminal_server("webui-v3",
                                           ESP3DClientType::webui_websocket);
#if defined(WS_DATA_FEATURE)
WebSocket_Server websocket_data_server("arduino", ESP3DClientType::websocket);
#endif  // WS_DATA_FEATURE
bool WebSocket_Server::pushMSG(const char *data) {
  if (_websocket_server) {
    esp3d_log("[%u]Broadcast %s", _current_id, data);
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
    esp3d_log("[%u]Send %s", num, data);
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
      esp3d_log("[%u] Disconnected! port %d", num,
                websocket_data_server.port());
      break;
    case WStype_CONNECTED: {
      websocket_data_server.initAuthentication();
      esp3d_log("[%u] Connected! port %d, %s", num,
                websocket_data_server.port(), payload);
    } break;
    case WStype_TEXT:
      esp3d_log("[%u] get Text: %s port %d", num, payload,
                websocket_data_server.port());
      websocket_data_server.push2RXbuffer(payload, length);
      break;
    case WStype_BIN:
      esp3d_log("[%u] get binary length: %u port %d", num, length,
                websocket_data_server.port());
      websocket_data_server.push2RXbuffer(payload, length);
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
      esp3d_log("[%u] Socket Disconnected port %d!", num,
                websocket_terminal_server.port());
      break;
    case WStype_CONNECTED: {
      esp3d_log("[%u] Connected! port %d, %s", num,
                websocket_terminal_server.port(), (const char *)payload);
      msg = "currentID:" + String(num);
      // send message to client
      websocket_terminal_server.set_currentID(num);
      websocket_terminal_server.pushMSG(num, msg.c_str());
      msg = "activeID:" + String(num);
      websocket_terminal_server.pushMSG(msg.c_str());
      websocket_terminal_server.enableOnly(num);
      esp3d_log("[%u] Socket connected port %d", num,
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
        // esp3d_log("[IGNORED][%u] get Text: %s  port %d", num, payload,
        // websocket_terminal_server.port());
      break;
    case WStype_BIN:
      // we do not expect any input
      // esp3d_log("[IGNORED][%u] get binary length: %u  port %d", num,
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
    esp3d_log("Process Message");
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
        esp3d_log("WS Broadcast bin port %d: %d bytes", port(), _TXbufferSize);
      }
      // refresh timout
      _lastTXflush = millis();
    }
  }
  // reset buffer
  _TXbufferSize = 0;
}

#endif  // HTTP_FEATURE || WS_DATA_FEATURE
