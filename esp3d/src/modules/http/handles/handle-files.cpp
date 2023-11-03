/*
 handle-files.cpp - ESP3D http handle

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

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
#if defined(HTTP_FEATURE) && defined(FILESYSTEM_FEATURE)
#include "../http_server.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
#include "../../authentication/authentication_service.h"
#include "../../filesystem/esp_filesystem.h"

#ifdef FILESYSTEM_TIMESTAMP_FEATURE
#include "../../time/time_service.h"
#endif  // FILESYSTEM_TIMESTAMP_FEATURE

// Filesystem
// Filesystem files list and file commands
void HTTP_Server::handleFSFileList() {
  HTTP_Server::set_http_headers();

  level_authenticate_type auth_level =
      AuthenticationService::authenticated_level();
  if (auth_level == LEVEL_GUEST) {
    _upload_status = UPLOAD_STATUS_NONE;
    _webserver->send(401, "text/plain", "Wrong authentication!");
    return;
  }
  String path;
  String status = "ok";
  if ((_upload_status == UPLOAD_STATUS_FAILED) ||
      (_upload_status == UPLOAD_STATUS_CANCELLED)) {
    status = "Upload failed";
    _upload_status = UPLOAD_STATUS_NONE;
  }
  if (_webserver->hasArg("quiet")) {
    if (_webserver->arg("quiet") == "yes") {
      status = "{\"status\":\"" + status + "\"}";
      _webserver->send(200, "text/plain", status.c_str());
      return;
    }
  }
  // get current path
  if (_webserver->hasArg("path")) {
    path += _webserver->arg("path");
  }
  // to have a clean path
  path.trim();
  path.replace("//", "/");
  if (path[path.length() - 1] != '/') {
    path += "/";
  }
  // check if query need some action
  if (_webserver->hasArg("action")) {
    // delete a file
    if (_webserver->arg("action") == "delete" &&
        _webserver->hasArg("filename")) {
      String filename;
      String shortname = _webserver->arg("filename");
      shortname.replace("/", "");
      filename = path + _webserver->arg("filename");
      filename.replace("//", "/");
      if (!ESP_FileSystem::exists(filename.c_str())) {
        status = shortname + " does not exists!";
      } else {
        if (ESP_FileSystem::remove(filename.c_str())) {
          status = shortname + " deleted";
          // what happen if no "/." and no other subfiles for SPIFFS like?
          String ptmp = path;
          if ((path != "/") && (path[path.length() - 1] = '/')) {
            ptmp = path.substring(0, path.length() - 1);
          }
          if (!ESP_FileSystem::exists(ptmp.c_str())) {
            ESP_FileSystem::mkdir(ptmp.c_str());
          }
        } else {
          status = "Cannot deleted ";
          status += shortname;
        }
      }
    }
    // delete a directory
    if (_webserver->arg("action") == "deletedir" &&
        _webserver->hasArg("filename")) {
      String filename;
      String shortname = _webserver->arg("filename");
      shortname.replace("/", "");
      filename = path + _webserver->arg("filename");
      filename += "/";
      filename.replace("//", "/");
      if (filename != "/") {
        if (ESP_FileSystem::rmdir(filename.c_str())) {
          log_esp3d("Deleting %s", filename.c_str());
          status = shortname;
          status += " deleted";
        } else {
          status = "Cannot deleted ";
          status += shortname;
        }
      }
    }
    // create a directory
    if (_webserver->arg("action") == "createdir" &&
        _webserver->hasArg("filename")) {
      String filename;
      filename = path + _webserver->arg("filename");
      String shortname = _webserver->arg("filename");
      shortname.replace("/", "");
      filename.replace("//", "/");
      if (ESP_FileSystem::exists(filename.c_str())) {
        status = shortname + " already exists!";
      } else {
        if (!ESP_FileSystem::mkdir(filename.c_str())) {
          status = "Cannot create ";
          status += shortname;
        } else {
          status = shortname + " created";
        }
      }
    }
  }
  String buffer2send;
  buffer2send.reserve(1200);
  buffer2send = "{\"files\":[";
  String ptmp = path;
  if ((path != "/") && (path[path.length() - 1] = '/')) {
    ptmp = path.substring(0, path.length() - 1);
  }
  _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
  _webserver->sendHeader("Content-Type", "application/json");
  _webserver->sendHeader("Cache-Control", "no-cache");
  _webserver->send(200);
  if (ESP_FileSystem::exists(ptmp.c_str())) {
    ESP_File f = ESP_FileSystem::open(ptmp.c_str(), ESP_FILE_READ);
    // Parse files
    ESP_File sub = f.openNextFile();
    if (f) {
      bool needseparator = false;
      while (sub) {
        if (needseparator) {
          buffer2send += ",";
        } else {
          // for next entry
          needseparator = true;
        }
        buffer2send += "{\"name\":\"";
        buffer2send += sub.name();
        buffer2send += "\",\"size\":\"";
        if (sub.isDirectory()) {
          buffer2send += "-1";
        } else {
          buffer2send += ESP_FileSystem::formatBytes(sub.size());
        }
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
        buffer2send += "\",\"time\":\"";
        if (!sub.isDirectory()) {
          buffer2send += timeService.getDateTime((time_t)sub.getLastWrite());
        }
#endif  // FILESYSTEM_TIMESTAMP_FEATURE
        buffer2send += "\"}";
        if (buffer2send.length() > 1100) {
          _webserver->sendContent_P(buffer2send.c_str(), buffer2send.length());
          buffer2send = "";
        }
        sub.close();
        sub = f.openNextFile();
      }
      f.close();
    } else {
      if (status == "ok") {
        status = "cannot open" + ptmp;
      } else {
        status += ", cannot open" + ptmp;
      }
    }
  } else {
    if (status == "ok") {
      status = ptmp + " does not exists!";
    } else {
      status += ", " + ptmp + " does not exists!";
    }
  }
  buffer2send += "],\"path\":\"" + path + "\",";

  if (ESP_FileSystem::totalBytes() > 0) {
    buffer2send += "\"occupation\":\"" +
                   String(100 * ESP_FileSystem::usedBytes() /
                          ESP_FileSystem::totalBytes()) +
                   "\",";
  } else {
    status = "FileSystem Error";
    buffer2send += "\"occupation\":\"0\",";
  }
  buffer2send += "\"status\":\"" + status + "\",";
  buffer2send += "\"total\":\"" +
                 ESP_FileSystem::formatBytes(ESP_FileSystem::totalBytes()) +
                 "\",";
  buffer2send += "\"used\":\"" +
                 ESP_FileSystem::formatBytes(ESP_FileSystem::usedBytes()) +
                 "\"}";
  path = "";
  _webserver->sendContent_P(buffer2send.c_str(), buffer2send.length());
  _webserver->sendContent("");
  _upload_status = UPLOAD_STATUS_NONE;
}

#endif  // HTTP_FEATURE && FILESYSTEM_FEATURE
