/*
  esp3doutput.h -  output functions class

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

#define ESP_NO_CLIENT                   0
#define ESP_SERIAL_CLIENT               1
#define ESP_TELNET_CLIENT               2
#define ESP_HTTP_CLIENT                 4
#define ESP_WEBSOCKET_TERMINAL_CLIENT   8
#define ESP_REMOTE_SCREEN_CLIENT        16
#define ESP_STREAM_HOST_CLIENT          30
#define ESP_BT_CLIENT                   32
#define ESP_SCREEN_CLIENT               64
#define ESP_WEBSOCKET_CLIENT            128
#define ESP_SOCKET_SERIAL_CLIENT        129
#define ESP_ECHO_SERIAL_CLIENT          130
#define ESP_SERIAL_BRIDGE_CLIENT        150
#define ESP_ALL_CLIENTS                 255

#define ESP_STREAM_HOST_OUTPUT        ESP_SERIAL_CLIENT

#define ESP_OUTPUT_IP_ADDRESS          0
#define ESP_OUTPUT_STATUS              1
#define ESP_OUTPUT_PROGRESS            2
#define ESP_OUTPUT_STATE               3

#define ESP_STATE_DISCONNECTED         0

#ifndef _ESP3DOUTPUT_H
#define _ESP3DOUTPUT_H

#include "Print.h"
#include "../include/esp3d_config.h"
#ifdef HTTP_FEATURE
#if defined (ARDUINO_ARCH_ESP32)
class WebServer;
#define WEBSERVER WebServer
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER ESP8266WebServer
#endif //ARDUINO_ARCH_ESP8266
#endif //HTTP_FEATURE


class ESP3DOutput : public Print
{
public:
    ESP3DOutput(uint8_t client = 0);
#ifdef HTTP_FEATURE
    ESP3DOutput(WEBSERVER * webserver);
#endif //HTTP_FEATURE
    ~ESP3DOutput();
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);

    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    uint8_t client(uint8_t client = 0);
    size_t dispatch (const uint8_t * sbuf, size_t len, uint8_t ignoreClient = 0);
    size_t printMSG(const char * s, bool withNL = true);
    size_t printMSGLine(const char * s);
    size_t printERROR(const char * s, int code_error = 500);
    size_t printLN(const char * s);
    void flush();
    int availableforwrite();
    static bool isOutput(uint8_t flag, bool fromsettings = false);
    static void toScreen(uint8_t output_type, const char * s);
    static const char * encodeString(const char * s);
#ifdef HTTP_FEATURE
    bool footerSent()
    {
        return _footerSent;
    }
#endif //HTTP_FEATURE
private:
    uint8_t _client;
#ifdef HTTP_FEATURE
    int _code;
    bool _headerSent;
    bool _footerSent;
    WEBSERVER * _webserver;
#endif //HTTP_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    static uint8_t _serialoutputflags;
#endif //COMMUNICATION_PROTOCOL
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
    static uint8_t _remotescreenoutputflags;
#endif //HAS_DISPLAY || HAS_SERIAL_DISPLAY
#if defined (WS_DATA_FEATURE)
    static uint8_t _websocketoutputflags;
#endif //WS_DATA_FEATURE
#if defined (TELNET_FEATURE)
    static uint8_t _telnetoutputflags;
#endif //TELNET_FEATURE
#if defined (DISPLAY_DEVICE)
    static uint8_t _screenoutputflags;
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
    static uint8_t _BToutputflags;
#endif //BLUETOOTH_FEATURE  
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    static uint8_t _serialBridgeoutputflags;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
};

#endif //_ESP3DOUTPUT_H

