/*
  esp_sd.cpp - ESP3D SD support class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#ifdef SD_DEVICE
#include <time.h>

#include "esp_sd.h"

#define ESP_MAX_SD_OPENHANDLE 4
#if (SD_DEVICE == ESP_SD_NATIVE) && defined(ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#include <SD.h>
File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#elif ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && \
    defined(ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#define NO_GLOBAL_SD
#include <SdFat.h>
#if SDFAT_FILE_TYPE == 1
typedef File32 File;
#elif SDFAT_FILE_TYPE == 2
typedef ExFile File;
#elif SDFAT_FILE_TYPE == 3
typedef FsFile File;
#else  // SDFAT_FILE_TYPE
#error Invalid SDFAT_FILE_TYPE
#endif  // SDFAT_FILE_TYPE
File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#elif ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && \
    defined(ARDUINO_ARCH_ESP32)
#include <SdFat.h>
#if (SD_DEVICE == ESP_SDFAT2)
#if SDFAT_FILE_TYPE == 1
typedef File32 File;
#elif SDFAT_FILE_TYPE == 2
typedef ExFile File;
#elif SDFAT_FILE_TYPE == 3
typedef FsFile File;
#else  // SDFAT_FILE_TYPE
#error Invalid SDFAT_FILE_TYPE
#endif  // SDFAT_FILE_TYPE
#endif
File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#else
#include <FS.h>
File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#endif

#if defined(ESP3DLIB_ENV)
#include "../../include/Marlin/cardreader.h"
#endif  // ESP3DLIB_ENV

#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
bool ESP_SD::_enabled = false;
#if SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD
#include <SPI.h>
#endif  // SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD
bool ESP_SD::enableSharedSD() {
  esp3d_log("Enable Shared SD if possible");
  if (_enabled) {
    esp3d_log("Already enabled, skip");
    return false;
  }
#if defined(ESP_SD_CS_SENSE) && ESP_SD_CS_SENSE != -1
  bool active_cs = !digitalRead(ESP_SD_CS_SENSE);
  if (active_cs) {
    esp3d_log("SD CS is active, skip");
    return false;
  }
#endif  // ESP_SD_CS_SENSE

  _enabled = true;
#if defined(ESP_FLAG_SHARED_SD_PIN) && ESP_FLAG_SHARED_SD_PIN != -1
  // need to check if SD is in use ?
  // Method : TBD
  // 1 - check sd cs state ? what about SDIO then ?
  // 2 - check M27 status ?
  esp3d_log("SD shared enabled PIN %d with %d", ESP_FLAG_SHARED_SD_PIN,
            ESP_FLAG_SHARED_SD_VALUE);
  digitalWrite(ESP_FLAG_SHARED_SD_PIN, ESP_FLAG_SHARED_SD_VALUE);
  ESP3DHal::wait(100);
#endif  // ESP_FLAG_SHARED_SD_PIN

#if SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD
  esp3d_log("Custom spi : CS: %d,  Miso: %d, Mosi: %d, SCK: %d", ESP_SD_CS_PIN,
            ESP_SD_MISO_PIN, ESP_SD_MOSI_PIN, ESP_SD_SCK_PIN);
  SPI.begin(ESP_SD_SCK_PIN, ESP_SD_MISO_PIN, ESP_SD_MOSI_PIN, ESP_SD_CS_PIN);
#endif  // SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD

#if defined(ESP3DLIB_ENV)
  // check if card is not currently in use
  if (card.isMounted() && (IS_SD_PRINTING() || IS_SD_FETCHING() ||
                           IS_SD_PAUSED() || IS_SD_FILE_OPEN())) {
    _enabled = false;
  } else {
    card.release();
  }
#endif  // ESP3DLIB_ENV

  return _enabled;
}

bool ESP_SD::disableSharedSD() {
#if SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD
  pinMode(ESP_SD_CS_PIN, INPUT_PULLUP);
  pinMode(ESP_SD_MISO_PIN, INPUT_PULLUP);
  pinMode(ESP_SD_MOSI_PIN, INPUT_PULLUP);
  pinMode(ESP_SD_SCK_PIN, INPUT_PULLUP);
  pinMode(ESP_SD_D1_PIN, INPUT_PULLUP);
  pinMode(ESP_SD_D2_PIN, INPUT_PULLUP);
  SPI.end();
#endif  // SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD
  // do the switch
  digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
  ESP3DHal::wait(100);
  return true;
}
#endif  // SD_DEVICE_CONNECTION == ESP_SHARED_SD

bool ESP_SD::_started = false;
uint8_t ESP_SD::_state = ESP_SDCARD_NOT_PRESENT;
uint8_t ESP_SD::_spi_speed_divider = 1;
bool ESP_SD::_sizechanged = true;
uint8_t ESP_SD::setState(uint8_t flag) {
  _state = flag;
  return _state;
}

uint8_t ESP_SD::getFSType(const char* path) {
  (void)path;
  return FS_SD;
}

bool ESP_SD::accessFS(uint8_t FS) {
  (void)FS;
  bool res = true;
  // if card is busy do not let another task access SD and so prevent a release
  if (_state == ESP_SDCARD_BUSY) {
    esp3d_log("SD Busy");
    return false;
  }
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
  if (ESP_SD::enableSharedSD()) {
    esp3d_log("Access shared SD ok");
    res = true;
  } else {
    esp3d_log_e("Enable shared SD failed");
    res = false;
  }
#else
  esp3d_log("Accessing Direct SD");
  res = true;
#endif  // SD_DEVICE_CONNECTION == ESP_SHARED_SD
  if (res) {
    esp3d_log("Checking SD state");
    if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
      esp3d_log_e("SD not present");
      res = false;
      // Sd is not available so release it
      ESP_SD::releaseFS(FS);
    } else {
      esp3d_log("SD present");
      res = true;
      esp3d_log("Accessing SD is ok");
      ESP_SD::setState(ESP_SDCARD_BUSY);
    }
  }
  return res;
}
void ESP_SD::releaseFS(uint8_t FS) {
  (void)FS;
  esp3d_log("Release SD");
  setState(ESP_SDCARD_IDLE);
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
  _enabled = false;
#if defined(ESP_FLAG_SHARED_SD_PIN) && ESP_FLAG_SHARED_SD_PIN != -1
  if (ESP_SD::disableSharedSD()) {
    esp3d_log("Shared SD disabled");
  }
#endif  // ESP_FLAG_SHARED_SD_PIN
#if defined(ESP3DLIB_ENV)
  esp3d_log("Mount SD in Marlin");
  card.mount();
#endif  // ESP3DLIB_ENV
#endif  // SD_DEVICE_CONNECTION == ESP_SHARED_SD
}

void ESP_SD::handle() {}

ESP_SDFile::ESP_SDFile(const char* name, const char* filename, bool isdir,
                       size_t size) {
  _isdir = isdir;
  _dirlist = "";
  _index = -1;
  _filename = filename;
  _name = name;
  _lastwrite = 0;
  _iswritemode = false;
  _size = size;
}

ESP_SDFile::~ESP_SDFile() {
  // esp3d_log("Destructor %s index %d",(_isdir)?"Dir":"File", _index);
}

ESP_SDFile::operator bool() const {
  if ((_index != -1) || (_filename.length() > 0)) {
    // esp3d_log("Bool yes %d %d",_index,  _filename.length());
    return true;
  } else {
    return false;
  }
}

bool ESP_SDFile::isOpen() { return !(_index == -1); }

const char* ESP_SDFile::name() const { return _name.c_str(); }

const char* ESP_SDFile::filename() const { return _filename.c_str(); }

bool ESP_SDFile::isDirectory() { return _isdir; }

size_t ESP_SDFile::size() { return _size; }

time_t ESP_SDFile::getLastWrite() { return _lastwrite; }

int ESP_SDFile::available() {
  if (_index == -1 || _isdir) {
    return 0;
  }
  return tSDFile_handle[_index].available();
}

size_t ESP_SDFile::write(uint8_t i) {
  if ((_index == -1) || _isdir) {
    return 0;
  }
  return tSDFile_handle[_index].write(i);
}

size_t ESP_SDFile::write(const uint8_t* buf, size_t size) {
  if ((_index == -1) || _isdir) {
    return 0;
  }
  return tSDFile_handle[_index].write(buf, size);
}

int ESP_SDFile::read() {
  if ((_index == -1) || _isdir) {
    return -1;
  }
  return tSDFile_handle[_index].read();
}

size_t ESP_SDFile::read(uint8_t* buf, size_t size) {
  if ((_index == -1) || _isdir) {
    return -1;
  }
  return tSDFile_handle[_index].read(buf, size);
}

void ESP_SDFile::flush() {
  if ((_index == -1) || _isdir) {
    return;
  }
  tSDFile_handle[_index].flush();
}

ESP_SDFile& ESP_SDFile::operator=(const ESP_SDFile& other) {
  // esp3d_log("Copy %s", other._filename.c_str());
  _isdir = other._isdir;
  _index = other._index;
  _filename = other._filename;
  _name = other._name;
  _size = other._size;
  _iswritemode = other._iswritemode;
  _dirlist = other._dirlist;
  _lastwrite = other._lastwrite;
  return *this;
}

bool ESP_SD::setSPISpeedDivider(uint8_t speeddivider) {
  if (speeddivider > 0) {
    _spi_speed_divider = speeddivider;
    return true;
  }
  return false;
}

#endif  // SD_DEVICE
