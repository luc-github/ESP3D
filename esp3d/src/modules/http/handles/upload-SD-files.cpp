/*
 upload-SD-files.cpp - ESP3D http handle

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
#if defined (HTTP_FEATURE) && defined(SD_DEVICE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../filesystem/esp_sd.h"
#include "../../authentication/authentication_service.h"
//SD files uploader handle
void HTTP_Server::SDFileupload ()
{
    //get authentication status
    level_authenticate_type auth_level= AuthenticationService::authenticated_level();
    static String filename;
    static ESP_SDFile fsUploadFile;
    static uint32_t timecheck;
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
        _upload_status=UPLOAD_STATUS_FAILED;
    } else {
        HTTPUpload& upload = _webserver->upload();
        String upload_filename = upload.filename;
        if ((_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {

            //Upload start
            if (upload.status == UPLOAD_FILE_START) {
                timecheck= millis();
                _upload_status = UPLOAD_STATUS_ONGOING;
                if (upload_filename[0] != '/') {
                    filename = "/" + upload_filename;
                } else {
                    filename = upload.filename;
                }
                //Sanity check
                if (ESP_SD::exists (filename.c_str()) ) {
                    ESP_SD::remove (filename.c_str());
                }
                if (fsUploadFile.isOpen() ) {
                    fsUploadFile.close();
                }
                String  sizeargname  = upload.filename + "S";
                //TODO add busy state and handle it for upload
                if (ESP_SD::getState(true) != ESP_SDCARD_IDLE) {
                    _upload_status=UPLOAD_STATUS_FAILED;
                }
                if (_upload_status!=UPLOAD_STATUS_FAILED) {
                    if (_webserver->hasArg (sizeargname.c_str()) ) {
                        size_t freespace = ESP_SD::totalBytes() - ESP_SD::usedBytes();
                        size_t filesize = _webserver->arg (sizeargname.c_str()).toInt();
                        if (freespace < filesize ) {
                            _upload_status=UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                        }
                    }
                }
                if (_upload_status!=UPLOAD_STATUS_FAILED) {
                    //create file
                    fsUploadFile = ESP_SD::open(filename.c_str(), ESP_FILE_WRITE);
                    //check If creation succeed
                    if (fsUploadFile) {
                        //if yes upload is started
                        _upload_status= UPLOAD_STATUS_ONGOING;
                    } else {
                        //if no set cancel flag
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_CREATION, "File creation failed");
                    }

                }
                //Upload write
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if file is available and no error
                if(fsUploadFile && _upload_status == UPLOAD_STATUS_ONGOING) {
                    //no error so write post date
                    if(upload.currentSize != fsUploadFile.write(upload.buf, upload.currentSize)) {
                        //we have a problem set flag UPLOAD_STATUS_FAILED
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    _upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                }
                //Upload end
            } else if(upload.status == UPLOAD_FILE_END) {
                uint32_t filesize = 0;
                //check if file is still open
                if(fsUploadFile) {
                    //close it
                    fsUploadFile.close();
                    //check size
                    String  sizeargname  = upload.filename + "S";
                    //fsUploadFile = ESP_SD::open (filename, ESP_FILE_READ);
                    filesize = fsUploadFile.size();
                    _upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    if (_webserver->hasArg (sizeargname.c_str()) ) {
                        if (_webserver->arg (sizeargname.c_str()) != String(filesize)) {
                            _upload_status = UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_SIZE, "File upload failed");
                        }
                    }
                    if (_upload_status == UPLOAD_STATUS_ONGOING) {
                        _upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    _upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_FILE_CLOSE, "File close failed");
                }
                Serial.print(filesize);
                Serial.print(" B in  ");
                Serial.print((millis()-timecheck) / 1000);
                Serial.println(" sec");
                //Upload cancelled
            } else {
                if (_upload_status == UPLOAD_STATUS_ONGOING) {
                    _upload_status = UPLOAD_STATUS_FAILED;
                }
            }
        }
    }

    if(_upload_status == UPLOAD_STATUS_FAILED) {
        cancelUpload();
        if(fsUploadFile) {
            fsUploadFile.close();
        }
        if (auth_level != LEVEL_GUEST) {
            if (ESP_SD::exists (filename.c_str())) {
                ESP_SD::remove (filename.c_str());
            }
        }
    }
}
#endif //HTTP_FEATURE && SD_DEVICE
