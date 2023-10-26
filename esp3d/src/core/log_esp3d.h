/*
  log_esp3d.h - esp3d log functions

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

#ifndef _LOG_ESP3D_H
#define _LOG_ESP3D_H
#include "../include/defines.h"
#include "../include/esp3d_config.h"

#ifndef ESP3D_DEBUG_LEVEL
#define ESP3D_DEBUG_LEVEL LOG_LEVEL_NONE
#endif  // ESP3D_DEBUG_LEVEL

#define LOG_ESP3D_INIT
#define LOG_ESP3D_NETWORK_INIT
#define LOG_ESP3D_NETWORK_HANDLE
#define LOG_ESP3D_NETWORK_END

#if defined(ESP_LOG_FEATURE)
#if defined(ARDUINO_ARCH_ESP8266)
// no need with latest esp8266 core
#define pathToFileName(p) p
#endif  // ARDUINO_ARCH_ESP8266

#undef log_esp3d
#undef log_esp3ds
#undef log_esp3d_e
#undef log_esp3d_d
// Serial
#if (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)

extern void initEsp3dLog();
#ifndef ESP3DLIB_ENV
#if ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0
#define LOG_OUTPUT_SERIAL Serial
#endif  // LOG_OUTPUT_SERIAL0
#if ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1
#define LOG_OUTPUT_SERIAL Serial1
#endif  // LOG_OUTPUT_SERIAL1
#if ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2
#define LOG_OUTPUT_SERIAL Serial2
#endif  // LOG_OUTPUT_SERIAL2
#undef LOG_ESP3D_INIT
#define LOG_ESP3D_INIT initEsp3dLog();
#else
#define LOG_OUTPUT_SERIAL MYSERIAL1

#endif  // ESP3DLIB_ENV

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE
#define log_esp3d(format, ...)                                               \
  LOG_OUTPUT_SERIAL.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#define log_esp3ds(format, ...) LOG_OUTPUT_SERIAL.printf(format, ##__VA_ARGS__)
#else
#define log_esp3d(format, ...)
#define log_esp3ds(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define log_esp3d_d(format, ...)                                             \
  LOG_OUTPUT_SERIAL.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#else
#define log_esp3d_d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_DEBUG
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define log_esp3d_e(format, ...)                                             \
  LOG_OUTPUT_SERIAL.printf("[ESP3D-ERROR][%s:%u] %s(): " format "\r\n",      \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#else
#define log_esp3d_e(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#endif  // LOG_OUTPUT_SERIAL0 || LOG_OUTPUT_SERIAL1 || LOG_OUTPUT_SERIAL2

// Telnet
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#include "../modules/telnet/telnet_server.h"
extern Telnet_Server telnet_log;
#undef LOG_ESP3D_NETWORK_INIT
#undef LOG_ESP3D_NETWORK_END
#undef LOG_ESP3D_NETWORK_HANDLE
#define LOG_ESP3D_NETWORK_INIT telnet_log.begin(LOG_ESP3D_OUTPUT_PORT, true);
#define LOG_ESP3D_NETWORK_HANDLE telnet_log.handle();
#define LOG_ESP3D_NETWORK_END telnet_log.end();
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE
#define log_esp3d(format, ...)                                        \
  if (telnet_log.isConnected())                                       \
  telnet_log.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                    pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                    ##__VA_ARGS__)
#define log_esp3ds(format, ...) \
  if (telnet_log.isConnected()) telnet_log.printf(format, ##__VA_ARGS__)
#else
#define log_esp3d(format, ...)
#define log_esp3ds(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define log_esp3d_d(format, ...)                                      \
  if (telnet_log.isConnected())                                       \
  telnet_log.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                    pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                    ##__VA_ARGS__)
#else
#define log_esp3d_d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define log_esp3d_e(format, ...)                                      \
  if (telnet_log.isConnected())                                       \
  telnet_log.printf("[ESP3D-ERROR][%s:%u] %s(): " format "\r\n",      \
                    pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                    ##__VA_ARGS__)

#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#endif  // LOG_OUTPUT_TELNET

// Telnet
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
#include "../modules/websocket/websocket_server.h"
extern WebSocket_Server websocket_log;
#undef LOG_ESP3D_NETWORK_INIT
#undef LOG_ESP3D_NETWORK_END
#undef LOG_ESP3D_NETWORK_HANDLE
#define LOG_ESP3D_NETWORK_INIT websocket_log.begin(LOG_ESP3D_OUTPUT_PORT, true);
#define LOG_ESP3D_NETWORK_HANDLE websocket_log.handle();
#define LOG_ESP3D_NETWORK_END websocket_log.end();
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE
#define log_esp3d(format, ...)                                           \
  websocket_log.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#define log_esp3ds(format, ...) websocket_log.printf(format, ##__VA_ARGS__)
#else
#define log_esp3d(format, ...)
#define log_esp3ds(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define log_esp3d_d(format, ...)                                         \
  websocket_log.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#else
#define log_esp3d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= ESP_LOG_DEBUG

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define log_esp3d(format, ...)                                           \
  websocket_log.printf("[ESP3D-ERROR][%s:%u] %s(): " format "\r\n",      \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#endif  // LOG_OUTPUT_WEBSOCKET
#else
#define log_esp3d(format, ...)
#define log_esp3ds(format, ...)
#define log_esp3d_e(format, ...)
#define log_esp3d_d(format, ...)
#endif  // ESP_LOG_FEATURE

#endif  //_LOG_ESP3D_H
