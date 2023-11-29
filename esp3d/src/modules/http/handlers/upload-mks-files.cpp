/*
 upload-mks-files.cpp - ESP3D http handle

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
#if defined(HTTP_FEATURE) && (COMMUNICATION_PROTOCOL == MKS_SERIAL)
#include "../http_server.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
#include "../../authentication/authentication_service.h"
#include "../../mks/mks_service.h"

// MKS files uploader handle
void HTTP_Server::MKSFileupload() {
  static uint32_t fragmentID = 0;
  static uint8_t buf2send[MKS_FRAME_DATA_MAX_SIZE];
  static size_t buf2sendlen = 0;
  // get authentication status
  ESP3DAuthenticationLevel auth_level =
      AuthenticationService::getAuthenticatedLevel();
  // Guest cannot upload - only admin
  if (auth_level == ESP3DAuthenticationLevel::guest) {
    pushError(ESP_ERROR_AUTHENTICATION, "Upload rejected", 401);
    _upload_status = UPLOAD_STATUS_FAILED;
  } else {
    HTTPUpload& upload = _webserver->upload();
    if (upload.status == UPLOAD_FILE_START) {
      buf2sendlen = 0;
      esp3d_log("Starting upload");
      _upload_status = UPLOAD_STATUS_ONGOING;
      size_t fileSize = 0;
      String filename = upload.filename;
      String sfilename = "s" + filename;
      esp3d_log("Filename: %s", filename.c_str());
      // No / in filename
      if (filename[0] == '/') {
        filename.remove(0, 1);
      }
      // for remote path or device
      // if USB on TFT 0:<path>
      // if SD on TFT  1:<path>
      // if SD on Robin 0:<path> or just <path>

      if (_webserver->hasArg("rpath")) {
        String upload_filename = _webserver->arg("rpath") + "/" + filename;
        filename = upload_filename;
        if (filename[0] == '/') {
          filename.remove(0, 1);
        }
        // this is target device
        if (filename.startsWith("USB:") || filename.startsWith("SD:")) {
          String cmd = "M998 ";
          if (filename.startsWith("USB:")) {
            cmd += "0";
            filename.remove(0, strlen("USB:"));
          } else {
            cmd += "1";
            filename.remove(0, strlen("SD:"));
          }
          MKSService::sendGcodeFrame(cmd.c_str());
          ESP3DHal::wait(10);
        }
      }
      if (_webserver->hasArg(sfilename)) {
        fileSize = _webserver->arg(sfilename).toInt();
      } else if (_webserver->hasHeader("Content-Length")) {
        fileSize = _webserver->header("Content-Length").toInt();
      }
      fragmentID = 0;
      esp3d_log("Filename: %s Size:%d", filename.c_str(), fileSize);
      if (MKSService::sendFirstFragment(filename.c_str(), fileSize)) {
        MKSService::uploadMode();
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (_upload_status == UPLOAD_STATUS_ONGOING) {
        uint currentsize = upload.currentSize;
        uint8_t* currentBuffer = upload.buf;
        while ((currentsize > 0) && (_upload_status == UPLOAD_STATUS_ONGOING)) {
          while ((buf2sendlen < MKS_FRAME_DATA_MAX_SIZE) && currentsize > 0) {
            buf2send[buf2sendlen] = currentBuffer[0];
            buf2sendlen++;
            currentsize--;
            currentBuffer++;
          }
          if (buf2sendlen == MKS_FRAME_DATA_MAX_SIZE) {
            esp3d_log("Send %d chars in Fragment %d", buf2sendlen, fragmentID);
            if (MKSService::sendFragment(buf2send, buf2sendlen, fragmentID)) {
              buf2sendlen = 0;
              fragmentID++;
            } else {
              _upload_status = UPLOAD_STATUS_FAILED;
              pushError(ESP_ERROR_FILE_WRITE, "File write failed");
            }
          }
          ESP3DHal::wait(0);
        }
      }

    } else if (upload.status == UPLOAD_FILE_END) {
      if (_upload_status == UPLOAD_STATUS_ONGOING) {
        esp3d_log("Upload end");
        fragmentID = MKSService::getFragmentID(fragmentID, true);
        esp3d_log("Send %d chars in Fragment %d", buf2sendlen, fragmentID);
        if (MKSService::sendFragment(buf2send, buf2sendlen, fragmentID)) {
          _upload_status = UPLOAD_STATUS_SUCCESSFUL;
        } else {
          _upload_status = UPLOAD_STATUS_FAILED;
          pushError(ESP_ERROR_FILE_CLOSE, "File close failed");
        }
        MKSService::commandMode();
      }
    } else {
      // error
      _upload_status = UPLOAD_STATUS_FAILED;
      pushError(ESP_ERROR_FILE_WRITE, "File write failed");
    }
  }
  if (_upload_status == UPLOAD_STATUS_FAILED) {
    cancelUpload();
    // TBC need to do that
    // MKSService::sendFragment(nullptr,0,MKSService::getFragmentID(fragmentID,
    // true));
    MKSService::commandMode();
  }
}
#endif  // HTTP_FEATURE && (COMMUNICATION_PROTOCOL == MKS_SERIAL)
