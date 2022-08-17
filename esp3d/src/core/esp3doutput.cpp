/*
  esp3d output.h -  output functions class

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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../include/esp3d_config.h"
#include "esp3doutput.h"
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../modules/serial/serial_service.h"
#endif // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../modules/serial2socket/serial2socket.h"
#endif // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../modules/mks/mks_service.h"
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(GCODE_HOST_FEATURE)
#include "../modules/gcode_host/gcode_host.h"
#endif //GCODE_HOST_FEATURE

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
uint8_t ESP3DOutput::_serialoutputflags = DEFAULT_SERIAL_OUTPUT_FLAG;
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
uint8_t ESP3DOutput::_remotescreenoutputflags = DEFAULT_REMOTE_SCREEN_FLAG;
#endif //HAS_DISPLAY || HAS_SERIAL_DISPLAY
#if defined (WS_DATA_FEATURE)
uint8_t ESP3DOutput::_websocketoutputflags = DEFAULT_WEBSOCKET_FLAG;
#endif //WS_DATA_FEATURE
#if defined (TELNET_FEATURE)
uint8_t ESP3DOutput::_telnetoutputflags = DEFAULT_TELNET_FLAG;
#endif //TELNET_FEATURE
#if defined (DISPLAY_DEVICE)
uint8_t ESP3DOutput::_screenoutputflags = DEFAULT_SCREEN_FLAG;
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
uint8_t ESP3DOutput::_BToutputflags = DEFAULT_BT_FLAG;
#endif //BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
uint8_t ESP3DOutput::_serialBridgeoutputflags = DEFAULT_SERIAL_BRIDGE_FLAG;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
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

const uint8_t activeClients [] = {
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    ESP_SERIAL_CLIENT,
#endif // ESP_SERIAL_CLIENT
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    ESP_SERIAL_BRIDGE_CLIENT,
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if defined (TELNET_FEATURE)
    ESP_TELNET_CLIENT,
#endif //TELNET_FEATURE
#if defined (HTTP_FEATURE)
    ESP_HTTP_CLIENT,
#endif //HTTP_FEATURE
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
    ESP_REMOTE_SCREEN_CLIENT,
#endif // defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
#if defined (BLUETOOTH_FEATURE)
    ESP_BT_CLIENT,
#endif //BLUETOOTH_FEATURE
#if defined (DISPLAY_DEVICE)
    ESP_SCREEN_CLIENT,
#endif //DISPLAY_DEVICE
#if defined (WS_DATA_FEATURE)
    ESP_WEBSOCKET_CLIENT,
#endif //WS_DATA_FEATURE
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    ESP_SOCKET_SERIAL_CLIENT,
    ESP_ECHO_SERIAL_CLIENT,
#endif // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    ESP_NO_CLIENT
};

//tool function to avoid string corrupt JSON files
const char * ESP3DOutput::encodeString(const char * s)
{
    static String tmp;
    tmp = s;
    while(tmp.indexOf("'")!=-1) {
        tmp.replace("'", "&#39;");
    }
    while(tmp.indexOf("\"")!=-1) {
        tmp.replace("\"", "&#34;");
    }
    if (tmp =="") {
        tmp=" ";
    }
    return tmp.c_str();
}

void ESP3DOutput::toScreen(uint8_t output_type, const char * s)
{
    switch (output_type) {
    case ESP_OUTPUT_IP_ADDRESS:
#ifdef DISPLAY_DEVICE
        esp3d_display.updateIP();
#endif //DISPLAY_DEVICE
        break;
    case ESP_OUTPUT_STATUS:
#ifdef DISPLAY_DEVICE
        esp3d_display.setStatus(s);
#endif //DISPLAY_DEVICE
        break;
    case ESP_OUTPUT_PROGRESS:
#ifdef DISPLAY_DEVICE
        esp3d_display.progress((uint8_t)atoi(s));
#endif //DISPLAY_DEVICE
        break;
    case ESP_OUTPUT_STATE:
#ifdef DISPLAY_DEVICE
        switch(atoi(s)) {
        case ESP_STATE_DISCONNECTED:
            esp3d_display.setStatus("Disconnected");
            break;
        default :
            break;
        }
#endif //DISPLAY_DEVICE
        break;
    default:
        (void)s;
        break;
    }
}

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

uint8_t ESP3DOutput::client(uint8_t client )
{
    if(client != 0) {
        _client = client;
    }
    return _client;
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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        _serialoutputflags= Settings_ESP3D::read_byte (ESP_SERIAL_FLAG);
#endif // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        _serialBridgeoutputflags= Settings_ESP3D::read_byte (ESP_SERIAL_BRIDGE_FLAG);
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
        _remotescreenoutputflags= Settings_ESP3D::read_byte (ESP_REMOTE_SCREEN_FLAG);
#endif // defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
#if defined (WS_DATA_FEATURE)
        _websocketoutputflags= Settings_ESP3D::read_byte (ESP_WEBSOCKET_FLAG);
#endif // WS_DATA_FEATURE
#if defined (TELNET_FEATURE)
        _telnetoutputflags= Settings_ESP3D::read_byte (ESP_TELNET_FLAG);
#endif //TELNET_FEATURE
#if defined (DISPLAY_DEVICE)
        _screenoutputflags= Settings_ESP3D::read_byte (ESP_SCREEN_FLAG);
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
        _BToutputflags= Settings_ESP3D::read_byte (ESP_BT_FLAG);
#endif //BLUETOOTH_FEATURE
    }
    switch(flag) {
    case ESP_ECHO_SERIAL_CLIENT:
    case ESP_SERIAL_CLIENT:
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        return _serialoutputflags;
#endif // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        return 0;
    case ESP_SERIAL_BRIDGE_CLIENT:
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        return _serialBridgeoutputflags;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
        return 0;
    case ESP_REMOTE_SCREEN_CLIENT:
#if defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
        return _remotescreenoutputflags;
#endif // defined(HAS_DISPLAY) || defined(HAS_SERIAL_DISPLAY)
        return 0;
    case ESP_WEBSOCKET_CLIENT:
#if defined (WS_DATA_FEATURE)
        return _websocketoutputflags;
#endif // WS_DATA_FEATURE
        return 0;
    case ESP_TELNET_CLIENT:
#if defined (TELNET_FEATURE)
        return _telnetoutputflags;
#endif //TELNET_FEATURE
        return 0;
    case ESP_SCREEN_CLIENT:
#if defined (DISPLAY_DEVICE)
        return _screenoutputflags;
#endif //DISPLAY_DEVICE
        return 0;
    case ESP_BT_CLIENT:
#if defined (BLUETOOTH_FEATURE)
        return _BToutputflags;
#endif //BLUETOOTH_FEATURE
        return 0;
    default:
        return true;
    }
}

size_t ESP3DOutput::dispatch (const uint8_t * sbuf, size_t len, uint8_t ignoreClient)
{
    log_esp3d("Dispatch %d chars from client %d and ignore %d", len, _client, ignoreClient);
#if defined(GCODE_HOST_FEATURE)
    if (!(_client == ESP_STREAM_HOST_CLIENT || ESP_STREAM_HOST_CLIENT==ignoreClient)) {
        log_esp3d("Dispatch  to gcode host");
        esp3d_gcode_host.push(sbuf, len);
    }
#endif //GCODE_HOST_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    if (!(_client == ESP_SERIAL_CLIENT || ESP_SERIAL_CLIENT==ignoreClient)) {
        if (isOutput(ESP_SERIAL_CLIENT)) {
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
            log_esp3d("Dispatch  to gcode frame");
            MKSService::sendGcodeFrame((const char *)sbuf);
#else
            log_esp3d("Dispatch  to serial service");
            serial_service.write(sbuf, len);
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
        }
    }
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    if (!(_client == ESP_SOCKET_SERIAL_CLIENT || ESP_SOCKET_SERIAL_CLIENT==ignoreClient)) {
        log_esp3d("Dispatch to serial socket client %d is %d,  or is %d", _client, ESP_SOCKET_SERIAL_CLIENT, ignoreClient);
        Serial2Socket.push(sbuf, len);
    }
    if (!(_client == ESP_ECHO_SERIAL_CLIENT || ESP_ECHO_SERIAL_CLIENT==ignoreClient ||_client == ESP_SOCKET_SERIAL_CLIENT)) {
        log_esp3d("Dispatch to echo serial");
        MYSERIAL1.write(sbuf, len);
    }
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL 
#if defined (HTTP_FEATURE) //no need to block it never
    if (!((_client == ESP_WEBSOCKET_TERMINAL_CLIENT) || (_client == ESP_HTTP_CLIENT)|| (ESP_WEBSOCKET_TERMINAL_CLIENT==ignoreClient) || (ESP_HTTP_CLIENT==ignoreClient))) {
        if (websocket_terminal_server) {
            log_esp3d("Dispatch websocket terminal");
            websocket_terminal_server.write(sbuf, len);
        }
    }
#endif //HTTP_FEATURE    
#if defined (BLUETOOTH_FEATURE)
    if (!(_client == ESP_BT_CLIENT  || ESP_BT_CLIENT==ignoreClient)) {
        if (isOutput(ESP_BT_CLIENT) && bt_service.started()) {
            log_esp3d("Dispatch to bt");
            bt_service.write(sbuf, len);
        }
    }
#endif //BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    if (!(_client == ESP_SERIAL_BRIDGE_CLIENT || ESP_SERIAL_BRIDGE_CLIENT==ignoreClient)) {
        if (isOutput(ESP_SERIAL_BRIDGE_CLIENT) && serial_bridge_service.started()) {
            log_esp3d("Dispatch to serial bridge");
            serial_bridge_service.write(sbuf, len);
        }
    }
#endif //ESP_SERIAL_BRIDGE_OUTPUT 
#if defined (TELNET_FEATURE)
    if (!(_client == ESP_TELNET_CLIENT || ESP_TELNET_CLIENT==ignoreClient)) {
        if (isOutput(ESP_TELNET_CLIENT) && telnet_server.started()) {
            log_esp3d("Dispatch  to telnet");
            telnet_server.write(sbuf, len);
        }
    }
#endif //TELNET_FEATURE 
#if defined (WS_DATA_FEATURE)
    if (!(_client == ESP_WEBSOCKET_CLIENT || ESP_WEBSOCKET_CLIENT==ignoreClient)) {
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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
        serial_service.flush();
        break;
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
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
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
        serial_bridge_service.flush();
        break;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
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

size_t ESP3DOutput::printMSGLine(const char * s)
{

    if (_client == ESP_ALL_CLIENTS) {
        //process each client one by one
        log_esp3d("PrintMSG to all clients");
        for (uint8_t c=0; c < sizeof(activeClients); c++) {
            if (activeClients[c]) {
                log_esp3d("Sending PrintMSG to client %d", activeClients[c]);
                _client = activeClients[c];
                printMSG(s);
            }
        }
        _client = ESP_ALL_CLIENTS;
        return strlen(s);
    }
    if (!isOutput(_client)) {
        return 0;
    }
    String display;
    log_esp3d("PrintMSG to client %d", _client);
    if (_client == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
        if (_webserver) {
            if (!_headerSent && !_footerSent) {
                _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
                _webserver->sendHeader("Content-Type","text/html");
                _webserver->sendHeader("Cache-Control","no-cache");
                _webserver->send(_code);
                _headerSent = true;
            }
            if (_headerSent && !_footerSent) {
                _webserver->sendContent_P((const char*)s,strlen(s));
                _webserver->sendContent_P((const char*)"\n",1);
                return strlen(s+1);
            }
        }

#endif //HTTP_FEATURE
        return 0;
    }
    //this is not supposed to be displayed on any screen
    if (_client == ESP_SCREEN_CLIENT || _client == ESP_REMOTE_SCREEN_CLIENT ||_client == ESP_SCREEN_CLIENT ) {
        return print(s);
    }
    switch(Settings_ESP3D::GetFirmwareTarget()) {
    case GRBL:
        display = "[MSG:";
        display += s;
        display += "]";
        break;
    case MARLIN_EMBEDDED:
    case MARLIN:
        if (((_client == ESP_ECHO_SERIAL_CLIENT) ||(_client == ESP_STREAM_HOST_CLIENT)) && (strcmp(s, "ok") == 0)) {
            return 0;
        }

        if (_client == ESP_ECHO_SERIAL_CLIENT) {
            display = "echo:";
        } else {
            display = ";echo:";
        }

        display += s;
        break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:

        display = ";";

        display += s;
    }

    return printLN(display.c_str());

}

size_t ESP3DOutput::printMSG(const char * s, bool withNL)
{

    if (_client == ESP_ALL_CLIENTS) {
        //process each client one by one
        log_esp3d("PrintMSG to all clients");
        for (uint8_t c=0; c < sizeof(activeClients); c++) {
            if (activeClients[c]) {
                log_esp3d("Sending PrintMSG to client %d", activeClients[c]);
                _client = activeClients[c];
                printMSG(s, withNL);
            }
        }
        _client = ESP_ALL_CLIENTS;
        return strlen(s);
    }
    if (!isOutput(_client)) {
        return 0;
    }
    String display;
    log_esp3d("PrintMSG to client %d", _client);
    if (_client == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
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
#endif //HTTP_FEATURE
        return 0;
    }

    if (_client == ESP_SCREEN_CLIENT) {
        return print(s);
    }
    switch(Settings_ESP3D::GetFirmwareTarget()) {
    case GRBL:
        display = "[MSG:";
        display += s;
        display += "]";
        break;
    case MARLIN_EMBEDDED:
    case MARLIN:
        if (((_client == ESP_ECHO_SERIAL_CLIENT) ||(_client == ESP_STREAM_HOST_CLIENT)) && (strcmp(s, "ok") == 0)) {
            return 0;
        }
        if (_client == ESP_REMOTE_SCREEN_CLIENT) {
#if defined(HAS_SERIAL_DISPLAY)
            display = HAS_SERIAL_DISPLAY;
#endif //HAS_REMOTE_SCREEN
            display += "M117 ";
            withNL = true;
            log_esp3d("Screen should display %s%s", display.c_str(),s);
        } else {
            if (_client == ESP_ECHO_SERIAL_CLIENT) {
                display = "echo:";
            } else {
                display = ";echo:";
            }
        }
        display += s;
        break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:
        if (_client == ESP_REMOTE_SCREEN_CLIENT) {
            display = "M117 ";
            withNL = true;
        } else {
            display = ";";
        }
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
    String display = "";
    if (!isOutput(_client)) {
        return 0;
    }
    if (_client == ESP_SCREEN_CLIENT) {
        return print(s);
    }
    if (_client == ESP_HTTP_CLIENT) {
#ifdef HTTP_FEATURE
        (void)code_error;
        if (_webserver) {
            if (!_headerSent && !_footerSent) {
                _webserver->sendHeader("Cache-Control","no-cache");
                if (s[0]!='{') {
                    display = "error: ";
                } else {
                    display ="";
                }
                display += s;
                _webserver->send (code_error, "text/plain", display.c_str());
                _headerSent = true;
                _footerSent = true;
                return display.length();
            }
        }
#endif //HTTP_FEATURE
        return 0;
    }
    switch(Settings_ESP3D::GetFirmwareTarget()) {
    case GRBL:
        if (s[0]!='{') {
            display = "error: ";
        }
        display += s;
        break;
    case MARLIN_EMBEDDED:
    case MARLIN:
        if (s[0]!='{') {
            display = "error: ";
        }
        display += s;
        break;
    case SMOOTHIEWARE:
    case REPETIER:
    default:
        if (s[0]!='{') {
            display = ";error: ";
        }
        display += s;
    }
    return printLN(display.c_str());
}

int ESP3DOutput::availableforwrite()
{
    switch (_client) {
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
        return serial_service.availableForWrite();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
        return serial_bridge_service.availableForWrite();
#endif //ESP_SERIAL_BRIDGE_OUTPUT
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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        return serial_service.availableForWrite();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_SERIAL_CLIENT:
        return serial_service.write(c);
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_ECHO_SERIAL_CLIENT:
        return  MYSERIAL1.write(c);
    case ESP_SOCKET_SERIAL_CLIENT:
        return Serial2Socket.write(c);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        return bt_service.write(c);
#endif //BLUETOOTH_FEATURE 
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
        return  serial_bridge_service.write(c);
        break;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
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
        bt_service.write(c);
#endif //BLUETOOTH_FEATURE 
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        serial_bridge_service.write(c);
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if defined (TELNET_FEATURE)
        telnet_server.write(c);
#endif //TELNET_FEATURE 
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        serial_service.write(c);
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        MYSERIAL1.write(c);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        return 1;
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
        esp3d_display.setStatus((const char *)buffer);
        return size;
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
    case ESP_BT_CLIENT:
        return bt_service.write(buffer, size);
#endif //BLUETOOTH_FEATURE 
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP_SERIAL_BRIDGE_CLIENT:
        return serial_bridge_service.write(buffer, size);
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if defined (TELNET_FEATURE)
    case ESP_TELNET_CLIENT:
        return telnet_server.write(buffer, size);
#endif //TELNET_FEATURE
#if defined (WS_DATA_FEATURE)
    case ESP_WEBSOCKET_CLIENT:
        return websocket_data_server.write(buffer, size);
#endif //WS_DATA_FEATURE
#if defined(GCODE_HOST_FEATURE)
    case  ESP_STREAM_HOST_CLIENT: {
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        log_esp3d("ESP_STREAM_HOST_CLIENT do a dispatch to all clients but socket serial");
        dispatch(buffer, size,ESP_SOCKET_SERIAL_CLIENT);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        log_esp3d("ESP_STREAM_HOST_CLIENT do a dispatch to all clients but serial");
        dispatch(buffer, size,ESP_SERIAL_CLIENT);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    }
    return size;
    break;
#endif //GCODE_HOST_FEATURE

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP_REMOTE_SCREEN_CLIENT:
    case ESP_SERIAL_CLIENT:
        return serial_service.write(buffer, size);
        break;
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_REMOTE_SCREEN_CLIENT:
        log_esp3d("Writing to remote screen: %s",buffer);
        return  Serial2Socket.push(buffer, size);
        break;
    case ESP_ECHO_SERIAL_CLIENT:
        return  MYSERIAL1.write(buffer, size);
        break;
    case ESP_SOCKET_SERIAL_CLIENT:
        return  Serial2Socket.push(buffer, size);
        break;
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP_ALL_CLIENTS:
#if defined (BLUETOOTH_FEATURE)
        bt_service.write(buffer, size);
#endif //BLUETOOTH_FEATURE
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        serial_bridge_service.write(buffer, size);
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if defined (TELNET_FEATURE)
        telnet_server.write(buffer, size);
#endif //TELNET_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        serial_service.write(buffer, size);
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        MYSERIAL1.write(buffer, size);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        return size;
    default :
        break;
    }
    return 0;
}



