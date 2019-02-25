/*
  debug_esp3d.h - esp3d debug functions

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _DEBUG_ESP3D_H
#define _DEBUG_ESP3D_H

#include "../include/esp3d_config.h"
#define log_esp3d(format, ...)
#define log_esp3dS(format, ...)
#define DEBUG_ESP3D_INIT
#undef DEBUG_ESP3D

//Serial
#if defined(DEBUG_OUTPUT_SERIAL0) || defined(DEBUG_OUTPUT_SERIAL1) || defined(DEBUG_OUTPUT_SERIAL2)
#if defined(ARDUINO_ARCH_ESP8266)
extern const char * pathToFileName(const char * path);
#endif //ARDUINO_ARCH_ESP8266
#undef DEBUG_ESP3D_INIT
#undef log_esp3d
#undef log_esp3dS
#define DEBUG_ESP3D
#ifdef DEBUG_OUTPUT_SERIAL0
#define DEBUG_OUTPUT_SERIAL Serial
#endif //DEBUG_OUTPUT_SERIAL0
#ifdef DEBUG_OUTPUT_SERIAL1
#define DEBUG_OUTPUT_SERIAL Serial1
#endif //DEBUG_OUTPUT_SERIAL1
#ifdef DEBUG_OUTPUT_SERIAL2
#define DEBUG_OUTPUT_SERIAL Serial2
#endif //DEBUG_OUTPUT_SERIAL2
#define DEBUG_ESP3D_INIT DEBUG_OUTPUT_SERIAL.begin(115200);
#define log_esp3d(format, ...) DEBUG_OUTPUT_SERIAL.printf("[ESP3D][%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define log_esp3dS(format, ...) DEBUG_OUTPUT_SERIAL.printf(format "\r\n", ##__VA_ARGS__)
#endif //defined(DEBUG_OUTPUT_SERIAL0) || defined(DEBUG_OUTPUT_SERIAL1) || defined(DEBUG_OUTPUT_SERIAL2)

#endif //_DEBUG_ESP3D_H 
