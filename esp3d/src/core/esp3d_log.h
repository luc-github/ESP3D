/*
  esp3d_log.h - esp3d log functions

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
/*
void esp3d_logf(uint8_t level, const char* format, ...);


void esp3d_logf(uint8_t level, const char* format, ...) {
    #if ESP_LOG_FEATURE == ESP_LOG_FEATURE
   //TODO: if not started => return
    #endif // ESP_LOG_FEATURE == ESP_LOG_FEATURE

    char    default_buffer[64];
    char*   buffer_ptr = default_buffer;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);

    //add before format [ESP3D] / [ESP3D-DEBUG] / [ESP3D-ERROR] according level

size_t len = vsnprintf(NULL, 0, format, arg);
va_end(copy);
if (len >= sizeof(default_buffer)) {
  buffer_ptr = (char *)malloc((len + 1)sizeof(char));
  if (buffer_ptr == NULL) {
    return;
  }
    }
    len = vsnprintf(buffer_ptr, len + 1, format, arg);
    #if ESP_LOG_FEATURE == ESP_LOG_FEATURE
   //TODO: log buffer_ptr
    #endif // ESP_LOG_FEATURE == ESP_LOG_FEATURE
    va_end(arg);
    if (buffer_ptr != default_buffer) {
       free(buffer_ptr);
    }
}

*/

#ifndef _LOG_ESP3D_H
#define _LOG_ESP3D_H
#include "../include/esp3d_config.h"
#include "../include/esp3d_defines.h"

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
#define esp3d_log(format, ...)                                               \
  LOG_OUTPUT_SERIAL.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#else
#define esp3d_log(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define esp3d_log_d(format, ...)                                             \
  LOG_OUTPUT_SERIAL.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#else
#define esp3d_log_d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_DEBUG
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define esp3d_log_e(format, ...)                                             \
  LOG_OUTPUT_SERIAL.printf("[ESP3D-ERROR][%s:%u] %s(): " format "\r\n",      \
                           pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                           ##__VA_ARGS__)
#else
#define esp3d_log_e(format, ...)
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
#define esp3d_log(format, ...)                                        \
  if (telnet_log.isConnected())                                       \
  telnet_log.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                    pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                    ##__VA_ARGS__)
#else
#define esp3d_log(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define esp3d_log_d(format, ...)                                      \
  if (telnet_log.isConnected())                                       \
  telnet_log.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                    pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                    ##__VA_ARGS__)
#else
#define esp3d_log_d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define esp3d_log_e(format, ...)                                      \
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
#define esp3d_log(format, ...)                                           \
  websocket_log.printf("[ESP3D][%s:%u] %s(): " format "\r\n",            \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#else
#define esp3d_log(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define esp3d_log_d(format, ...)                                         \
  websocket_log.printf("[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n",      \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#else
#define esp3d_log(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= ESP_LOG_DEBUG

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define esp3d_log(format, ...)                                           \
  websocket_log.printf("[ESP3D-ERROR][%s:%u] %s(): " format "\r\n",      \
                       pathToFileName(__FILE__), __LINE__, __FUNCTION__, \
                       ##__VA_ARGS__)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#endif  // LOG_OUTPUT_WEBSOCKET
#else
#define esp3d_log(format, ...)
#define esp3d_log_e(format, ...)
#define esp3d_log_d(format, ...)
#endif  // ESP_LOG_FEATURE

#endif  //_LOG_ESP3D_H
