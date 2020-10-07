/*
 handle-filenotfound.cpp - ESP3D http handle

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
#include "../../filesystem/esp_filesystem.h"
#include "../../authentication/authentication_service.h"
#if defined(SD_DEVICE)
#include "../../filesystem/esp_sd.h"
#endif //SD_DEVICE

//Handle not registred path on FS neither SD ///////////////////////
void HTTP_Server:: handle_not_found()
{
    if (AuthenticationService::authenticated_level() == LEVEL_GUEST) {
        _webserver->send (401, "text/plain", "Wrong authentication!");
        return;
    }
    String path = _webserver->urlDecode(_webserver->uri());
    String contentType =  getContentType(path.c_str());
    String pathWithGz = path + ".gz";
#if defined (FILESYSTEM_FEATURE)
    if(ESP_FileSystem::exists(pathWithGz.c_str()) || ESP_FileSystem::exists(path.c_str())) {
        if(ESP_FileSystem::exists(pathWithGz.c_str())) {
            _webserver->sendHeader("Content-Encoding", "gzip");
            path = pathWithGz;
        }
        if(!StreamFSFile(path.c_str(),contentType.c_str())) {
            log_esp3d("Stream `%s` failed", path.c_str());
        }
        return;
    }
#endif //#if defined (FILESYSTEM_FEATURE)

#if defined (SD_DEVICE)
    if (path.startsWith("/sd/")) {
        path = path.substring(3);
        pathWithGz = path + ".gz";
        if(ESP_SD::exists(pathWithGz.c_str()) || ESP_SD::exists(path.c_str())) {
            if(ESP_SD::exists(pathWithGz.c_str())) {
                _webserver->sendHeader("Content-Encoding", "gzip");
                path = pathWithGz;
            }
            if(!StreamSDFile(path.c_str(),contentType.c_str())) {
                log_esp3d("Stream `%s` failed", path.c_str());
            }
            return;
        }
    }
#endif //#if defined (SD_DEVICE)

#ifdef FILESYSTEM_FEATURE
    //check local page
    path = "/404.htm";
    contentType =  getContentType(path.c_str());
    pathWithGz =  path + ".gz";
    if(ESP_FileSystem::exists(pathWithGz.c_str()) || ESP_FileSystem::exists(path.c_str())) {
        if(ESP_FileSystem::exists(pathWithGz.c_str())) {
            _webserver->sendHeader("Content-Encoding", "gzip");
            path = pathWithGz;
        }
        if(!StreamFSFile(path.c_str(),contentType.c_str())) {
            log_esp3d("Stream `%s` failed", path.c_str());
        }
        return;
    }
#endif //FILESYSTEM_FEATURE
    //let's keep simple just send minimum
    _webserver->send(404);

}
#endif //HTTP_FEATURE
