/*
 handle-command.cpp - ESP3D http handle

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

//Handle web command query and send answer//////////////////////////////
void HTTP_Server::handle_web_command ()
{
    level_authenticate_type auth_level = AuthenticationService::authenticated_level();
    //log_esp3d("Authentication = %d", auth_level);
    String cmd = "";
    if (_webserver->hasArg ("cmd")) {
        cmd = _webserver->arg ("cmd");
        ESP3DOutput  output(_webserver);
        if(!cmd.endsWith("\n")) {
            cmd+="\n";    //need to validate command
        }
        esp3d_commands.process((uint8_t*)cmd.c_str(), cmd.length(), &output, auth_level);
    } else {
        _webserver->send (400, "text/plain", "Invalid command");
    }
    return;
}
#endif //HTTP_FEATURE
