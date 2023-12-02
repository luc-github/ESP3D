/*
  esp3d_log.cpp -  log esp3d functions class

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
#if defined(ESP_LOG_FEATURE)
// telnet
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#include "../modules/telnet/telnet_server.h"
Telnet_Server telnet_log;
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
// Websocket
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
#include "../modules/websocket/websocket_server.h"
WebSocket_Server websocket_log("log");
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET

#ifndef LOG_ESP3D_BAUDRATE
#define LOG_ESP3D_BAUDRATE 115200
#endif  //~LOG_ESP3D_BAUDRATE

#if defined(ARDUINO_ARCH_ESP8266)
// no need with latest esp8266 core
#define pathToFileName(p) p
#endif  // ARDUINO_ARCH_ESP8266

// Serial
#if (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)

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
#else
#define LOG_OUTPUT_SERIAL MYSERIAL1
#endif  // ESP3DLIB_ENV
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0 || ESP_LOG_FEATURE ==
        // LOG_OUTPUT_SERIAL1 || ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2

void esp3d_logf(uint8_t level, const char* format, ...) {
#if (((ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) ||  \
      (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1) ||  \
      (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)) && \
     !defined(ESP3DLIB_ENV))
  if (!LOG_OUTPUT_SERIAL.availableForWrite()) return;
#endif  // ((ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || (ESP_...
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
  if (!telnet_log.started() || !telnet_log.isConnected()) {
    return;
  }
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
  if (!websocket_log.started()) {
    return;
  }
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET

  size_t len = 0;
  char default_buffer[64];
  char* buffer_ptr = default_buffer;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);

  len = vsnprintf(NULL, 0, format, arg);

  va_end(copy);
  if (len >= sizeof(default_buffer)) {
    buffer_ptr = (char*)malloc((len + 1) * sizeof(char));
    if (buffer_ptr == NULL) {
      return;
    }
  }

  len = vsnprintf(buffer_ptr, len + 1, format, arg);

#if (((ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) ||  \
      (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1) ||  \
      (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)) && \
     !defined(ESP3DLIB_ENV))
  LOG_OUTPUT_SERIAL.write((uint8_t*)buffer_ptr, strlen(buffer_ptr));
  LOG_OUTPUT_SERIAL.flush();
#endif  // ((ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || (ESP_...

#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
  telnet_log.writeBytes((uint8_t*)buffer_ptr, strlen(buffer_ptr));
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
  websocket_log.writeBytes((uint8_t*)buffer_ptr, strlen(buffer_ptr));
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET

  va_end(arg);
  if (buffer_ptr != default_buffer) {
    free(buffer_ptr);
  }
}

void esp3d_log_init() {
#if (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1) || \
    (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)
#ifdef ARDUINO_ARCH_ESP8266
  LOG_OUTPUT_SERIAL.begin(LOG_ESP3D_BAUDRATE, SERIAL_8N1, SERIAL_FULL,
                          (ESP_LOG_TX_PIN == -1) ? 1 : ESP_LOG_TX_PIN);
#if ESP_LOG_RX_PIN != -1
  LOG_OUTPUT_SERIAL
      .pins((ESP_LOG_TX_PIN == -1) ? 1 : ESP_LOG_TX_PIN, ESP_LOG_RX_PIN)
#endif  // ESP_LOG_RX_PIN != -1
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
          LOG_OUTPUT_SERIAL.begin(LOG_ESP3D_BAUDRATE, SERIAL_8N1,
                                  ESP_LOG_RX_PIN, ESP_LOG_TX_PIN);
#endif  // ARDUINO_ARCH_ESP32

#endif  // (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0) || (ESP_LOG_FEATURE ==
        // LOG_OUTPUT_SERIAL1)||(ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2)
}

void esp3d_network_log_init() {
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
  telnet_log.begin(LOG_ESP3D_OUTPUT_PORT, true);
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
  websocket_log.begin(LOG_ESP3D_OUTPUT_PORT);
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
}
void esp3d_network_log_handle() {
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
  telnet_log.handle();
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
  websocket_log.handle();
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
}
void esp3d_network_log_end() {
#if ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
  telnet_log.end();
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
#if ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
  websocket_log.end();
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
}

#endif  // ESP_LOG_FEATURE
