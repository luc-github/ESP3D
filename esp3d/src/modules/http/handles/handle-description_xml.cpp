/*
 handle-SSDP.cpp - ESP3D http handle

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
#include "../../../include/esp3d_config.h"
#if defined (HTTP_FEATURE) && defined (SSDP_FEATURE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#ifdef SSDP_FEATURE
#include <ESP32SSDP.h>
#endif //SSDP_FEATURE
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif //SSDP_FEATURE
#endif //ARDUINO_ARCH_ESP8266
void HTTP_Server::handle_SSDP()
{
    if (_webserver) {
#if defined (ARDUINO_ARCH_ESP32)
        _webserver->send(200, "text/xml", SSDP.getSchema());
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
        SSDP.schema(_webserver->client());
#endif //ARDUINO_ARCH_ESP8266
    }
}
#endif //HTTP_FEATURE && SSDP_FEATURE
