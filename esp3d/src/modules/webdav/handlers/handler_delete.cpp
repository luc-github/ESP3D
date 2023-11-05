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
#include "../webdav_server.h"

void WebdavServer::handler_delete(const char* url) {
  log_esp3d_d("Processing DELETE");
  int code = 204;
  size_t sp = clearPayload();
  log_esp3d("Payload size: %d", sp);
  uint8_t fsType = WebDavFS::getFSType(url);
  log_esp3d("FS type of %s : %d", url, fsType);
  // url cannot be root
  if (!isRoot(url)) {
    if (WebDavFS::accessFS(fsType)) {
      // check if file exists
      if (WebDavFS::exists(url)) {
        // try remove as file
        if (!WebDavFS::remove(url)) {
          log_esp3d_d("Failed to remove file %s", url);
          log_esp3d_d("Trying to remove as directory");
          // if failed try remove as directory
          if (!WebDavFS::rmdir(url)) {
            log_esp3d_d("Failed to remove directory %s", url);
            code = 500;
            log_esp3d_e("Failed to remove %s", url);
          }
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
  if (code != 204) {
    log_esp3d_e("Sending response code %d", code);
    send_response_code(code);
    send_webdav_headers();
  }
}

#endif  // WEBDAV_FEATURE
