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

void WebdavServer::handler_copy(const char* url) {
  esp3d_log("Processing COPY");
  int code = 201;
  size_t sp = clearPayload();
  (void)sp;
  esp3d_log("Payload size: %d", sp);
  uint8_t fsTypeOrigin = WebDavFS::getFSType(url);
  esp3d_log("FS type of %s : %d", url, fsTypeOrigin);
  String destination = "";
  if (hasHeader("Destination")) {
    destination = getHeader("Destination");
    esp3d_log("Destination: %s", destination.c_str());
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
        if (WebDavFS::accessFS(fsTypeOrigin)) {
          // check if file exists
          if (WebDavFS::exists(url)) {
            // Check if origin is a directory
            WebDavFile fileOrigin = WebDavFS::open(url);
            if (fileOrigin) {
              if (fileOrigin.isDirectory()) {
                code = 413;
                esp3d_log_e("File %s is a directory", url);
              } else {
                // check if destination exists
                WebDavFile fileDestination =
                    WebDavFS::open(destination.c_str());
                if (fileDestination) {
                  if (fileDestination.isDirectory()) {
                    code = 412;
                    esp3d_log_e("File %s is a directory", destination.c_str());
                  }
                  fileDestination.close();
                  overwrite = false;
                } else {
                  esp3d_log("Destination does not exist");
                  overwrite = true;
                }
                // check available space
#if WEBDAV_FEATURE == FS_ROOT
                uint64_t free_space = WebDavFS::freeBytes(fsTypeDestination);
#else
#if WEBDAV_FEATURE == FS_FLASH
                size_t free_space;
#endif
#if WEBDAV_FEATURE == FS_SD
                uint64_t free_space;
#endif
                free_space = WebDavFS::freeBytes();
#endif
                if (overwrite) {
                  if (WebDavFS::exists(destination.c_str())) {
                    if (free_space + fileOrigin.size() <
                        fileDestination.size()) {
                      code = 507;
                      esp3d_log_e("Not enough space");
                      overwrite = false;
                    } else {  // there is enough space to overwrite
                      code = 204;
                    }
                  } else {
                    if (free_space < fileOrigin.size()) {
                      code = 507;
                      esp3d_log_e("Not enough space");
                      overwrite = false;
                    }
                  }

                  if (overwrite) {
                    esp3d_log("Overwrite allowed");
                    if (WebDavFS::exists(destination.c_str()) &&
                        !WebDavFS::remove(destination.c_str())) {
                      code = 500;
                      esp3d_log_e("Failed to remove %s", destination.c_str());
                      overwrite = false;
                    } else {
                      // copy file
                      WebDavFile fileDestination =
                          WebDavFS::open(destination.c_str(), ESP_FILE_WRITE);
                      if (fileDestination) {
                        size_t data_read = 0;
                        size_t data_to_read = fileOrigin.size();
                        while (data_read < data_to_read) {
                          ESP3DHal::wait(0);
#if defined(ARDUINO_ARCH_ESP32)
                          uint8_t buff[2048];
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
                          uint8_t buff[512];
#endif  // ARDUINO_ARCH_ESP8266
                          size_t read_size =
                              fileOrigin.read(buff, sizeof(buff));
                          if (read_size > 0) {
                            // always check if data is sent as expected for each
                            // write
                            if (read_size !=
                                fileDestination.write(buff, read_size)) {
                              esp3d_log_e("Failed to write data");
                              code = 500;
                              break;
                            }
                            data_read += read_size;
                          } else {
                            // done reading
                            break;
                          }
                        }
                        fileDestination.close();
                        if (data_read != data_to_read) {
                          code = 500;
                          esp3d_log_e("Failed to copy %s to %s", url,
                                      destination.c_str());
                          overwrite = false;
                        }
                      } else {
                        code = 500;
                        esp3d_log_e("Failed to open %s", destination.c_str());
                        overwrite = false;
                      }
                    }
                  }
                }
                if (overwrite) {
                  if (!WebDavFS::rename(url, destination.c_str())) {
                    code = 500;
                    esp3d_log_e("Failed to move %s to %s", url,
                                destination.c_str());
                  }
                }
              }
              fileOrigin.close();
            } else {
              code = 500;
              esp3d_log_e("Failed to open %s", url);
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
