/*
  webdav_server.cpp -  webdav server functions class

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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

#if defined(WEBDAV_FEATURE)
#include "../../../core/esp3d_string.h"
#include "../webdav_server.h"
#if defined(HTTP_FEATURE)
#include "../../websocket/websocket_server.h"
#endif  // HTTP_FEATURE

void WebdavServer::handler_put(const char* url) {
  esp3d_log("Processing PUT");
  int code = 201;
  size_t content_length = 0;
  uint8_t fsType = WebDavFS::getFSType(url);
  esp3d_log("FS type of %s : %d", url, fsType);
  if (hasHeader("Content-Length")) {
    content_length = atoi(getHeader("Content-Length"));
    esp3d_log("Content-Length: %d", content_length);
  } else {
    esp3d_log("Content-Length not set");
  }

  // url cannot be root
  if (!isRoot(url)) {
    if (WebDavFS::accessFS(fsType)) {
      bool hasError = false;
      // check if file exists
      if (WebDavFS::exists(url)) {
        esp3d_log("File %s already exists", url);
        code = 204;
        WebDavFile file = WebDavFS::open(url);
        if (file) {
          if (file.isDirectory()) {
            code = 412;
            esp3d_log_e("File %s is a directory", url);
            hasError = true;
          }
          file.close();
          if (!hasError) {
            // check size available
#if WEBDAV_FEATURE == FS_ROOT
            uint64_t free_space = WebDavFS::freeBytes(fsType);
#else
#if WEBDAV_FEATURE == FS_FLASH
            size_t free_space;
#endif
#if WEBDAV_FEATURE == FS_SD
            uint64_t free_space;
#endif
            free_space = WebDavFS::freeBytes();
#endif
            if (free_space + file.size() < content_length) {
              code = 507;
              esp3d_log_e("Not enough space");
              hasError = true;
            } else {  // there is enough space
              if (!WebDavFS::remove(
                      url)) {  // so delete existing file to replace it
                code = 500;
                esp3d_log_e("Failed to remove %s", url);
                hasError = true;
              }
            }
          }
        } else {
          hasError = true;
          code = 500;
          esp3d_log_e("Failed to open %s", url);
        }
      } else {
        // check size available
#if WEBDAV_FEATURE == FS_ROOT
        uint64_t free_space = WebDavFS::freeBytes(fsType);
#else
#if WEBDAV_FEATURE == FS_FLASH
        size_t free_space;
#endif
#if WEBDAV_FEATURE == FS_SD
        uint64_t free_space;
#endif
        free_space = WebDavFS::freeBytes();
#endif
        if (free_space < content_length) {
          code = 507;
          esp3d_log_e("Not enough space");
          hasError = true;
        }
      }
      if (!hasError) {  // read  payload and write it to file
        WebDavFile file = WebDavFS::open(url, ESP_FILE_WRITE);
        if (file) {
          size_t received = 0;
#if defined(ARDUINO_ARCH_ESP32)
          uint8_t chunk[2048];
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
          uint8_t chunk[512];
#endif  // ARDUINO_ARCH_ESP8266
#if defined(HTTP_FEATURE)
          Esp3dTimout updateWS(2000);
#endif  // HTTP_FEATURE
          while (_client.available() && received < content_length) {
            ESP3DHal::wait(0);
            size_t received_bytes = _client.read(chunk, sizeof(chunk));
            if (received_bytes != file.write(chunk, received_bytes)) {
              code = 500;
              esp3d_log_e("Failed to write %s", url);
              break;
            } else {
              received += received_bytes;
            }

#if defined(HTTP_FEATURE)
            // update websocket every 2000 ms
            if (updateWS.isTimeout()) {
              websocket_terminal_server.handle();
              updateWS.reset();
            }
#endif  // HTTP_FEATURE
          }
          if (received != content_length) {
            code = 500;
            esp3d_log_e("Failed to write %s", url);
          } else {
            esp3d_log("File %s written", url);
            send_response_code(code);  // 201 or 204
            send_webdav_headers();
            time_t now = time(nullptr);
            send_header("Last-Modified",
                        esp3d_string::getTimeString(now, true));
          }

          file.close();
        } else {
          code = 500;
          esp3d_log_e("Failed to open %s for writing", url);
        }
      }
      WebDavFS::releaseFS(fsType);
    } else {
      code = 503;
      esp3d_log_e("FS not available");
    }
  } else {
    code = 400;
    esp3d_log_e("Root cannot be deleted");
  }
  if (code != 201 && code != 204) {
    esp3d_log_e("Sending response code %d", code);
    send_response_code(code);
    send_webdav_headers();
  }
}

#endif  // WEBDAV_FEATURE
