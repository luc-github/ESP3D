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
    if (WebDavFS::exists(url) || url == "/") {
      WebDavFile root = WebDavFS::open(url);
      if (root) {
        send_response_code(code);
        send_webdav_headers();

        String body = PROPFIND_RESPONSE_BODY_HEADER_1;
        if (depth != requestedDepth) {
          body += " depth=\"";
          body += depth;
          body += "\"";
        }
        body += PROPFIND_RESPONSE_BODY_HEADER_2;
        body += "<D:response xmlns:esp3d=\"DAV:\">\r\n";
        body += "<D:href>";
        body += url;
        body += "</D:href>\r\n";
        body += "<D:propstat>\r\n";
        body += "<D:prop>\r\n";
        body += "<esp3d:getlastmodified>";
        body += root.getLastWrite();
        body += "</esp3d:getlastmodified>\r\n";
        if (root.isDirectory()) {
          body += "<D:resourcetype>";
          body += "<D:collection/>";
          body += "</D:resourcetype>\r\n";
        } else {
          body += "<D:resourcetype/>";
          body += "<esp3d:getcontentlength>";
          body += root.size();
          body += "</esp3d:getcontentlength>\r\n";
        }

        body += "<esp3d:displayname>";
        body += url;
        body += "</esp3d:displayname>\r\n";
        body += "</D:prop>\r\n";
        body += "<D:status>HTTP/1.1 200 OK</D:status>\r\n";
        body += "</D:propstat>\r\n";
        body += "</D:response>\r\n";

        // TODO: send as chunck id depth = 1
        // else send as response
        if (depth == "1" && root.isDirectory()) {
          WebDavFile entry = root.openNextFile();
          while (entry) {
            yield();
            // TODO: send as chunck the data
            entry.close();
            entry = root.openNextFile();
          }
          body = PROPFIND_RESPONSE_BODY_FOOTER;
          // TODO: send as chunck
        } else {
          body += PROPFIND_RESPONSE_BODY_FOOTER;
          send_response(body.c_str());
        }
        root.close();
      } else {
        code = 503;
        log_esp3d("File not opened");
      }

    } else {
      code = 404;
      log_esp3d("File not found");
    }
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
