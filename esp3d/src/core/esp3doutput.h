/*
  esp3Doutput.h -  out functions class

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
#define ESP_PRINTER_LCD_CLIENT          16
#define ESP_BT_CLIENT                   32
#define ESP_SCREEN_CLIENT               64
#define ESP_WEBSOCKET_CLIENT            128
#define ESP_ALL_CLIENTS                 255

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
    uint8_t client()
    {
        return _client;
    }
    size_t dispatch (uint8_t * sbuf, size_t len);
    size_t printMSG(const char * s, bool withNL = true);
    size_t printERROR(const char * s, int code_error = 500);
    size_t printLN(const char * s);
    void flush();
    int availableforwrite();
    static bool isOutput(uint8_t flag, bool fromsettings = false);
private:
    uint8_t _client;
#ifdef HTTP_FEATURE
    int _code;
    bool _headerSent;
    bool _footerSent;
    WEBSERVER * _webserver;
#endif //HTTP_FEATURE
    static uint8_t _outputflags;
};

class ESP3DGlobalOutput
{
public:
    static void SetStatus(const char * status);
    static void display_progress(uint8_t v);
    static void display_IP(bool force = false);
};

#endif //_ESP3DOUTPUT_H

