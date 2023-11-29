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

void WebdavServer::handler_move(const char* url) {
  esp3d_log("Processing MOVE");
  int code = 201;
  size_t sp = clearPayload();
  (void)sp;
  esp3d_log("Payload size: %d", sp);
  uint8_t fsTypeOrigin = WebDavFS::getFSType(url);
  esp3d_log("FS type of %s : %d", url, fsTypeOrigin);
  String destination = "";
  if (hasHeader("Destination")) {
    destination = getHeader("Destination");
    destination = urlDecode(destination.c_str());
    if (destination.startsWith("http://") ||
        destination.startsWith("https://")) {
      destination = destination.substring(destination.indexOf("/", 8));
      destination = destination.substring(destination.indexOf("/"));
      esp3d_log("Destination trimmed: %s", destination.c_str());
      if (destination != "/") {
        if (destination.endsWith("/")) {
          destination = destination.substring(0, destination.length() - 1);
        }
      }
    }
    esp3d_log("Destination: %s", destination.c_str());
    uint8_t fsTypeDestination = WebDavFS::getFSType(destination.c_str());
    esp3d_log("FS type of %s : %d", destination.c_str(), fsTypeDestination);
    if (fsTypeDestination != fsTypeOrigin) {
      code = 409;
      esp3d_log_e("Destination and origin must be on same FS");
    } else {
      bool overwrite = false;
      if (hasHeader("Overwrite")) {
        overwrite = (strcmp(getHeader("Overwrite"), "T") == 0);
        esp3d_log("Overwrite: %s", overwrite ? "true" : "false");
      }
      // url cannot be root
      if (!isRoot(url) && !isRoot(destination.c_str())) {
        String url_dir = url_dir + "/";
        if (WebDavFS::accessFS(fsTypeOrigin)) {
          // check if file exists
          esp3d_log("Checking if %s exists", url);
          if (WebDavFS::exists(url) || WebDavFS::exists(url_dir.c_str())) {
            // check if destination exists
            esp3d_log("Checking if %s exists", destination.c_str());
            if (WebDavFS::exists(destination.c_str())) {
              esp3d_log("Destination already exists");
              if (!overwrite) {
                code = 412;
                esp3d_log_e("Overwrite not allowed");
              } else {
                esp3d_log("Overwrite allowed");
                code = 204;
                if (!WebDavFS::remove(destination.c_str())) {
                  code = 500;
                  esp3d_log_e("Failed to remove %s", destination.c_str());
                  overwrite = false;
                }
              }
            } else {
              esp3d_log("Destination does not exist");
              overwrite = true;
            }
            if (overwrite) {
              if (!WebDavFS::rename(url, destination.c_str())) {
                code = 500;
                esp3d_log_e("Failed to move %s to %s", url,
                            destination.c_str());
              }
            }
          } else {
            code = 404;
            esp3d_log_e("File not found");
          }
          WebDavFS::releaseFS(fsTypeOrigin);
        } else {
          code = 503;
          esp3d_log_e("FS not available");
        }
      } else {
        code = 400;
        esp3d_log_e("Root cannot be deleted");
      }
    }
  } else {
    code = 400;
    esp3d_log_e("Destination not set");
  }
  esp3d_log_e("Sending response code %d", code);
  send_response_code(code);
  send_webdav_headers();
}

#endif  // WEBDAV_FEATURE
