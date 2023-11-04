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

#define PROPFIND_RESPONSE_BODY_HEADER_1        \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
  "<D:multistatus xmlns:D=\"DAV:\""
#define PROPFIND_RESPONSE_BODY_HEADER_2 ">"

#define PROPFIND_RESPONSE_BODY_FOOTER "</D:multistatus>"

void WebdavServer::handler_propfind(const char* url) {
  log_esp3d("Processing PROPFIND");
  int code = 207;
  String depth = "0";
  String requestedDepth = "0";
  if (hasHeader("Depth")) {
    depth = getHeader("Depth");
    log_esp3d("Depth: %s", depth.c_str());
    requestedDepth = depth;
    if (depth == "infinity") {
      depth = "1";
      log_esp3d("Depth set to 1");
    }
  } else {
    log_esp3d("Depth not set");
  }

  size_t sp = clearPayload();
  log_esp3d("Payload size: %d", sp);

  uint8_t fsType = WebDavFS::getFSType(url);
  log_esp3d("FS type of %s : %d", url, fsType);
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
        body += "<D:response xmlns:esp3d=\"DAV:\">";
        body += "<D:href>";
        body += url;
        body += "</D:href>";
        body += "<D:propstat>";
        body += "<D:prop>";
        body += "<esp3d:getlastmodified>";
        body += esp3d_string::getTimeString((time_t)root.getLastWrite(), true);
        body += "</esp3d:getlastmodified>";
        if (root.isDirectory()) {
          body += "<D:resourcetype>";
          body += "<D:collection/>";
          body += "</D:resourcetype>";
        } else {
          body += "<D:resourcetype/>";
          body += "<esp3d:getcontentlength>";
          body += root.size();
          body += "</esp3d:getcontentlength>";
        }
        body += "<esp3d:displayname>";
        body += url;
        body += "</esp3d:displayname>";
        body += "</D:prop>";
        body += "<D:status>HTTP/1.1 200 OK</D:status>";
        body += "</D:propstat>";
        body += "</D:response>";

        if (depth == "0") {
          body += PROPFIND_RESPONSE_BODY_FOOTER;
          send_response(body.c_str());
          log_esp3d("%s", body.c_str());
        } else {
          send_chunk_content(body.c_str());
          log_esp3d("%s", body.c_str());
          if (depth == "1" && root.isDirectory()) {
            log_esp3d("Depth 1, parsing directory");
            WebDavFile entry = root.openNextFile();
            while (entry) {
              yield();
              log_esp3d("Processing %s from %s", entry.name(), url);
              body = "<D:response xmlns:esp3d=\"DAV:\">";
              body += "<D:href>";
              body += url;
              if (strcmp(url, "/") != 0) {
                body += "/";
                log_esp3d("Adding / to *%s*", url);
              }
              body += entry.name()[0] == '/' ? entry.name() + 1 : entry.name();
              body += "</D:href>";
              body += "<D:propstat>";
              body += "<D:prop>";
              body += "<esp3d:getlastmodified>";
              body += esp3d_string::getTimeString((time_t)entry.getLastWrite(),
                                                  true);
              body += "</esp3d:getlastmodified>";
              if (entry.isDirectory()) {
                body += "<D:resourcetype>";
                body += "<D:collection/>";
                body += "</D:resourcetype>";
              } else {
                body += "<D:resourcetype/>";
                body += "<esp3d:getcontentlength>";
                body += entry.size();
                body += "</esp3d:getcontentlength>";
              }
              body += "<esp3d:displayname>";
              body += entry.name()[0] == '/' ? entry.name() + 1 : entry.name();
              body += "</esp3d:displayname>";
              body += "</D:prop>";
              body += "<D:status>HTTP/1.1 200 OK</D:status>";
              body += "</D:propstat>";
              body += "</D:response>";
              log_esp3d("%s", body.c_str());
              send_chunk_content(body.c_str());
              entry.close();
              entry = root.openNextFile();
            }
            log_esp3d("%s", PROPFIND_RESPONSE_BODY_FOOTER);
            send_chunk_content(PROPFIND_RESPONSE_BODY_FOOTER);
            // End of chunk
            send_chunk_content("");
          }
        }
        root.close();
      } else {
        code = 503;
        log_esp3d_e("File not opened");
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
  if (code != 207) {
    log_esp3d_e("Sending response code %d", code);
    send_response_code(code);
    send_webdav_headers();
  }
}

#endif  // WEBDAV_FEATURE
