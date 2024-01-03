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

#define LOCK_RESPONSE_BODY_PART_1                      \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?><D:prop " \
  "xmlns:D=\"DAV:\"><D:lockdiscovery><D:activelock><D:locktoken><D:href>"
#define LOCK_RESPONSE_BODY_PART_2 \
  "</D:href></D:locktoken></D:activelock></D:lockdiscovery></D:prop>"

void WebdavServer::handler_lock(const char* url) {
  esp3d_log("Processing LOCK");
  int code = 200;

  size_t sp = clearPayload();
  (void)sp;
  esp3d_log("Payload size: %d", sp);

  uint8_t fsType = WebDavFS::getFSType(url);
  esp3d_log("FS type of %s : %d", url, fsType);
  if (!isRoot(url)) {
    if (WebDavFS::accessFS(fsType)) {
      if (WebDavFS::exists(url)) {
        // make token
        String token = "opaquelocktoken:";
        token += esp3d_string::generateUUID(url);
        send_response_code(code);
        send_webdav_headers();
        // add token to header
        send_header("Lock-Token", token.c_str());

        // build response body
        String body = LOCK_RESPONSE_BODY_PART_1;
        body += token;
        body += LOCK_RESPONSE_BODY_PART_2;
        // send body
        send_response(body.c_str());
        esp3d_log("%s", body.c_str());
      } else {
        code = 404;
        esp3d_log_e("File not found");
      }
      WebDavFS::releaseFS(fsType);
    } else {
      code = 503;
      esp3d_log_e("FS not available");
    }
  } else {
    code = 400;
    esp3d_log_e("Root cannot be locked");
  }
  if (code != 200) {
    esp3d_log_e("Sending response code %d", code);
    send_response_code(code);
    send_webdav_headers();
  }
}

#endif  // WEBDAV_FEATURE
