/*
  esp_config_file.cpp - ESP3D configuration file support class

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

#include "../../include/esp3d_config.h"

#ifdef SD_UPDATE_FEATURE
#include "../filesystem/esp_sd.h"
#include "esp_config_file.h"

#define LINE_MAX_SIZE 255
#define SECTION_MAX_SIZE 10
#define KEY_MAX_SIZE 30
#define VALUE_MAX_SIZE 128

const char *protectedkeys[] = {"NOTIF_TOKEN1",   "NOTIF_TOKEN2",
                               "AP_Password",    "STA_Password",
                               "ADMIN_PASSWORD", "USER_PASSWORD"};

ESP_ConfigFile::ESP_ConfigFile(const char *path, TProcessingFunction fn) {
  _filename = (char *)malloc(strlen(path) + 1);
  strcpy(_filename, path);
  _pfunction = fn;
}

bool ESP_ConfigFile::processFile() {
  bool res = true;
  if (!ESP_SD::exists(_filename)) {
    esp3d_log_e("No ini file");
    return false;
  }
  ESP_SDFile rFile = ESP_SD::open(_filename);
  if (rFile) {
    bool processing = true;
    char line[LINE_MAX_SIZE + 1];
    char section[SECTION_MAX_SIZE + 1];  // system / network / services
    char key[KEY_MAX_SIZE + 1];
    uint8_t pos = 0;
    line[0] = '\0';
    section[0] = '\0';
    while (processing) {
      // to handle file without endline
      processing = rFile.available();
      char c = '\0';
      if (processing) {
        c = (char)rFile.read();
        if (!((c == '\n') || (c == '\r')) && (pos < (LINE_MAX_SIZE - 1))) {
          line[pos] = c;
          pos++;
        }
      }
      if ((c == '\n') || (c == '\r') || !processing ||
          (pos == (LINE_MAX_SIZE - 1))) {
        line[pos] = '\0';
        char *stmp = trimSpaces(line);
        if (strlen(stmp) > 0) {
          // is comment ?
          if (!isComment(stmp)) {
            // is section ?
            if (isSection(stmp)) {
              strcpy(section, getSectionName(stmp));
            } else {
              // is key + value?
              if (isValue(stmp) && strlen(section) > 0) {
                strcpy(key, getKeyName(stmp));
                if (_pfunction) {
                  if (!_pfunction(section, key, getValue(stmp))) {
                    res = false;
                  }
                }
              }
            }
          }
        }
        pos = 0;
        line[pos] = '\0';
      }
    }
    rFile.close();
    return res;
  }
  esp3d_log_e("Cannot open ini file");
  return false;
}

bool ESP_ConfigFile::isComment(char *line) {
  if (strlen(line) > 0) {
    if ((line[0] == ';') || (line[0] == '#')) {
      return true;
    }
  }
  return false;
}

bool ESP_ConfigFile::isSection(char *line) {
  if (strlen(line) > 0) {
    if ((line[0] == '[') && (line[strlen(line) - 1] == ']')) {
      return true;
    }
  }
  return false;
}

bool ESP_ConfigFile::isValue(char *line) {
  if (strlen(line) > 3) {
    for (uint8_t i = 1; i < strlen(line) - 2; i++) {
      if (line[i] == '=') {
        return true;
      }
    }
  }
  return false;
}

char *ESP_ConfigFile::getSectionName(char *line) {
  line[strlen(line) - 1] = '\0';
  return trimSpaces(&line[1], SECTION_MAX_SIZE);
}

char *ESP_ConfigFile::getKeyName(char *line) {
  for (uint8_t i = 0; i < strlen(line); i++) {
    if (line[i] == '=') {
      line[i] = '\0';
      return trimSpaces(line, KEY_MAX_SIZE);
    }
  }
  return NULL;
}

char *ESP_ConfigFile::getValue(char *line) {
  char *startptr = line + strlen(line) + 1;
  while (*startptr == '\0') {
    startptr++;
  }
  return trimSpaces(startptr, VALUE_MAX_SIZE);
}

char *ESP_ConfigFile::trimSpaces(char *line, uint8_t maxsize) {
  char *endptr = line + strlen(line) - 1;
  char *startptr = line;
  while (endptr >= line && isspace(*endptr)) {
    *endptr-- = '\0';
  }
  endptr = line + strlen(line) - 1;
  while (endptr != startptr && isspace(*startptr)) {
    startptr++;
  }
  if ((maxsize > 0) && (strlen(startptr) > maxsize)) {
    startptr[maxsize] = '\0';
  }
  return startptr;
}

ESP_ConfigFile::~ESP_ConfigFile() { free(_filename); }

bool ESP_ConfigFile::isScrambleKey(const char *key, const char *str) {
  if (strlen(key) > strlen(str)) {
    return false;
  }
  for (uint8_t p = 0; p < strlen(str); p++) {
    if (p < strlen(key)) {
      if (key[p] != str[p]) {
        return false;
      }
    } else {
      if (str[p] != ' ') {
        if (str[p] == '=') {
          return true;
        } else {
          return false;
        }
      }
    }
  }

  return false;
}

bool ESP_ConfigFile::revokeFile() {
  char *filename;
  if (!ESP_SD::exists(_filename)) {
    esp3d_log("No ini file to revoke");
    return false;
  }
  filename = (char *)malloc(strlen(_filename) + 1);
  strcpy(filename, _filename);
  filename[strlen(filename) - 3] = 'o';
  filename[strlen(filename) - 2] = 'k';
  filename[strlen(filename) - 1] = '\0';
  ESP_SD::remove(filename);
  ESP_SDFile wFile = ESP_SD::open(filename, ESP_FILE_WRITE);
  ESP_SDFile rFile = ESP_SD::open(_filename);
  free(filename);
  if (wFile && rFile) {
    bool processing = true;
    char line[LINE_MAX_SIZE + 1];
    uint8_t pos = 0;
    line[0] = '\0';
    while (processing) {
      processing = rFile.available();
      char c = '\0';
      if (processing) {
        c = (char)rFile.read();
        if (!((c == '\n') || (c == '\r')) && (pos < (LINE_MAX_SIZE - 1))) {
          line[pos] = c;
          pos++;
        }
      }
      if ((c == '\n') || (c == '\r') || !processing ||
          (pos == (LINE_MAX_SIZE - 1))) {
        line[pos] = '\0';
        char *stmp = trimSpaces(line);
        if (strlen(stmp) > 0) {
          if (sizeof(protectedkeys) > 0) {
            bool foundscramble = false;
            uint8_t size = sizeof(protectedkeys) / sizeof(char *);
            for (uint8_t i = 0; (i < size) && !foundscramble; i++) {
              if (isScrambleKey(protectedkeys[i], stmp)) {
                strcpy(line, protectedkeys[i]);
                strcat(line, "=********");
                stmp = line;
                foundscramble = true;
              }
            }
          }
          wFile.write((const uint8_t *)stmp, strlen(stmp));
          wFile.write('\r');
          wFile.write('\n');
        }
        pos = 0;
        line[pos] = '\0';
      }
    }
    wFile.close();
    rFile.close();
    ESP_SD::remove(_filename);
    return true;
  }
  esp3d_log_e("Cannot open / create revoked file");
  if (wFile) {
    wFile.close();
  }
  if (rFile) {
    rFile.close();
  }
  return false;
}

#endif  // SD_UPDATE_FEATURE
