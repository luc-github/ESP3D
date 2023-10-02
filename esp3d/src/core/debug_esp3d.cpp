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

#ifndef DEBUG_BAUDRATE
#define DEBUG_BAUDRATE 115200
#endif //~DEBUG_BAUDRATE

void initDebug()
{
#if (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0) || (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1)||(ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2)
#ifdef ARDUINO_ARCH_ESP8266
    DEBUG_OUTPUT_SERIAL.begin(DEBUG_BAUDRATE, SERIAL_8N1, SERIAL_FULL, (ESP_DEBUG_TX_PIN == -1)?1:ESP_DEBUG_TX_PIN);
#if ESP_DEBUG_RX_PIN != -1
    DEBUG_OUTPUT_SERIAL.pins((ESP_DEBUG_TX_PIN == -1)?1:ESP_DEBUG_TX_PIN, ESP_DEBUG_RX_PIN)
#endif //ESP_DEBUG_RX_PIN != -1
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    DEBUG_OUTPUT_SERIAL.begin (DEBUG_BAUDRATE, SERIAL_8N1, ESP_DEBUG_RX_PIN, ESP_DEBUG_TX_PIN);
#endif //ARDUINO_ARCH_ESP32

#endif // (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0) || (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1)||(ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2)
}

//Telnet
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET
Telnet_Server telnet_debug;
#endif // ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET

//Websocket
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
WebSocket_Server websocket_debug("debug");
#endif // ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
#endif //ESP_DEBUG_FEATURE
