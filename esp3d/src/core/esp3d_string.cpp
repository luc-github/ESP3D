/*
  esp3d_string.cpp - esp3d strings helpers

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
#include "esp3d_string.h"

#include <Arduino.h>

#include "../include/esp3d_config.h"

#if defined(TIMESTAMP_FEATURE)
#include "../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE

const char* esp3d_string::getTimeString(time_t time, bool isGMT) {
  static char buffer[40];
  memset(buffer, 0, sizeof(buffer));
  struct tm* tm_info;
  struct tm tmstruct;

  if (isGMT) {
    // convert to GMT time
    tm_info = gmtime_r(&time, &tmstruct);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
  } else {
    // convert to local time
    tm_info = localtime_r(&time, &tmstruct);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm_info);
  }
  if (!isGMT) {
#if defined(TIMESTAMP_FEATURE)
    // if time zone is set add it
    strcat(buffer, timeService.getTimeZone());
#else
    // add Z to indicate UTC time because no time zone is set
    strcat(buffer, "Z");
#endif  // TIMESTAMP_FEATURE
  }
  return buffer;
}

// Update hash function used by generateUUID
void update_hash(uint8_t* data, size_t len, uint8_t* hash_buffer,
                 uint8_t hash_size) {
  static bool reverse = false;
  reverse = !reverse;
  int start_index = reverse ? hash_size : 0;
  for (uint8_t i = 0; i < hash_size; i++) {
    int idx =
        reverse ? (start_index - i) % hash_size : (start_index + i) % hash_size;
    if (i >= len) {
      hash_buffer[idx] ^= random(1, 254);
    } else {
      hash_buffer[idx] ^= data[i];
    }
  }
}

const char* esp3d_string::generateUUID(const char* seed) {
  static String token;
  String tmp;
  uint8_t hash_buffer[16];
  memset(hash_buffer, 0, 16);
  if (!seed) {
    tmp = "ESP3D ecosystem";
  } else {
    tmp = seed;
  }
  // init random seed
  randomSeed(time(NULL));
  // Use seed
  update_hash((uint8_t*)tmp.c_str(), tmp.length(), hash_buffer, 16);

  // use epoch time
  uint64_t millisec = millis();
  update_hash((uint8_t*)&millisec, sizeof(millisec), hash_buffer, 16);

  // use current time
  time_t now;
  time(&now);
  update_hash((uint8_t*)&now, sizeof(now), hash_buffer, 16);

  tmp = "";
  // now hash all the buffer
  for (int i = 0; i < 16; i++) {
    char hex[3];
    sprintf(hex, "%02x", hash_buffer[i]);
    tmp += hex;
  }

  // format the uuid on 36 chars
  token = tmp.substring(0, 7) + "-";
  token += tmp.substring(8, 8 + 3) + "-";
  token += tmp.substring(12, 12 + 3) + "-";
  token += tmp.substring(16, 16 + 3) + "-";
  token += &tmp[20];

  return token.c_str();
}

const char* esp3d_string::getContentType(const char* filename) {
  String file_name = filename;
  file_name.toLowerCase();
  if (file_name.endsWith(".htm")) {
    return "text/html";
  } else if (file_name.endsWith(".html")) {
    return "text/html";
  } else if (file_name.endsWith(".css")) {
    return "text/css";
  } else if (file_name.endsWith(".js")) {
    return "application/javascript";
  } else if (file_name.endsWith(".png")) {
    return "image/png";
  } else if (file_name.endsWith(".gif")) {
    return "image/gif";
  } else if (file_name.endsWith(".jpeg")) {
    return "image/jpeg";
  } else if (file_name.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (file_name.endsWith(".ico")) {
    return "image/x-icon";
  } else if (file_name.endsWith(".xml")) {
    return "text/xml";
  } else if (file_name.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (file_name.endsWith(".zip")) {
    return "application/x-zip";
  } else if (file_name.endsWith(".gz")) {
    return "application/x-gzip";
  } else if (file_name.endsWith(".txt") || file_name.endsWith(".gcode") ||
             file_name.endsWith(".gco") || file_name.endsWith(".g")) {
    return "text/plain";
  }
  return "application/octet-stream";
}

// tool function to avoid string corrupt JSON files
const char* esp3d_string::encodeString(const char* s) {
  static String tmp;
  tmp = s;
  while (tmp.indexOf("'") != -1) {
    tmp.replace("'", "&#39;");
  }
  while (tmp.indexOf("\"") != -1) {
    tmp.replace("\"", "&#34;");
  }
  if (tmp == "") {
    tmp = " ";
  }
  return tmp.c_str();
}

// helper to format size to readable string
const char* esp3d_string::formatBytes(uint64_t bytes) {
  static String res;
  if (bytes < 1024) {
    res = String((uint16_t)bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    res = String((float)(bytes / 1024.0), 2) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    res = String((float)(bytes / 1024.0 / 1024.0), 2) + " MB";
  } else {
    res = String((float)(bytes / 1024.0 / 1024.0 / 1024.0), 2) + " GB";
  }
  return res.c_str();
}