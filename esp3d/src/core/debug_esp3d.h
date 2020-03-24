/*
  debug_esp3d.h - esp3d debug functions

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

#ifndef _DEBUG_ESP3D_H
#define _DEBUG_ESP3D_H

#include "../include/esp3d_config.h"
#if defined(ARDUINO_ARCH_ESP32)
#define log_esp3d(format, ...) log_d(format, ##__VA_ARGS__)
#define log_esp3dS(format, ...) log_d(format, ##__VA_ARGS__)
#else
#define log_esp3d(format, ...)
#define log_esp3dS(format, ...)
#endif
#define DEBUG_ESP3D_INIT
#define DEBUG_ESP3D_NETWORK_INIT
#define DEBUG_ESP3D_NETWORK_HANDLE
#define DEBUG_ESP3D_NETWORK_END
#if defined(ESP_DEBUG_FEATURE)
#if defined(ARDUINO_ARCH_ESP8266)
extern const char * pathToFileName(const char * path);
#endif //ARDUINO_ARCH_ESP8266
#undef log_esp3d
#undef log_esp3dS
//Serial
#if (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0) || (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1) || (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2)

#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0
#define DEBUG_OUTPUT_SERIAL Serial
#endif //DEBUG_OUTPUT_SERIAL0
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1
#define DEBUG_OUTPUT_SERIAL Serial1
#endif //DEBUG_OUTPUT_SERIAL1
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2
#define DEBUG_OUTPUT_SERIAL Serial2
#endif //DEBUG_OUTPUT_SERIAL2
#undef DEBUG_ESP3D_INIT
#define DEBUG_ESP3D_INIT DEBUG_OUTPUT_SERIAL.begin(115200);
#define log_esp3d(format, ...) DEBUG_OUTPUT_SERIAL.printf("[ESP3D][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define log_esp3dS(format, ...) DEBUG_OUTPUT_SERIAL.printf(format "\r\n", ##__VA_ARGS__)
#endif //DEBUG_OUTPUT_SERIAL0 || DEBUG_OUTPUT_SERIAL1 || DEBUG_OUTPUT_SERIAL2

//Telnet
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET
#include "../modules/telnet/telnet_server.h"
extern Telnet_Server telnet_debug;
#undef DEBUG_ESP3D_NETWORK_INIT
#undef DEBUG_ESP3D_NETWORK_END
#undef DEBUG_ESP3D_NETWORK_HANDLE
#define DEBUG_ESP3D_NETWORK_INIT telnet_debug.begin(DEBUG_ESP3D_OUTPUT_PORT, true);
#define DEBUG_ESP3D_NETWORK_HANDLE telnet_debug.handle();
#define DEBUG_ESP3D_NETWORK_END telnet_debug.end();
#define log_esp3d(format, ...) telnet_debug.printf("[ESP3D][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define log_esp3dS(format, ...) telnet_debug.printf(format "\r\n", ##__VA_ARGS__)
#endif // DEBUG_OUTPUT_TELNET

//Telnet
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
#include "../modules/websocket/websocket_server.h"
extern WebSocket_Server websocket_debug;
#undef DEBUG_ESP3D_NETWORK_INIT
#undef DEBUG_ESP3D_NETWORK_END
#undef DEBUG_ESP3D_NETWORK_HANDLE
#define DEBUG_ESP3D_NETWORK_INIT websocket_debug.begin(DEBUG_ESP3D_OUTPUT_PORT, true);
#define DEBUG_ESP3D_NETWORK_HANDLE websocket_debug.handle();
#define DEBUG_ESP3D_NETWORK_END websocket_debug.end();
#define log_esp3d(format, ...) websocket_debug.printf("[ESP3D][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define log_esp3dS(format, ...) websocket_debug.printf(format "\r\n", ##__VA_ARGS__)
#endif // DEBUG_OUTPUT_WEBSOCKET

#endif //ESP_DEBUG_FEATURE
#endif //_DEBUG_ESP3D_H 
