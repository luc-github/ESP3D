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

#pragma once

#include "../include/esp3d_config.h"
#include "../include/esp3d_defines.h"
#if defined(ESP_LOG_FEATURE)
extern void esp3d_logf(uint8_t level, const char* format, ...);
extern void esp3d_network_log_init();
extern void esp3d_network_log_handle();
extern void esp3d_network_log_end();

extern void esp3d_log_init();

#if !defined(ESP3D_DEBUG_LEVEL)
#define ESP3D_DEBUG_LEVEL LOG_LEVEL_NONE
#endif  // ESP3D_DEBUG_LEVEL
#if defined(ARDUINO_ARCH_ESP8266)
// no need with latest esp8266 core
#define pathToFileName(p) p
#endif  // ARDUINO_ARCH_ESP8266

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_VERBOSE
#define esp3d_log(format, ...)                                                 \
  esp3d_logf(LOG_LEVEL_VERBOSE, "[ESP3D-VERBOSE][%s:%u] %s(): " format "\r\n", \
             pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define esp3d_log(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_VERBOSE

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define esp3d_log_d(format, ...)                                           \
  esp3d_logf(LOG_LEVEL_DEBUG, "[ESP3D-DEBUG][%s:%u] %s(): " format "\r\n", \
             pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define esp3d_log_d(format, ...)
#endif  // ESP3D_DEBUG_LEVEL>= LOG_LEVEL_DEBUG

#if ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define esp3d_log_e(format, ...)                                           \
  esp3d_logf(LOG_LEVEL_ERROR, "[ESP3D-ERROR][%s:%u] %s(): " format "\r\n", \
             pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ESP3D_LOG_INIT_FN esp3d_log_init();
#define ESP3D_LOG_NETWORK_INIT_FN esp3d_network_log_init();
#define ESP3D_LOG_NETWORK_HANDLE_FN esp3d_network_log_handle();
#define ESP3D_LOG_NETWORK_END_FN esp3d_network_log_end();
#else
#define esp3d_log_e(format, ...)
#endif  // ESP3D_DEBUG_LEVEL >= LOG_LEVEL_ERROR

#else
#define esp3d_log_e(format, ...)
#define esp3d_log_d(format, ...)
#define esp3d_log(format, ...)
#undef ESP3D_DEBUG_LEVEL
#define ESP3D_LOG_INIT_FN
#define ESP3D_LOG_NETWORK_INIT_FN
#define ESP3D_LOG_NETWORK_HANDLE_FN
#define ESP3D_LOG_NETWORK_END_FN
#endif  // ESP_LOG_FEATURE