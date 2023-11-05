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
  log_esp3d_d("Processing COPY");
  int code = 201;
  size_t sp = clearPayload();
  log_esp3d("Payload size: %d", sp);
  uint8_t fsTypeOrigin = WebDavFS::getFSType(url);
  log_esp3d("FS type of %s : %d", url, fsTypeOrigin);
  String destination = "";
  if (hasHeader("Destination")) {
    destination = getHeader("Destination");
    log_esp3d("Destination: %s", destination.c_str());
    uint8_t fsTypeDestination = WebDavFS::getFSType(destination.c_str());
    log_esp3d("FS type of %s : %d", destination.c_str(), fsTypeDestination);
    if (fsTypeDestination != fsTypeOrigin) {
      code = 409;
      log_esp3d_e("Destination and origin must be on same FS");
    } else {
      bool overwrite = false;
      if (hasHeader("Overwrite")) {
        overwrite = (strcmp(getHeader("Overwrite"), "T") == 0);
        log_esp3d("Overwrite: %s", overwrite ? "true" : "false");
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
                log_esp3d_e("File %s is a directory", url);
              } else {
                // check if destination exists
                WebDavFile fileDestination =
                    WebDavFS::open(destination.c_str());
                if (fileDestination) {
                  if (fileDestination.isDirectory()) {
                    code = 412;
                    log_esp3d_e("File %s is a directory", destination.c_str());
                  }
                  fileDestination.close();
                  overwrite = false;
                } else {
                  log_esp3d("Destination does not exist");
                  overwrite = true;
                }
                // check available space
                if (overwrite) {
                  if (WebDavFS::exists(destination.c_str())) {
                    if (WebDavFS::freeBytes(fsTypeOrigin) + fileOrigin.size() <
                        fileDestination.size()) {
                      code = 507;
                      log_esp3d_e("Not enough space");
                      overwrite = false;
                    } else {  // there is enough space to overwrite
                      code = 204;
                    }
                  } else {
                    if (WebDavFS::freeBytes(fsTypeOrigin) < fileOrigin.size()) {
                      code = 507;
                      log_esp3d_e("Not enough space");
                      overwrite = false;
                    }
                  }

                  if (overwrite) {
                    log_esp3d("Overwrite allowed");
                    if (WebDavFS::exists(destination.c_str()) &&
                        !WebDavFS::remove(destination.c_str())) {
                      code = 500;
                      log_esp3d_e("Failed to remove %s", destination.c_str());
                      overwrite = false;
                    } else {
                      // copy file
                      WebDavFile fileDestination =
                          WebDavFS::open(destination.c_str(), ESP_FILE_WRITE);
                      if (fileDestination) {
                        size_t data_read = 0;
                        size_t data_to_read = fileOrigin.size();
                        while (data_read < data_to_read) {
                          Hal::wait(0);
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
                              log_esp3d_e("Failed to write data");
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
                          log_esp3d_e("Failed to copy %s to %s", url,
                                      destination.c_str());
                          overwrite = false;
                        }
                      } else {
                        code = 500;
                        log_esp3d_e("Failed to open %s", destination.c_str());
                        overwrite = false;
                      }
                    }
                  }
                }
                if (overwrite) {
                  if (!WebDavFS::rename(url, destination.c_str())) {
                    code = 500;
                    log_esp3d_e("Failed to move %s to %s", url,
                                destination.c_str());
                  }
                }
              }
              fileOrigin.close();
            } else {
              code = 500;
              log_esp3d_e("Failed to open %s", url);
            }
          } else {
            code = 404;
            log_esp3d_e("File not found");
          }
          WebDavFS::releaseFS(fsTypeOrigin);
        } else {
          code = 503;
          log_esp3d_e("FS not available");
        }
      } else {
        code = 400;
        log_esp3d_e("Root cannot be deleted");
      }
    }
  } else {
    code = 400;
    log_esp3d_e("Destination not set");
  }

  log_esp3d_e("Sending response code %d", code);
  send_response_code(code);
  send_webdav_headers();
}

#endif  // WEBDAV_FEATURE
