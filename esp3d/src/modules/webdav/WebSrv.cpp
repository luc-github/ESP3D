/*
    Dead simple web-server.
    Supports only one simultaneous client, knows how to handle GET and POST.

    Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
    Simplified/Adapted for ESPWebDav:
    Copyright (c) 2018 Gurpreet Bal https://github.com/ardyesp/ESPWebDAV
    Copyright (c) 2020 David Gauchard https://github.com/d-a-v/ESPWebDAV

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
    Modified 22 Jan 2021 by Luc Lebosse (ESP3D Integration)
*/
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"

#if defined (WEBDAV_FEATURE)
#include "ESPWebDAV.h"

// Sections are copied from ESP8266Webserver

String ESPWebDAV::getMimeType(const String& path)
{
    if (path.endsWith(".html")) {
        return "text/html";
    } else if (path.endsWith(".htm")) {
        return "text/html";
    } else if (path.endsWith(".css")) {
        return "text/css";
    } else if (path.endsWith(".txt")) {
        return "text/plain";
    } else if (path.endsWith(".js")) {
        return "application/javascript";
    } else if (path.endsWith(".json")) {
        return "application/json";
    } else if (path.endsWith(".png")) {
        return "image/png";
    } else if (path.endsWith(".gif")) {
        return "image/gif";
    } else if (path.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (path.endsWith(".ico")) {
        return "image/x-icon";
    } else if (path.endsWith(".svg")) {
        return "image/svg+xml";
    } else if (path.endsWith(".ttf")) {
        return "application/x-font-ttf";
    } else if (path.endsWith(".otf")) {
        return "application/x-font-opentype";
    } else if (path.endsWith(".woff")) {
        return "application/font-woff";
    } else if (path.endsWith(".woff2")) {
        return "application/font-woff2";
    } else if (path.endsWith(".eot")) {
        return "application/vnd.ms-fontobject";
    } else if (path.endsWith(".sfnt")) {
        return "application/font-sfnt";
    } else if (path.endsWith(".xml")) {
        return "text/xml";
    } else if (path.endsWith(".pdf")) {
        return "application/pdf";
    } else if (path.endsWith(".zip")) {
        return "application/zip";
    } else if (path.endsWith(".gz")) {
        return "application/x-gzip";
    } else if (path.endsWith(".appcache")) {
        return "text/cache-manifest";
    }

    return "application/octet-stream";
}




String ESPWebDAV::urlDecode(const String& text)
{
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = text.length();
    unsigned int i = 0;
    while (i < len) {
        char decodedChar;
        char encodedChar = text.charAt(i++);
        if ((encodedChar == '%') && (i + 1 < len)) {
            temp[2] = text.charAt(i++);
            temp[3] = text.charAt(i++);
            decodedChar = strtol(temp, NULL, 16);
        } else {
            if (encodedChar == '+') {
                decodedChar = ' ';
            } else {
                decodedChar = encodedChar;    // normal ascii char
            }
        }
        decoded += decodedChar;
    }
    return decoded;
}

void ESPWebDAV::handleClient()
{
    if (!server) {
        log_esp3d("handleClient: server is null");
        return;
    }

    if (server->hasClient()) {
        if (!locClient || !locClient.available()) {
            // no or sleeping current client
            // take it over
            locClient = server->available();
            m_persistent_timer_ms = millis();
            log_esp3d("NEW CLIENT-------------------------------------------------------");
        }
    }

    if (!locClient || !locClient.available()) {
        // no current client
        //do not put debug here or it will flood the serial
        return;
    }

    // extract uri, headers etc
    parseRequest();

    if (!m_persistent)
        // close the connection
    {
        log_esp3d("CLOSE CONNECTION-------------------------------------------------------");
        locClient.stop();
    }
}


bool ESPWebDAV::parseRequest()
{
    // Read the first line of HTTP request
    String req = locClient.readStringUntil('\r');
    locClient.readStringUntil('\n');

    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
        log_esp3d("Invalid request %s", req.c_str());
        return false;
    }

    method = req.substring(0, addr_start);
    uri = urlDecode(req.substring(addr_start + 1, addr_end));
    return ESPWebDAVCore::parseRequest(method, uri, &locClient, getMimeType);
}

#endif //WEBDAV_FEATURE
