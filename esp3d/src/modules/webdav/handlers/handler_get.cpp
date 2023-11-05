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
#include "../../websocket/websocket_server.h"
#if defined(HTTP_FEATURE)
#include "../webdav_server.h"
#endif  // HTTP_FEATURE

void WebdavServer::handler_get(const char* url) {
  log_esp3d_d("Processing GET");
  int code = 200;
  size_t sp = clearPayload();
  log_esp3d("Payload size: %d", sp);
  uint8_t fsType = WebDavFS::getFSType(url);
  log_esp3d("FS type of %s : %d", url, fsType);
  // url cannot be root
  if (!isRoot(url)) {
    if (WebDavFS::accessFS(fsType)) {
      // check if file exists
      if (WebDavFS::exists(url)) {
        WebDavFile file = WebDavFS::open(url);
        if (file) {
          // send response
          send_response_code(code);
          send_webdav_headers();
          send_header("Last-Modified", esp3d_string::getTimeString(
                                           (time_t)file.getLastWrite(), true));
          send_header("Content-Length", String(file.size()).c_str());
          send_header("Content-Type", esp3d_string::getContentType(url));
          // end the headers with a blank line
          _client.write("\r\n");
          // send file content
          size_t sent = 0;
          size_t toSend = file.size();
#if defined(ARDUINO_ARCH_ESP32)
          uint8_t buff[2048];
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
          uint8_t buff[1024];
#endif  // ARDUINO_ARCH_ESP8266
#if defined(HTTP_FEATURE)
          Esp3dTimout updateWS(2000);
#endif  // HTTP_FEATURE
          while (sent < toSend && _client.connected()) {
            Hal::wait(0);
            size_t read = file.read(buff, sizeof(buff));
            if (read > 0) {
              // always check if data is sent as expected for each write
              if (read != _client.write(buff, read)) {
                log_esp3d_e("Failed to send data");
                break;
              }
              sent += read;
            } else {
              // done reading
              break;
            }
#if defined(HTTP_FEATURE)
            // update websocket every 2000 ms
            if (updateWS.isTimeout()) {
              websocket_terminal_server.handle();
              updateWS.reset();
            }
#endif  // HTTP_FEATURE
          }
          // always check if data is sent as expected in total
          if (sent != toSend) {
            log_esp3d_e("Failed to send data, sent %d of %d", sent, toSend);
          }
          file.close();
        } else {
          code = 500;
          log_esp3d_e("Failed to open %s", url);
        }
      } else {
        code = 404;
        log_esp3d_e("File not found");
      }
      WebDavFS::releaseFS(fsType);
    } else {
      code = 503;
      log_esp3d_e("FS not available");
    }
  } else {
    code = 400;
    log_esp3d_e("Root cannot be deleted");
  }
  if (code != 200) {
    log_esp3d_e("Sending response code %d", code);
    send_response_code(code);
    send_webdav_headers();
  }
}
#endif  // WEBDAV_FEATURE
