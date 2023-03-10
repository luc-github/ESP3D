/*
 handle-config.cpp - ESP3D http handle

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
#if defined (HTTP_FEATURE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../authentication/authentication_service.h"
#include "../../../core/commands.h"
#include "../../../core/esp3doutput.h"

//Handle web command query [ESP420]plain and send answer//////////////////////////////
void HTTP_Server::handle_config ()
{
    level_authenticate_type auth_level = AuthenticationService::authenticated_level();
    String cmd = "[ESP420]";
    if (_webserver->hasArg("json")) {
        cmd = "[ESP420]json="+_webserver->arg("json");
    }
    ESP3DOutput  output(_webserver);
    output.printMSGLine("<pre>");
    esp3d_commands.process((uint8_t*)cmd.c_str(), cmd.length(), &output, auth_level);
    output.printMSGLine("</pre>");
    return;
}
#endif //HTTP_FEATURE
