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
#include "esp3d_settings.h"

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#endif  // WIFI_FEATURE || ETH_FEATURE

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

// Encode a string to be used in a URL
const char* esp3d_string::urlEncode(const char* s) {
  static String encoded;
  encoded = "";
  char temp[4];
  for (size_t i = 0; i < strlen(s); i++) {
    temp[0] = s[i];
    if (temp[0] == 32) {  // space
      encoded.concat('+');
    } else if ((temp[0] >= 48 && temp[0] <= 57)     /*0-9*/
               || (temp[0] >= 65 && temp[0] <= 90)  /*A-Z*/
               || (temp[0] >= 97 && temp[0] <= 122) /*a-z*/
    ) {
      encoded.concat(temp[0]);
    } else {  // character needs encoding
      snprintf(temp, 4, "%%%02X", temp[0]);
      encoded.concat(temp);
    }
  }
  return encoded.c_str();
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

bool esp3d_string::isPrintableChar(char ch) {
  int c = static_cast<int>(ch);
  if (c == '\t' || c == '\r' || (c >= ' ' && c <= '~') || c >= 128) {
    return true;
  }
  return false;
}

const char* esp3d_string::expandString(const char* s, bool formatspace) {
  static String tmp;
  tmp = s;
  if (tmp.indexOf("%") != -1) {
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    tmp.replace("%ESP_IP%", NetConfig::localIP().c_str());
    tmp.replace("%ESP_NAME%", NetConfig::hostname());
#else
    tmp.replace("%ESP_IP%", "???");
    tmp.replace("%ESP_NAME%", "???");
#endif  // WIFI_FEATURE || ETH_FEATURE
#if defined(TIMESTAMP_FEATURE)
    String dt = timeService.getCurrentTime();
    if (formatspace) dt.replace(" ", "\\ ");
    tmp.replace("%ESP_DATETIME%", dt.c_str());
#else
    tmp.replace("%ESP_DATETIME%", "???");
#endif  // TIMESTAMP_FEATURE
  }
  return tmp.c_str();
}

const char* esp3d_string::formatDuration(uint64_t duration) {
  unsigned long seconds = duration / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  static String result;
  bool display = false;
  result = "";
  if (days > 0) {
    result += String(days) + "d ";
    display = true;
  }
  if (hours > 0 || display) {
    result += String(hours) + "h ";
    display = true;
  }
  if (minutes > 0 || display) {
    result += String(minutes) + "m ";
    display = true;
  }
  result += String(seconds) + "s";

  return result.c_str();
}

bool esp3d_string::isRealTimeCommand(char c) {
  if (ESP3DSettings::GetFirmwareTarget() == GRBL ||
      ESP3DSettings::GetFirmwareTarget() == GRBLHAL) {
    // Standard characters
    if (c == '?' || c == '!' || c == '~' || c == 0x18) {  // 0x18 is  ^X
      return true;
    }

    // Range >= 0x80 et <= 0xA4
    const unsigned char uc = static_cast<unsigned char>(c);
    if (uc >= 0x80 && uc <= 0xA4) {
      return true;
    }
  }
  return false;
}