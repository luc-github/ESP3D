/*
 upload-files.cpp - ESP3D http handle

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
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../../serial2socket/serial2socket.h"
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL



#ifdef ESP_BENCHMARK_FEATURE
#include "../../../core/benchmark.h"
#endif //ESP_BENCHMARK_FEATURE

//FS files uploader handle
void HTTP_Server::FSFileupload ()
{
#ifdef ESP_BENCHMARK_FEATURE
    static uint64_t bench_start;
    static size_t bench_transfered;
#endif//ESP_BENCHMARK_FEATURE
    //get authentication status
    level_authenticate_type auth_level= AuthenticationService::authenticated_level();
    static String filename;
    static ESP_File fsUploadFile;
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
        _upload_status=UPLOAD_STATUS_FAILED;
    } else {
        HTTPUpload& upload = _webserver->upload();
        String upload_filename = upload.filename;
        if ((_upload_status != UPLOAD_STATUS_FAILED) || (upload.status == UPLOAD_FILE_START)) {
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            Serial2Socket.pause();
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            //Upload start
            if (upload.status == UPLOAD_FILE_START) {
#ifdef ESP_BENCHMARK_FEATURE
                bench_start = millis();
                bench_transfered = 0;
#endif//ESP_BENCHMARK_FEATURE
                _upload_status = UPLOAD_STATUS_ONGOING;
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
                if (ESP_FileSystem::exists (filename.c_str()) ) {
                    ESP_FileSystem::remove (filename.c_str());
                }
                if (fsUploadFile.isOpen() ) {
                    fsUploadFile.close();
                }
                String  sizeargname  = upload.filename + "S";
                if (_webserver->hasArg (sizeargname.c_str()) ) {
                    size_t freespace = ESP_FileSystem::totalBytes() - ESP_FileSystem::usedBytes();
                    size_t filesize = _webserver->arg (sizeargname.c_str()).toInt();
                    if (freespace < filesize ) {
                        _upload_status=UPLOAD_STATUS_FAILED;
                        pushError(ESP_ERROR_NOT_ENOUGH_SPACE, "Upload rejected");
                    }
                }
                if (_upload_status!=UPLOAD_STATUS_FAILED) {
                    //create file
                    fsUploadFile = ESP_FileSystem::open(filename.c_str(), ESP_FILE_WRITE);
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
#ifdef ESP_BENCHMARK_FEATURE
                    bench_transfered += upload.currentSize;
#endif//ESP_BENCHMARK_FEATURE
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
                log_esp3d("upload end");
                //check if file is still open
                if(fsUploadFile) {
                    //close it
                    fsUploadFile.close();
#ifdef ESP_BENCHMARK_FEATURE
                    benchMark("FS upload", bench_start, millis(), bench_transfered);
#endif//ESP_BENCHMARK_FEATURE
                    //check size
                    String  sizeargname  = upload.filename + "S";
                    //fsUploadFile = ESP_FileSystem::open (filename, ESP_FILE_READ);
                    uint32_t filesize = fsUploadFile.size();
                    _upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    if (_webserver->hasArg (sizeargname.c_str()) ) {
                        log_esp3d("Size check: %s vs %s", _webserver->arg (sizeargname.c_str()).c_str(), String(filesize).c_str());
                        if (_webserver->arg (sizeargname.c_str()) != String(filesize)) {
                            log_esp3d("Size Error");
                            _upload_status = UPLOAD_STATUS_FAILED;
                            pushError(ESP_ERROR_SIZE, "File upload failed");
                        }
                    }
                    if (_upload_status == UPLOAD_STATUS_ONGOING) {
                        _upload_status = UPLOAD_STATUS_SUCCESSFUL;
                    }
                } else {
                    //we have a problem set flag UPLOAD_STATUS_FAILED
                    log_esp3d("Close Error");
                    _upload_status=UPLOAD_STATUS_FAILED;
                    pushError(ESP_ERROR_FILE_CLOSE, "File close failed");
                }
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
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
            if (ESP_FileSystem::exists (filename.c_str())) {
                ESP_FileSystem::remove (filename.c_str());
            }
        }
#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        Serial2Socket.pause(false);
#endif // ESP3DLIB_ENV && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    }
}
#endif //HTTP_FEATURE && FILESYSTEM_FEATURE
