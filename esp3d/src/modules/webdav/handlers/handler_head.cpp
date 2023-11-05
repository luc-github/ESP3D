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

void WebdavServer::handler_head(const char* url) {
  log_esp3d("Processing HEAD");
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
