/*
  esp3desp3dmsg.h -  output functions class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "esp3d_message.h"

#include "../include/esp3d_config.h"

ESP3DRequest no_id{.id = 0};
#if defined(ESP_LOG_FEATURE)
int msg_counting = 0;
#endif  // ESP_LOG_FEATURE

bool ESP3DMessageManager::deleteMsg(ESP3DMessage* message) {
  if (!message) return false;
  if (message->data) {
    free(message->data);
  }
  free(message);
  message = NULL;
  esp3d_log("Deletion : Now we have %ld msg", --msg_counting);
  return true;
}

ESP3DMessage* ESP3DMessageManager::newMsg() {
  ESP3DMessage* newMsgPtr = (ESP3DMessage*)malloc(sizeof(ESP3DMessage));
  if (newMsgPtr) {
    esp3d_log("Creation : Now we have %ld msg", ++msg_counting);
    // esp3d_log("Creation : Now we have %ld msg", ++msg_counting);
    newMsgPtr->data = nullptr;
    newMsgPtr->size = 0;
    newMsgPtr->origin = ESP3DClientType::no_client;
    newMsgPtr->target = ESP3DClientType::all_clients;
    newMsgPtr->authentication_level = ESP3DAuthenticationLevel::guest;
    newMsgPtr->request_id.id = millis();
    newMsgPtr->type = ESP3DMessageType::head;
  } else {
    esp3d_log_e("Out of memory");
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(ESP3DRequest requestId) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = ESP3DClientType::command;
    newMsgPtr->request_id = requestId;
  }
  return newMsgPtr;
}

bool ESP3DMessageManager::copyMsgInfos(ESP3DMessage* newMsgPtr,
                                       ESP3DMessage msg) {
  if (!newMsgPtr) {
    return false;
  }
  newMsgPtr->origin = msg.origin;
  newMsgPtr->target = msg.target;
  newMsgPtr->authentication_level = msg.authentication_level;
  newMsgPtr->request_id = msg.request_id;
  newMsgPtr->type = msg.type;
  return true;
}

ESP3DMessage* ESP3DMessageManager::copyMsgInfos(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    copyMsgInfos(newMsgPtr, msg);
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::copyMsg(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg(msg.origin, msg.target, msg.data, msg.size,
                                   msg.authentication_level);
  if (newMsgPtr) {
    newMsgPtr->request_id = msg.request_id;
    newMsgPtr->type = msg.type;
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target, const uint8_t* data,
    size_t length, ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg(origin, target, authentication_level);
  if (newMsgPtr) {
    if (!setDataContent(newMsgPtr, data, length)) {
      deleteMsg(newMsgPtr);
      newMsgPtr = nullptr;
    }
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target,
    ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = origin;
    newMsgPtr->target = target;
    newMsgPtr->authentication_level = authentication_level;
  }
  return newMsgPtr;
}

bool ESP3DMessageManager::setDataContent(ESP3DMessage* msg, const uint8_t* data,
                                         size_t length) {
  if (!msg) {
    esp3d_log_e("no valid msg container");
    return false;
  }
  if (!data || length == 0) {
    esp3d_log_e("no data to set");
    return false;
  }
  if (msg->data) {
    free(msg->data);
  }

  // add some security in case data is called as string so add 1 byte for \0
  msg->data = (uint8_t*)malloc(sizeof(uint8_t) * (length + 1));
  if (msg->data) {
    memcpy(msg->data, data, length);
    msg->size = length;
    msg->data[length] =
        '\0';  // add some security in case data is called as string
    esp3d_log("Data content set to %s", msg->data);
    return true;
  }
  esp3d_log_e("Out of memory");
  return false;
}

#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../modules/serial/serial_service.h"
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../modules/serial2socket/serial2socket.h"
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "esp3d_settings.h"
#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../modules/websocket/websocket_server.h"
#endif  // HTTP_FEATURE || WS_DATA_FEATURE
#if defined(BLUETOOTH_FEATURE)
#include "../modules/bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE
#if defined(TELNET_FEATURE)
#include "../modules/telnet/telnet_server.h"
#endif  // TELNET_FEATURE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../modules/mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(GCODE_HOST_FEATURE)
#include "../modules/gcode_host/gcode_host.h"
#endif  // GCODE_HOST_FEATURE
#if defined(HTTP_FEATURE)
#include "../modules/http/http_server.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
#endif  // HTTP_FEATURE
#if defined(DISPLAY_DEVICE)
#include "../modules/display/display.h"
#endif  // DISPLAY_DEVICE

const uint8_t activeClients[] = {
#if !(defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)) && \
    (COMMUNICATION_PROTOCOL == RAW_SERIAL ||                  \
     COMMUNICATION_PROTOCOL == MKS_SERIAL)
    ESP_SERIAL_CLIENT,
#endif  // ESP_SERIAL_CLIENT
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    ESP_SERIAL_BRIDGE_CLIENT,
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
    ESP_TELNET_CLIENT,
#endif  // TELNET_FEATURE
#if defined(HTTP_FEATURE)
    ESP_HTTP_CLIENT,
#endif  // HTTP_FEATURE
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
    ESP_REMOTE_SCREEN_CLIENT,
#endif  // defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
#if defined(BLUETOOTH_FEATURE)
    ESP_BT_CLIENT,
#endif  // BLUETOOTH_FEATURE
#if defined(DISPLAY_DEVICE)
    ESP_SCREEN_CLIENT,
#endif  // DISPLAY_DEVICE
#if defined(WS_DATA_FEATURE)
    ESP_WEBSOCKET_CLIENT,
#endif  // WS_DATA_FEATURE
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    ESP_SOCKET_SERIAL_CLIENT,
    ESP_ECHO_SERIAL_CLIENT,
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    ESP_NO_CLIENT};

// tool function to avoid string corrupt JSON files
const char* ESP3D_Message::encodeString(const char* s) {
  static String tmp;
  tmp = s;
  while (tmp.indexOf("'") != -1) {
    tmp.replace("'", "&#39;");
  }
  while (tmp.indexOf("\"") != -1) {
    tmp.replace("\"", "&#34;");
  }
  if (tmp == "") {
    tmp = " ";
  }
  return tmp.c_str();
}

void ESP3D_Message::toScreen(uint8_t output_type, const char* s) {
  switch (output_type) {
    case ESP_OUTPUT_IP_ADDRESS:
#ifdef DISPLAY_DEVICE
      esp3d_display.updateIP();
#endif  // DISPLAY_DEVICE
      break;
    case ESP_OUTPUT_STATUS:
#ifdef DISPLAY_DEVICE
      esp3d_display.setStatus(s);
#endif  // DISPLAY_DEVICE
      break;
    case ESP_OUTPUT_PROGRESS:
#ifdef DISPLAY_DEVICE
      esp3d_display.progress((uint8_t)atoi(s));
#endif  // DISPLAY_DEVICE
      break;
    case ESP_OUTPUT_STATE:
#ifdef DISPLAY_DEVICE
      switch (atoi(s)) {
        case ESP_STATE_DISCONNECTED:
          esp3d_display.setStatus("Disconnected");
          break;
        default:
          break;
      }
#endif  // DISPLAY_DEVICE
      break;
    default:
      (void)s;
      break;
  }
}

// constructor
ESP3D_Message::ESP3D_Message(uint8_t target, uint8_t origin) {
  _target = target;
  _origin = origin;

#ifdef HTTP_FEATURE
  _code = 200;
  _headerSent = false;
  _footerSent = false;
  _webserver = nullptr;
#endif  // HTTP_FEATURE
}

uint8_t ESP3D_Message::target(uint8_t targetId) {
  _target = targetId;
  return _target;
}

uint8_t ESP3D_Message::origin(uint8_t originId) {
  _origin = originId;
  return _origin;
}

#ifdef HTTP_FEATURE
// constructor
ESP3D_Message::ESP3D_Message(WEBSERVER* webserver, uint8_t target,
                             uint8_t origin) {
  _target = ESP_HTTP_CLIENT;
  _code = 200;
  _headerSent = false;
  _footerSent = false;
  _webserver = webserver;
}
#endif  // HTTP_FEATURE

// destructor
ESP3D_Message::~ESP3D_Message() { flush(); }

size_t ESP3D_Message::dispatch(const uint8_t* sbuf, size_t len,
                               uint8_t ignoreClient) {
  esp3d_log("Dispatch %d chars from client %d and ignore %d", len, _target,
            ignoreClient);
#if defined(GCODE_HOST_FEATURE)
  if (!(_target == ESP_STREAM_HOST_CLIENT ||
        ESP_STREAM_HOST_CLIENT == ignoreClient)) {
    esp3d_log("Dispatch  to gcode host");
    esp3d_gcode_host.push(sbuf, len);
  }
#endif  // GCODE_HOST_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  if (!(_target == ESP_SERIAL_CLIENT || ESP_SERIAL_CLIENT == ignoreClient)) {
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    esp3d_log("Dispatch  to gcode frame");
    MKSService::sendGcodeFrame((const char*)sbuf);
#else
    esp3d_log("Dispatch  to serial service");
    esp3d_serial_service.writeBytes(sbuf, len);
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
  }
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
  if (!(_target == ESP_SOCKET_SERIAL_CLIENT ||
        ESP_SOCKET_SERIAL_CLIENT == ignoreClient)) {
    esp3d_log("Dispatch to serial socket client %d is %d,  or is %d", _target,
              ESP_SOCKET_SERIAL_CLIENT, ignoreClient);
    Serial2Socket.push(sbuf, len);
  }
  if (!(_target == ESP_ECHO_SERIAL_CLIENT ||
        ESP_ECHO_SERIAL_CLIENT == ignoreClient ||
        _target == ESP_SOCKET_SERIAL_CLIENT)) {
    esp3d_log("Dispatch to echo serial");
    MYSERIAL1.write(sbuf, len);
  }
#endif                     // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if defined(HTTP_FEATURE)  // no need to block it never
  if (!((_target == ESP_WEBSOCKET_TERMINAL_CLIENT) ||
        (_target == ESP_HTTP_CLIENT) ||
        (ESP_WEBSOCKET_TERMINAL_CLIENT == ignoreClient) ||
        (ESP_HTTP_CLIENT == ignoreClient))) {
    if (websocket_terminal_server) {
      esp3d_log("Dispatch websocket terminal");
      websocket_terminal_server.write(sbuf, len);
    }
  }
#endif  // HTTP_FEATURE
#if defined(BLUETOOTH_FEATURE)
  if (!(_target == ESP_BT_CLIENT || ESP_BT_CLIENT == ignoreClient)) {
    if (bt_service.started()) {
      esp3d_log("Dispatch to bt");
      bt_service.write(sbuf, len);
    }
  }
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  if (!(_target == ESP_SERIAL_BRIDGE_CLIENT ||
        ESP_SERIAL_BRIDGE_CLIENT == ignoreClient)) {
    if (serial_bridge_service.started()) {
      esp3d_log("Dispatch to serial bridge");
      serial_bridge_service.write(sbuf, len);
    }
  }
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
  if (!(_target == ESP_TELNET_CLIENT || ESP_TELNET_CLIENT == ignoreClient)) {
    if (telnet_server.started()) {
      esp3d_log("Dispatch  to telnet");
      telnet_server.write(sbuf, len);
    }
  }
#endif  // TELNET_FEATURE
#if defined(WS_DATA_FEATURE)
  if (!(_target == ESP_WEBSOCKET_CLIENT ||
        ESP_WEBSOCKET_CLIENT == ignoreClient)) {
    if (websocket_data_server.started()) {
      esp3d_log("Dispatch to websocket data server");
      websocket_data_server.write(sbuf, len);
    }
  }
#endif  // WS_DATA_FEATURE
  return len;
}

// Flush
void ESP3D_Message::flush() {
  switch (_target) {
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
      esp3d_serial_service.flush();
      break;
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#ifdef HTTP_FEATURE
    case ESP_HTTP_CLIENT:
      if (_webserver) {
        if (_headerSent && !_footerSent) {
          _webserver->sendContent("");
          _footerSent = true;
        }
      }
      break;
#endif  // HTTP_FEATURE
#if defined(BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
      bt_service.flush();
      break;
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
      serial_bridge_service.flush();
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
      telnet_server.flush();
      break;
#endif  // TELNET_FEATURE
    case ESP_ALL_CLIENTS:
      // do nothing because there are side effects
      break;
    default:
      break;
  }
}

size_t ESP3D_Message::printLN(const char* s) {
  switch (_target) {
    case ESP_HTTP_CLIENT:
      if (strlen(s) > 0) {
        println(s);
        return strlen(s) + 1;
      } else {
        println(" ");
        return strlen(s) + 2;
      }
      return 0;
    case ESP_TELNET_CLIENT:
      print(s);
      println("\r");
      return strlen(s) + 2;
    default:
      break;
  }
  return println(s);
}

size_t ESP3D_Message::printMSGLine(const char* s) {
  if (_target == ESP_ALL_CLIENTS) {
    // process each client one by one
    esp3d_log("PrintMSG to all clients");
    for (uint8_t c = 0; c < sizeof(activeClients); c++) {
      if (activeClients[c]) {
        esp3d_log("Sending PrintMSG to client %d", activeClients[c]);
        _target = activeClients[c];
        printMSG(s);
      }
    }
    _target = ESP_ALL_CLIENTS;
    return strlen(s);
  }

  String display;
  esp3d_log("PrintMSG to client %d", _target);
  if (_target == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
    if (_webserver) {
      if (!_headerSent && !_footerSent) {
        _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
        _webserver->sendHeader("Content-Type", "text/html");
        _webserver->sendHeader("Cache-Control", "no-cache");
        HTTP_Server::set_http_headers();
        _webserver->send(_code);
        _headerSent = true;
      }
      if (_headerSent && !_footerSent) {
        _webserver->sendContent_P((const char*)s, strlen(s));
        _webserver->sendContent_P((const char*)"\n", 1);
        return strlen(s + 1);
      }
    }

#endif  // HTTP_FEATURE
    return 0;
  }
  // this is not supposed to be displayed on any screen
  if (_target == ESP_SCREEN_CLIENT || _target == ESP_REMOTE_SCREEN_CLIENT ||
      _target == ESP_SCREEN_CLIENT) {
    return print(s);
  }
  switch (ESP3DSettings::GetFirmwareTarget()) {
    case GRBL:
      display = "[MSG:";
      display += s;
      display += "]";
      break;
    case MARLIN_EMBEDDED:
    case MARLIN:
      if (((_target == ESP_ECHO_SERIAL_CLIENT) ||
           (_target == ESP_STREAM_HOST_CLIENT)) &&
          (strcmp(s, "ok") == 0)) {
        return 0;
      }

      if (_target == ESP_ECHO_SERIAL_CLIENT) {
        display = "echo:";
      } else {
        display = ";echo:";
      }

      display += s;
      break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:

      display = ";";

      display += s;
  }

  return printLN(display.c_str());
}

size_t ESP3D_Message::printMSG(const char* s, bool withNL) {
  if (_target == ESP_ALL_CLIENTS) {
    // process each client one by one
    esp3d_log("PrintMSG to all clients");
    for (uint8_t c = 0; c < sizeof(activeClients); c++) {
      if (activeClients[c]) {
        esp3d_log("Sending PrintMSG to client %d", activeClients[c]);
        _target = activeClients[c];
        printMSG(s, withNL);
      }
    }
    _target = ESP_ALL_CLIENTS;
    return strlen(s);
  }

  String display;
  esp3d_log("PrintMSG to client %d", _target);
  if (_target == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
    if (_webserver) {
      if (!_headerSent && !_footerSent) {
        _webserver->sendHeader("Cache-Control", "no-cache");
        HTTP_Server::set_http_headers();
#ifdef ESP_ACCESS_CONTROL_ALLOW_ORIGIN
        _webserver->sendHeader("Access-Control-Allow-Origin", "*");
#endif  // ESP_ACCESS_CONTROL_ALLOw_ORIGIN
        _webserver->send(_code, "text/plain", s);
        _headerSent = true;
        _footerSent = true;
        return strlen(s);
      }
    }
#endif  // HTTP_FEATURE
    return 0;
  }

  if (_target == ESP_SCREEN_CLIENT) {
    return print(s);
  }
  switch (ESP3DSettings::GetFirmwareTarget()) {
    case GRBL:
      display = "[MSG:";
      display += s;
      display += "]";
      break;
    case MARLIN_EMBEDDED:
    case MARLIN:
      if (((_target == ESP_ECHO_SERIAL_CLIENT) ||
           (_target == ESP_STREAM_HOST_CLIENT)) &&
          (strcmp(s, "ok") == 0)) {
        return 0;
      }
      if (_target == ESP_REMOTE_SCREEN_CLIENT) {
#if defined(HAS_SERIAL_DISPLAY)
        display = HAS_SERIAL_DISPLAY;
#endif  // HAS_REMOTE_SCREEN
        display += "M117 ";
        withNL = true;
        esp3d_log("Screen should display %s%s", display.c_str(), s);
      } else {
        if (_target == ESP_ECHO_SERIAL_CLIENT) {
          display = "echo:";
        } else {
          display = ";echo:";
        }
      }
      display += s;
      break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:
      if (_target == ESP_REMOTE_SCREEN_CLIENT) {
        display = "M117 ";
        withNL = true;
      } else {
        display = ";";
      }
      display += s;
  }
  if (withNL) {
    return printLN(display.c_str());
  } else {
    return print(display.c_str());
  }
}

size_t ESP3D_Message::printERROR(const char* s, int code_error) {
  String display = "";

  if (_target == ESP_SCREEN_CLIENT) {
    return print(s);
  }
  if (_target == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
    (void)code_error;
    if (_webserver) {
      if (!_headerSent && !_footerSent) {
        _webserver->sendHeader("Cache-Control", "no-cache");
        HTTP_Server::set_http_headers();
        if (s[0] != '{') {
          display = "error: ";
        } else {
          display = "";
        }
        display += s;
        _webserver->send(code_error, "text/plain", display.c_str());
        _headerSent = true;
        _footerSent = true;
        return display.length();
      }
    }
#endif  // HTTP_FEATURE
    return 0;
  }
  switch (ESP3DSettings::GetFirmwareTarget()) {
    case GRBL:
      if (s[0] != '{') {
        display = "error: ";
      }
      display += s;
      break;
    case MARLIN_EMBEDDED:
    case MARLIN:
      if (s[0] != '{') {
        display = "error: ";
      }
      display += s;
      break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:
      if (s[0] != '{') {
        display = ";error: ";
      }
      display += s;
  }
  return printLN(display.c_str());
}

int ESP3D_Message::availableforwrite() {
  switch (_target) {
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
      return true;
      // return esp3d_serial_service.availableForWrite();
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
      return serial_bridge_service.availableForWrite();
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
      return bt_service.availableForWrite();
      break;
#endif  // BLUETOOTH_FEATURE
#if defined(TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
      return telnet_server.availableForWrite();
      break;
#endif  // TELNET_FEATURE
#if defined(WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
      return websocket_data_server.availableForWrite();
      break;
#endif  // WS_DATA_FEATURE
    case ESP_ALL_CLIENTS:
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
      return true;
      // return esp3d_serial_service.availableForWrite();
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
    default:
      break;
  }
  return 0;
}
/*
size_t ESP3D_Message::write(uint8_t c) {
  switch (_target) {
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
      return esp3d_serial_service.write(c);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_ECHO_SERIAL_CLIENT:
      return MYSERIAL1.write(c);
    case ESP_SOCKET_SERIAL_CLIENT:
      return Serial2Socket.write(c);
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if defined(BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
      return bt_service.write(c);
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
      return serial_bridge_service.write(c);
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
      return telnet_server.write(c);
#endif  // TELNET_FEATURE
#if defined(WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
      return websocket_data_server.write(c);
#endif  // WS_DATA_FEATURE
    case ESP_ALL_CLIENTS:
#if defined(BLUETOOTH_FEATURE)
      bt_service.write(c);
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
      serial_bridge_service.write(c);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
      telnet_server.write(c);
#endif  // TELNET_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
      esp3d_serial_service.write(c);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      MYSERIAL1.write(c);
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      return 1;
    default:
      return 0;
  }
}*/

