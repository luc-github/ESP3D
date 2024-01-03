/*
  http_server.h -  http server functions class

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

#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H
#include "../../core/esp3d_commands.h"
#include "../../include/esp3d_config.h"

// class WebSocketsServer;
#if defined(ARDUINO_ARCH_ESP32)
class WebServer;
#define WEBSERVER WebServer
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER ESP8266WebServer
#endif  // ARDUINO_ARCH_ESP8266

// Upload status
typedef enum {
  UPLOAD_STATUS_NONE = 0,
  UPLOAD_STATUS_FAILED = 1,
  UPLOAD_STATUS_CANCELLED = 2,
  UPLOAD_STATUS_SUCCESSFUL = 3,
  UPLOAD_STATUS_ONGOING = 4
} upload_status_type;

class HTTP_Server {
 public:
  static bool begin();
  static void end();
  static void handle();
  static bool started() { return _started; }
  static uint16_t port() { return _port; }
  static void set_http_headers();
  static bool dispatch(ESP3DMessage* msg);

 private:
  static void pushError(int code, const char* st, uint16_t web_error = 500,
                        uint16_t timeout = 1000);
  static void cancelUpload();
  static bool _started;
  static WEBSERVER* _webserver;
  static uint16_t _port;
  static uint8_t _upload_status;
  static const char* get_Splited_Value(String data, char separator, int index);
#ifdef SSDP_FEATURE
  static void handle_SSDP();
#endif  // SSDP_FEATURE
#ifdef CAMERA_DEVICE
  static void handle_snap();
#endif  // CAMERA_DEVICE

  static void init_handlers();
  static bool StreamFSFile(const char* filename, const char* contentType);
  static void handle_root();
  static void handle_login();
  static void handle_not_found();
  static void handle_web_command();
  static void handle_config();
  // static void handle_Websocket_Event(uint8_t num, uint8_t type, uint8_t *
  // payload, size_t length);
#ifdef FILESYSTEM_FEATURE
  static void FSFileupload();
  static void handleFSFileList();
#endif  // FILESYSTEM_FEATURE
#ifdef WEB_UPDATE_FEATURE
  static void handleUpdate();
  static void WebUpdateUpload();
#endif  // WEB_UPDATE_FEATURE
  // static bool is_realtime_cmd(char c);
#ifdef SD_DEVICE
  static void SDFileupload();
  static void handleSDFileList();
  static bool StreamSDFile(const char* filename, const char* contentType);
#endif  // SD_DEVICE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  static void MKSFileupload();
  static void handleMKSUpload();
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
};

extern ESP3DRequest code_200;
extern ESP3DRequest code_500;
extern ESP3DRequest code_404;
extern ESP3DRequest code_401;

#endif  //_HTTP_SERVER_H
