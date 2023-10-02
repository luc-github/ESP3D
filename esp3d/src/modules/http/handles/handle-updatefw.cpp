/*
 handle-updatefw.cpp - ESP3D http handle

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
#if defined (HTTP_FEATURE) && defined(WEB_UPDATE_FEATURE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../../core/esp3d.h"
#include "../../authentication/authentication_service.h"
//Web Update handler
void HTTP_Server::handleUpdate ()
{
    level_authenticate_type auth_level = AuthenticationService::authenticated_level();
    if (auth_level != LEVEL_ADMIN) {
        _upload_status = UPLOAD_STATUS_NONE;
        _webserver->send (401, "text/plain", "Wrong authentication!");
        return;
    }
    String jsonfile = "{\"status\":\"" ;
    switch(_upload_status) {
    case  UPLOAD_STATUS_NONE :
        jsonfile += "no file";
        break;
    case  UPLOAD_STATUS_CANCELLED :
        jsonfile += "canceled";
        break;
    case  UPLOAD_STATUS_SUCCESSFUL :
        jsonfile += "ok";
        break;
    default :
        jsonfile += "error";
        break;
    }
    jsonfile += "\"}";
    _webserver->sendHeader("Cache-Control", "no-cache");
    _webserver->send(200, "application/json", jsonfile);
    //if success restart
    if (_upload_status == UPLOAD_STATUS_SUCCESSFUL) {
        Hal::wait(1000);
        Esp3D::restart_esp();
    } else {
        _upload_status = UPLOAD_STATUS_NONE;
    }
}

#endif //HTTP_FEATURE && WEB_UPDATE_FEATURE
