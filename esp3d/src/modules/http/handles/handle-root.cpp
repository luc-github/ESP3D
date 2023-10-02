/*
 handle-root.cpp - ESP3D http handle

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
//embedded response file if no files on ESP Filesystem
#include "../embedded.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../filesystem/esp_filesystem.h"
//Root of Webserver/////////////////////////////////////////////////////
void HTTP_Server::handle_root()
{
    String path = ESP3D_HOST_PATH;
    //Some sanity check
    if (path[0]!='/') {
        path ="/" + path;
    }
    if (path[path.length()-1]!='/') {
        path = path + "/";
    }
    path += "index.html";
    String contentType =  getContentType(path.c_str());
    String pathWithGz = path + ".gz";
    //if have a index.html or gzip version this is default root page
    if((ESP_FileSystem::exists(pathWithGz.c_str()) || ESP_FileSystem::exists(path.c_str())) && !_webserver->hasArg("forcefallback") && _webserver->arg("forcefallback")!="yes") {
        log_esp3d("Path found `%s`", path.c_str());
        if(ESP_FileSystem::exists(pathWithGz.c_str())) {
            _webserver->sendHeader("Content-Encoding", "gzip");
            path = pathWithGz;
            log_esp3d("Path is gz `%s`", path.c_str());
        }
        if(!StreamFSFile(path.c_str(),contentType.c_str())) {
            log_esp3d("Stream `%s` failed", path.c_str());
        }
        return;
    }
    //if no lets launch the default content
    _webserver->sendHeader("Content-Encoding", "gzip");
    _webserver->send_P(200,"text/html",(const char *)tool_html_gz,tool_html_gz_size);
}
#endif //HTTP_FEATURE
