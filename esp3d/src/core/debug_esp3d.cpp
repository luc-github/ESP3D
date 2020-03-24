/*
  debug_esp3d.cpp -  debug esp3d functions class

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

#include "../include/esp3d_config.h"
#if defined(ESP_DEBUG_FEATURE)

#if defined(ARDUINO_ARCH_ESP8266)
const char * pathToFileName(const char * path)
{
    size_t i = 0;
    size_t pos = 0;
    char * p = (char *)path;
    while(*p) {
        i++;
        if(*p == '/' || *p == '\\') {
            pos = i;
        }
        p++;
    }
    return path+pos;
}
#endif //ARDUINO_ARCH_ESP8266 

//Telnet
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET
Telnet_Server telnet_debug;
#endif // ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET

//Websocket
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
WebSocket_Server websocket_debug;
#endif // ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
#endif //ESP_DEBUG_FEATURE
