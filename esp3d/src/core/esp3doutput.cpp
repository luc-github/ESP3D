/*
  serial_service.cpp -  serial services functions class

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
#include "esp3doutput.h"
#include "../modules/serial/serial_service.h"
#include "settings_esp3d.h"
#if defined (HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../modules/websocket/websocket_server.h"
#endif //HTTP_FEATURE || WS_DATA_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../modules/bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#if defined (TELNET_FEATURE)
#include "../modules/telnet/telnet_server.h"
#endif //TELNET_FEATURE
uint8_t ESP3DOutput::_outputflags = ESP_ALL_CLIENTS;
#if defined (HTTP_FEATURE)
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#endif //HTTP_FEATURE
#if defined (DISPLAY_DEVICE)
#include "../modules/display/display.h"
#endif //DISPLAY_DEVICE

//constructor
ESP3DOutput::ESP3DOutput(uint8_t client)
{
    _client = client;

#ifdef HTTP_FEATURE
    _code = 200;
    _headerSent = false;
    _footerSent = false;
    _webserver = nullptr;
#endif //HTTP_FEATURE
}

#ifdef HTTP_FEATURE
//constructor
ESP3DOutput::ESP3DOutput(WEBSERVER   * webserver)
{
    _client = ESP_HTTP_CLIENT;
    _code = 200;
    _headerSent = false;
    _footerSent = false;
    _webserver = webserver;
}
#endif //HTTP_FEATURE

//destructor
ESP3DOutput::~ESP3DOutput()
{
    flush();
}

bool ESP3DOutput::isOutput(uint8_t flag, bool fromsettings)
{
    if(fromsettings) {
        _outputflags = Settings_ESP3D::read_byte (ESP_OUTPUT_FLAG);
    }
    return ((_outputflags & flag) == flag);

}

size_t ESP3DOutput::dispatch (uint8_t * sbuf, size_t len)
{
    log_esp3d("Dispatch %d to %d", len, _client);
    if (_client != ESP_SERIAL_CLIENT) {
        if (isOutput(ESP_SERIAL_CLIENT)) {
            serial_service.write(sbuf, len);
        }
    }
#if defined (HTTP_FEATURE) //no need to block it never
    if (!((_client == ESP_WEBSOCKET_TERMINAL_CLIENT) || (_client == ESP_HTTP_CLIENT))) {
        if (websocket_terminal_server) {
            websocket_terminal_server.write(sbuf, len);
        }
    }
#endif //HTTP_FEATURE    
#if defined (BLUETOOTH_FEATURE)
    if (_client != ESP_BT_CLIENT) {
        if (isOutput(ESP_BT_CLIENT) && bt_service.started()) {
            bt_service.write(sbuf, len);
        }
    }
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
    if (_client != ESP_TELNET_CLIENT) {
        if (isOutput(ESP_TELNET_CLIENT) && telnet_server.started()) {
            telnet_server.write(sbuf, len);
        }
    }
#endif //TELNET_FEATURE 
#if defined (WS_DATA_FEATURE)
    if (_client != ESP_WEBSOCKET_CLIENT) {
        if (isOutput(ESP_WEBSOCKET_CLIENT) && websocket_data_server.started()) {
            log_esp3d("Dispatch to websocket data server");
            websocket_data_server.write(sbuf, len);
        }
    }
#endif //WS_DATA_FEATURE 
    return len;
}

//Flush
void ESP3DOutput::flush()
{
    if (!isOutput(_client)) {
        return ;
    }
    switch (_client) {
    case ESP_SERIAL_CLIENT:
        serial_service.flush();
        break;
#ifdef HTTP_FEATURE
    case ESP_HTTP_CLIENT:
        if (_webserver) {
            if (_headerSent && !_footerSent) {
                _webserver->sendContent("");
                _footerSent = true;
            }
        }
        break;
#endif //HTTP_FEATURE
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        bt_service.flush();
        break;
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
        telnet_server.flush();
        break;
#endif //TELNET_FEATURE 
    case ESP_ALL_CLIENTS:
        //do nothing because there are side effects
        break;
    default :
        break;
    }
}

size_t ESP3DOutput::printLN(const char * s)
{
    if (!isOutput(_client)) {
        return 0;
    }
    switch(_client) {
    case ESP_HTTP_CLIENT:
        if(strlen(s) > 0) {
            println(s);
            return strlen(s) + 1;
        } else {
            println(" ");
            return strlen(s) + 2;
        }
        return 0;
    case ESP_TELNET_CLIENT:
        print(s);
        println("\r");
        return strlen(s)+2;
    default:
        break;
    }
    return println(s);
}

size_t ESP3DOutput::printMSG(const char * s, bool withNL)
{
    if (!isOutput(_client)) {
        return 0;
    }
    String display;
#ifdef HTTP_FEATURE
    if (_client == ESP_HTTP_CLIENT) {

        if (_webserver) {
            if (!_headerSent && !_footerSent) {
                _webserver->sendHeader("Cache-Control","no-cache");
#ifdef ESP_ACCESS_CONTROL_ALLOW_ORIGIN
                _webserver->sendHeader("Access-Control-Allow-Origin", "*");
#endif //ESP_ACCESS_CONTROL_ALLOw_ORIGIN
                _webserver->send (_code, "text/plain", s);
                _headerSent = true;
                _footerSent = true;
                return strlen(s);
            }
        }
        return 0;
    }
#endif //HTTP_FEATURE
    if (_client & ESP_PRINTER_LCD_CLIENT) {
        if (isOutput(ESP_PRINTER_LCD_CLIENT)) {
            display= "M117 ";
            display+= s;
            return printLN(display.c_str());
        } else {
            return printLN(s);
        }
    }
    if (_client & ESP_SCREEN_CLIENT) {
        print(s);
    }
    switch(Settings_ESP3D::GetFirmwareTarget()) {
    case GRBL:
        display = "[MSG:";
        display += s;
        display += "]";
        break;
    case MARLIN:
    case MARLINKIMBRA:
        display = "echo: ";
        display += s;
        break;
    case REPETIER4DV:
    case SMOOTHIEWARE:
    case REPETIER:
    default:
        display = ";";
        display += s;
    }
    if(withNL) {
        return printLN(display.c_str());
    } else {
        return print(display.c_str());
    }
}

size_t ESP3DOutput::printERROR(const char * s, int code_error)
{
    if (!isOutput(_client)) {
        return 0;
    }
    if (_client == ESP_SCREEN_CLIENT) {
        return print(s);
    }
#ifdef HTTP_FEATURE
    _code = code_error;
    if (_client == ESP_HTTP_CLIENT) {

        if (_webserver) {
            if (!_headerSent && !_footerSent) {
                _webserver->sendHeader("Cache-Control","no-cache");
                _webserver->send (_code, "text/plain", s);
                _headerSent = true;
                _footerSent = true;
                return strlen(s);
            }
        }
        return 0;
    }
#else
    (void)code_error;
#endif //HTTP_FEATURE
    String display;
    switch(Settings_ESP3D::GetFirmwareTarget()) {
    case GRBL:

        display = "error: ";
        display += s;
        break;
    case MARLIN:
    case MARLINKIMBRA:
        display = "error: ";
        display += s;
        break;
    case REPETIER4DV:
    case SMOOTHIEWARE:
    case REPETIER:
    default:
        display = ";error: ";
        display += s;
    }
    return printLN(display.c_str());
}

int ESP3DOutput::availableforwrite()
{
    switch (_client) {
    case ESP_SERIAL_CLIENT:
        return serial_service.availableForWrite();
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        return bt_service.availableForWrite();
        break;
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
        return telnet_server.availableForWrite();
        break;
#endif //TELNET_FEATURE 
#if defined (WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
        return websocket_data_server.availableForWrite();
        break;
#endif //WS_DATA_FEATURE
    case ESP_ALL_CLIENTS:
        return serial_service.availableForWrite();
    default :
        break;
    }
    return  0;
}
size_t ESP3DOutput::write(uint8_t c)
{
    if (!isOutput(_client)) {
        return 0;
    }
    switch (_client) {
    case ESP_SERIAL_CLIENT:
        return serial_service.write(c);
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        if(bt_service.started()) {
            return bt_service.write(c);
        }
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
        return telnet_server.write(c);
#endif //TELNET_FEATURE 
#if defined (WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
        return websocket_data_server.write(c);
#endif //WS_DATA_FEATURE 
    case ESP_ALL_CLIENTS:
#if defined (BLUETOOTH_FEATURE)
        if(bt_service.started()) {
            bt_service.write(c);
        }
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
        if(telnet_server.started()) {
            telnet_server.write(c);
        }
#endif //TELNET_FEATURE 
        return serial_service.write(c);
    default :
        return 0;
    }
}

size_t ESP3DOutput::write(const uint8_t *buffer, size_t size)
{
    if (!isOutput(_client)) {
        return 0;
    }
    switch (_client) {
#ifdef HTTP_FEATURE
    case ESP_HTTP_CLIENT:

        if (_webserver) {
            if (!_headerSent && !_footerSent) {
                _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
                _webserver->sendHeader("Content-Type","text/html");
                _webserver->sendHeader("Cache-Control","no-cache");
                _webserver->send(_code);
                _headerSent = true;
            }
            if (_headerSent && !_footerSent) {
                _webserver->sendContent_P((const char*)buffer,size);
            }
        }
        break;
#endif //HTTP_FEATURE
#if defined (DISPLAY_DEVICE)
    case ESP_SCREEN_CLIENT:
        esp3d_display.SetStatus((const char *)buffer);
        return size;
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        if(bt_service.started()) {
            return bt_service.write(buffer, size);
        }
#endif //BLUETOOTH_FEATURE 
#if defined (TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
        if(telnet_server.started()) {
            return telnet_server.write(buffer, size);
        }
#endif //TELNET_FEATURE
#if defined (WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
        if(websocket_data_server.started()) {
            return websocket_data_server.write(buffer, size);
        }
#endif //WS_DATA_FEATURE
    case ESP_PRINTER_LCD_CLIENT:
    case ESP_SERIAL_CLIENT:
        return serial_service.write(buffer, size);
    case ESP_ALL_CLIENTS:
#if defined (BLUETOOTH_FEATURE)
        if(bt_service.started()) {
            bt_service.write(buffer, size);
        }
#endif //BLUETOOTH_FEATURE
#if defined (TELNET_FEATURE)
        if(telnet_server.started()) {
            telnet_server.write(buffer, size);
        }
#endif //TELNET_FEATURE
        return serial_service.write(buffer, size);
    default :
        break;
    }
    return 0;
}

void ESP3DGlobalOutput::SetStatus(const char * status)
{
#ifdef DISPLAY_DEVICE
    esp3d_display.SetStatus(status);
#else
    (void)status;
#endif //DISPLAY_DEVICE
}
void ESP3DGlobalOutput::display_progress(uint8_t v)
{
#ifdef DISPLAY_DEVICE
    esp3d_display.progress(v);
#else
    (void)v;
#endif //DISPLAY_DEVICE
}

void ESP3DGlobalOutput::display_IP(bool force)
{
#ifdef DISPLAY_DEVICE
    esp3d_display.display_IP(force);
#else
    (void)force;
#endif //DISPLAY_DEVICE
}