size_t ESP3D_Message::write(const uint8_t* buffer, size_t size) {
  switch (_target) {
#ifdef HTTP_FEATURE
    case ESP_HTTP_CLIENT:
      if (_webserver) {
        if (!_headerSent && !_footerSent) {
          _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
          _webserver->sendHeader("Content-Type", "text/html");
          _webserver->sendHeader("Cache-Control", "no-cache");
          HTTP_Server::set_http_headers();
          _webserver->send(_code);
          _headerSent = true;
        }
        if (_headerSent && !_footerSent) {
          _webserver->sendContent_P((const char*)buffer, size);
        }
      }
      break;
#endif  // HTTP_FEATURE
#if defined(DISPLAY_DEVICE)
    case ESP_SCREEN_CLIENT:
      esp3d_display.setStatus((const char*)buffer);
      return size;
#endif  // DISPLAY_DEVICE
#if defined(BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
      return bt_service.write(buffer, size);
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
      return serial_bridge_service.write(buffer, size);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
      return telnet_server.write(buffer, size);
#endif  // TELNET_FEATURE
#if defined(WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
      return websocket_data_server.write(buffer, size);
#endif  // WS_DATA_FEATURE
#if defined(GCODE_HOST_FEATURE)
    case ESP_STREAM_HOST_CLIENT: {
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      esp3d_log(
          "ESP_STREAM_HOST_CLIENT do a dispatch to all clients but socket "
          "serial");
      dispatch(buffer, size, ESP_SOCKET_SERIAL_CLIENT);
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
      esp3d_log(
          "ESP_STREAM_HOST_CLIENT do a dispatch to all clients but serial");
      dispatch(buffer, size, ESP_SERIAL_CLIENT);
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    }
      return size;
      break;
#endif  // GCODE_HOST_FEATURE

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_REMOTE_SCREEN_CLIENT:
    case ESP_SERIAL_CLIENT:
      return esp3d_serial_service.writeBytes(buffer, size);
      break;
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_REMOTE_SCREEN_CLIENT:
      esp3d_log("Writing to remote screen: %s", buffer);
      return Serial2Socket.push(buffer, size);
      break;
    case ESP_ECHO_SERIAL_CLIENT:
      return MYSERIAL1.write(buffer, size);
      break;
    case ESP_SOCKET_SERIAL_CLIENT:
      return Serial2Socket.push(buffer, size);
      break;
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_ALL_CLIENTS:
#if defined(BLUETOOTH_FEATURE)
      bt_service.write(buffer, size);
#endif  // BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
      serial_bridge_service.write(buffer, size);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(TELNET_FEATURE)
      telnet_server.write(buffer, size);
#endif  // TELNET_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
      esp3d_serial_service.writeBytes(buffer, size);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      MYSERIAL1.write(buffer, size);
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      return size;
    default:
      break;
  }
  return 0;
}