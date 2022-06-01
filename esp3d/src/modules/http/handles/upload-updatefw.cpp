/*
 upload-updatefw.cpp - ESP3D http handle

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
#define UPDATE_SIZE
#include <Update.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define UPDATE_SIZE ESP_FileSystem::max_update_size()
#endif //ARDUINO_ARCH_ESP8266
#include "../../filesystem/esp_filesystem.h"
#include "../../authentication/authentication_service.h"
#include "../../../core/esp3doutput.h"
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../../serial2socket/serial2socket.h"
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
//File upload for Web update
void HTTP_Server::WebUpdateUpload ()
{
    static size_t last_upload_update;
    static uint32_t downloadsize = 0;
    ESP3DOutput  output(ESP_REMOTE_SCREEN_CLIENT);
    //only admin can update FW
    if (AuthenticationService::authenticated_level() != LEVEL_ADMIN) {
        _upload_status = UPLOAD_STATUS_FAILED;
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
        output.printERROR("Update rejected!",401);
    } else {
        //get current file ID
        HTTPUpload& upload = _webserver->upload();
        if ((_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            Serial2Socket.pause();
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            //Upload start
            if(upload.status == UPLOAD_FILE_START) {
                output.printMSG("Update Firmware");
                _upload_status= UPLOAD_STATUS_ONGOING;
                String  sizeargname  = upload.filename + "S";
                if (_webserver->hasArg (sizeargname.c_str()) ) {
                    downloadsize = _webserver->arg (sizeargname).toInt();
                } else {
                    downloadsize = 0;
                }
                if (downloadsize > ESP_FileSystem::max_update_size()) {
                    _upload_status=UPLOAD_STATUS_FAILED;
                    output.printERROR("Update rejected!",500);
                    pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                }
                last_upload_update = 0;
                if (_upload_status != UPLOAD_STATUS_FAILED) {
                    if(!Update.begin(UPDATE_SIZE)) { //start with unknown = max available size
                        _upload_status=UPLOAD_STATUS_FAILED;
                        output.printERROR("Update rejected!",500);
                        pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                    } else {
                        output.printMSG("Update 0%");
                    }
                }
                //Upload write
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if no error
                if (_upload_status == UPLOAD_STATUS_ONGOING) {
                    //we do not know the total file size yet but we know the available space so let's use it
                    if (downloadsize != 0) {
                        if ( ((100 * upload.totalSize) / downloadsize) !=last_upload_update) {
                            if ( downloadsize > 0) {
                                last_upload_update = (100 * upload.totalSize) / downloadsize;
                            } else {
                                last_upload_update = upload.totalSize;
                            }
                            String s = "Update ";
                            s+= String(last_upload_update);
                            s+="/100";
                            output.printMSG(s.c_str());
                        }
                    }
                    if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                        _upload_status=UPLOAD_STATUS_FAILED;
                        output.printERROR("Update write failed!",500);
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                    }
                }
                //Upload end

            } else if(upload.status == UPLOAD_FILE_END) {
                if ((downloadsize!=0) && (downloadsize < upload.totalSize)) {
                    _upload_status=UPLOAD_STATUS_FAILED;
                    output.printERROR("Update write failed!",500);
                    pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                }
                if (_upload_status == UPLOAD_STATUS_ONGOING) {
                    if(Update.end(true)) { //true to set the size to the current progress
                        //Now Reboot
                        output.printMSG("Update 100%");
                        _upload_status=UPLOAD_STATUS_SUCCESSFUL;
                    } else {
                        output.printERROR("Update failed!",500);
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_UPDATE, "Update FW failed");
                    }
                } else {
                    _upload_status=UPLOAD_STATUS_FAILED;
                    output.printERROR("Update failed!", 500);
                    pushError(ESP_ERROR_UPDATE, "Update FW failed");
                }
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            } else {
                output.printERROR("Update failed!",500);
                _upload_status=UPLOAD_STATUS_FAILED;
            }
        }
    }
    if(_upload_status == UPLOAD_STATUS_FAILED) {
        cancelUpload();
        Update.end();
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    }
}
#endif //HTTP_FEATURE && WEB_UPDATE_FEATURE
