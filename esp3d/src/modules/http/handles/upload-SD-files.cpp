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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
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
#ifdef ESP_BENCHMARK_FEATURE
#include "../../../core/benchmark.h"
#endif //ESP_BENCHMARK_FEATURE
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../../serial2socket/serial2socket.h"
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../../websocket/websocket_server.h"

//SD files uploader handle
void HTTP_Server::SDFileupload ()
{
#ifdef ESP_BENCHMARK_FEATURE
    static uint64_t bench_start;
    static size_t bench_transfered;
#endif//ESP_BENCHMARK_FEATURE
    static uint64_t last_WS_update;
    //get authentication status
    level_authenticate_type auth_level= AuthenticationService::authenticated_level();
    static String filename;
    static ESP_SDFile fsUploadFile;
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
        _upload_status=UPLOAD_STATUS_FAILED;
    } else {
        HTTPUpload& upload = _webserver->upload();
        String upload_filename = upload.filename;
        if ((_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            Serial2Socket.pause(true);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            //Upload start
            if (upload.status == UPLOAD_FILE_START) {
                last_WS_update = millis();
#ifdef ESP_BENCHMARK_FEATURE
                bench_start = millis();
                bench_transfered = 0;
#endif//ESP_BENCHMARK_FEATURE
                _upload_status = UPLOAD_STATUS_ONGOING;
                if (!ESP_SD::accessFS()) {
                    _upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_NO_SD, "Upload rejected");
                    return;
                }
                if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT)  {
                    log_esp3d("Release Sd called");
                    ESP_SD::releaseFS();
                    _upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_NO_SD, "Upload rejected");
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                    Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                    return;
                }
                ESP_SD::setState(ESP_SDCARD_BUSY );

                if (upload_filename[0] != '/') {
                    filename = "/" + upload_filename;
                } else {
                    filename = upload.filename;
                }
                if (_webserver->hasArg ("rpath") ) {
                    upload_filename = _webserver->arg ("rpath") + filename;
                    if (upload_filename[0] != '/') {
                        filename = "/" + upload_filename;
                    } else {
                        filename = upload_filename;
                    }
                }
                //Sanity check
                if (ESP_SD::exists (filename.c_str()) ) {
                    ESP_SD::remove (filename.c_str());
                }
                String path = _webserver->arg ("path");
                if (path[0] != '/') {
                    path = "/" + path;
                }
                if (path[path.length()-1] != '/') {
                    path = path + "/";
                }
                if (_webserver->hasArg("createPath") && path.length() > 1) {
                    if (_webserver->arg("createPath")== "true")  {
                        int pos = path.indexOf('/',1);
                        while (pos != -1) {
                            String currentPath = path.substring(0, pos);
                            if (!ESP_SD::exists (currentPath.c_str()) ) {
                                if ( !ESP_SD::mkdir (currentPath.c_str()) ) {
                                    pushError(ESP_ERROR_FILE_CREATION, "Failed to create path", 500);
                                    _upload_status=UPLOAD_STATUS_FAILED;
                                    break;
                                }
                            }
                            if ((size_t)(pos+1) >= path.length()-1) {
                                pos=-1;
                                break;
                            } else {
                                pos = path.indexOf('/',pos+1);
                            }

                        }
                    }
                }
                if (fsUploadFile.isOpen() ) {
                    fsUploadFile.close();
                }
                String  sizeargname  = upload.filename + "S";
                log_esp3d("Uploading file %s", filename.c_str());
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
                    log_esp3d("Try file creation");
                    //create file
                    fsUploadFile = ESP_SD::open(filename.c_str(), ESP_FILE_WRITE);
                    //check If creation succeed
                    if (fsUploadFile) {
                        //if yes upload is started
                        _upload_status= UPLOAD_STATUS_ONGOING;
                        log_esp3d("Try file creation");
                    } else {
                        //if no set cancel flag
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_CREATION, "File creation failed");
                        log_esp3d("File creation failed");
                    }

                }
                //Upload write
            } else if(upload.status == UPLOAD_FILE_WRITE) {
                //check if file is available and no error
                if(fsUploadFile && _upload_status == UPLOAD_STATUS_ONGOING) {
#ifdef ESP_BENCHMARK_FEATURE
                    bench_transfered += upload.currentSize;
#endif//ESP_BENCHMARK_FEATURE
                    //update websocket every 2000 ms
                    if (millis()-last_WS_update > 2000) {
                        websocket_terminal_server.handle();
                        last_WS_update = millis();
                    }
                    //no error so write post date
                    int writeddatanb=fsUploadFile.write(upload.buf, upload.currentSize);
                    if(upload.currentSize != (size_t)writeddatanb) {
                        //we have a problem set flag UPLOAD_STATUS_FAILED
                        log_esp3d("File write failed du to mismatch size %d vs %d", writeddatanb, upload.currentSize);
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    _upload_status=UPLOAD_STATUS_FAILED;
                    log_esp3d("Error detected");
                    pushError(ESP_ERROR_FILE_WRITE, "File write failed");
                }
                //Upload end
            } else if(upload.status == UPLOAD_FILE_END) {
                uint32_t filesize = 0;
                //check if file is still open
                if(fsUploadFile) {
                    //close it
                    fsUploadFile.close();
#ifdef ESP_BENCHMARK_FEATURE
                    benchMark("SD upload", bench_start, millis(), bench_transfered);
#endif//ESP_BENCHMARK_FEATURE
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
                log_esp3d("Release Sd called");
                ESP_SD::releaseFS();
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                //Upload cancelled
            } else {
                if (_upload_status == UPLOAD_STATUS_ONGOING) {
                    _upload_status = UPLOAD_STATUS_FAILED;
                    log_esp3d("Release Sd called");
                    ESP_SD::releaseFS();
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                    Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
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
        log_esp3d("Release Sd called");
        ESP_SD::releaseFS();
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    }
    Hal::wait(5);
}
#endif //HTTP_FEATURE && SD_DEVICE
