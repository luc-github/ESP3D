/*
 upload-files.cpp - ESP3D http handle

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../../../include/esp3d_config.h"
#if defined (HTTP_FEATURE) && defined(FILESYSTEM_FEATURE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../filesystem/esp_filesystem.h"
#include "../../authentication/authentication_service.h"
//FS files uploader handle
void HTTP_Server::FSFileupload ()
{
    //get authentication status
    level_authenticate_type auth_level= AuthenticationService::authenticated_level();
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        _webserver->send (401, "text/plain", "Wrong authentication!");
        return;
    }
    static String filename;
    static ESP_File fsUploadFile;

    HTTPUpload& upload = _webserver->upload();
    //Upload start
    if(upload.status == UPLOAD_FILE_START) {
        String upload_filename = upload.filename;
        if (upload_filename[0] != '/') {
            filename = "/" + upload_filename;
        } else {
            filename = upload.filename;
        }

        if (ESP_FileSystem::exists (filename.c_str()) ) {
            ESP_FileSystem::remove (filename.c_str());
        }
        if (fsUploadFile.isOpen() ) {
            fsUploadFile.close();
        }
        //create file
        fsUploadFile = ESP_FileSystem::open(filename.c_str(), ESP_FILE_WRITE);
        //check If creation succeed
        if (fsUploadFile) {
            //if yes upload is started
            _upload_status= UPLOAD_STATUS_ONGOING;
        } else {
            //if no set cancel flag
            _upload_status=UPLOAD_STATUS_CANCELLED;
            _webserver->send (500, "text/plain", "Cannot create file!");
            _webserver->client().stop();
        }
        //Upload write
    } else if(upload.status == UPLOAD_FILE_WRITE) {
//check if file is available and no error
        if(fsUploadFile && _upload_status == UPLOAD_STATUS_ONGOING) {
            //no error so write post date
            fsUploadFile.write(upload.buf, upload.currentSize);
        } else {
            //we have a problem set flag UPLOAD_STATUS_CANCELLED
            _upload_status=UPLOAD_STATUS_CANCELLED;
            fsUploadFile.close();
            if (ESP_FileSystem::exists (filename.c_str())) {
                ESP_FileSystem::remove (filename.c_str());
            }
            _webserver->send (500, "text/plain", "Cannot write file!");
            _webserver->client().stop();
        }
//Upload end
    } else if(upload.status == UPLOAD_FILE_END) {
        //check if file is still open
        if(fsUploadFile) {
            //close it
            fsUploadFile.close();
            //check size
            String  sizeargname  = upload.filename + "S";
            //fsUploadFile = ESP_FileSystem::open (filename, ESP_FILE_READ);
            uint32_t filesize = fsUploadFile.size();
            //fsUploadFile.close();
            if (_webserver->hasArg (sizeargname.c_str()) ) {
                if (_webserver->arg (sizeargname.c_str()) != String(filesize)) {
                    _upload_status = UPLOAD_STATUS_FAILED;
                    ESP_FileSystem::remove (filename.c_str());
                }
            }
            if (_upload_status == UPLOAD_STATUS_ONGOING) {
                _upload_status = UPLOAD_STATUS_SUCCESSFUL;
            } else {
                _webserver->send (500, "text/plain", "Upload error!");
            }
        } else {
            //we have a problem set flag UPLOAD_STATUS_CANCELLED
            _upload_status=UPLOAD_STATUS_CANCELLED;
            _webserver->client().stop();
            if (ESP_FileSystem::exists (filename.c_str()) ) {
                ESP_FileSystem::remove (filename.c_str());
            }
            _webserver->send (500, "text/plain", "Upload error!");
        }
        //Upload cancelled
    } else {
        if (_upload_status == UPLOAD_STATUS_ONGOING) {
            _upload_status = UPLOAD_STATUS_CANCELLED;
        }
        if(fsUploadFile) {
            fsUploadFile.close();
        }
        if (ESP_FileSystem::exists (filename.c_str()) ) {
            ESP_FileSystem::remove (filename.c_str());
        }
        _webserver->send (500, "text/plain", "Upload error!");
    }
}
#endif //HTTP_FEATURE && FILESYSTEM_FEATURE
