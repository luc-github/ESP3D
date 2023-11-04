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

#define PROPFIND_RESPONSE_BODY_HEADER_1            \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
  "<D:multistatus xmlns:D=\"DAV:\""
#define PROPFIND_RESPONSE_BODY_HEADER_2 ">\r\n"

#define PROPFIND_RESPONSE_BODY_FOOTER "</D:multistatus>\r\n"

void WebdavServer::handler_propfind(const char* url) {
  log_esp3d("Processing PROPFIND");
  int code = 207;
  String depth = "0";
  String requestedDepth = "0";
  if (hasHeader("Depth")) {
    depth = getHeader("Depth");
    requestedDepth = depth;
    if (depth == "infinity") {
      depth = "1";
    }
  }
  clearPayload();
  uint8_t fsType = WebDavFS::getFSType(url);
  if (WebDavFS::accessFS(fsType)) {
    // TODO:
    WebDavFS::releaseFS(fsType);
  } else {
    code = 503;
    log_esp3d("FS not available");
  }
  if (code != 207) {
    send_response_code(code);
    send_webdav_headers();
  }
}

#endif  // WEBDAV_FEATURE
