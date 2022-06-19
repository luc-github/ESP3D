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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#ifdef SD_DEVICE
#include "esp_sd.h"
#include "../../core/genLinkedList.h"
#include <time.h>

#define ESP_MAX_SD_OPENHANDLE 4
#if (SD_DEVICE == ESP_SD_NATIVE)  && defined (ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#include <SD.h>
File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#elif ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined (ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#define NO_GLOBAL_SD
#include <SdFat.h>
sdfat::File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];
#elif ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined (ARDUINO_ARCH_ESP32)
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

#if defined (ESP3DLIB_ENV)
#include "../../include/Marlin/cardreader.h"
#endif // ESP3DLIB_ENV

#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
bool ESP_SD::_enabled = false;

bool ESP_SD::enableSharedSD()
{
    if(_enabled) {
        return false;
    }
    _enabled = true;
#if defined (ESP_FLAG_SHARED_SD_PIN)  && ESP_FLAG_SHARED_SD_PIN != -1
    //need to check if SD is in use ?
    //Method : TBD
    //1 - check sd cs state ? what about SDIO then ?
    //2 - check M27 status ?
    log_esp3d("SD shared enabled PIN %d with %d", ESP_FLAG_SHARED_SD_PIN, ESP_FLAG_SHARED_SD_VALUE);
    digitalWrite(ESP_FLAG_SHARED_SD_PIN, ESP_FLAG_SHARED_SD_VALUE);
#endif // ESP_FLAG_SHARED_SD_PIN
#if defined (ESP3DLIB_ENV)
    //check if card is not currently in use
    if (card.isMounted() && (IS_SD_PRINTING() ||IS_SD_FETCHING() ||IS_SD_PAUSED() ||  IS_SD_FILE_OPEN())) {
        _enabled = false;
    } else {
        card.release();
    }
#endif // ESP3DLIB_ENV
    return _enabled;
}
#endif // SD_DEVICE_CONNECTION == ESP_SHARED_SD

bool ESP_SD::_started = false;
uint8_t ESP_SD::_state = ESP_SDCARD_NOT_PRESENT;
uint8_t ESP_SD::_spi_speed_divider = 1;
bool ESP_SD::_sizechanged = true;
uint8_t ESP_SD::setState(uint8_t flag)
{
    _state =  flag;
    return _state;
}

uint8_t ESP_SD::getFSType(const char * path)
{
    (void)path;
    return FS_SD;
}

bool  ESP_SD::accessFS(uint8_t FS)
{
    (void)FS;
    //if card is busy do not let another task access SD and so prevent a release
    if (_state == ESP_SDCARD_BUSY) {
        log_esp3d("SD Busy");
        return false;
    }
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
    if  (ESP_SD::enableSharedSD()) {
        log_esp3d("Access SD");
        return true;
    } else {
        log_esp3d("Enable shared SD failed");
        return false;
    }
#else
    log_esp3d("Access SD");
    return true;
#endif // SD_DEVICE_CONNECTION == ESP_SHARED_SD
}
void  ESP_SD::releaseFS(uint8_t FS)
{
    (void)FS;
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
#if defined (ESP_FLAG_SHARED_SD_PIN)  && ESP_FLAG_SHARED_SD_PIN != -1
    log_esp3d("SD shared disabled PIN %d with %d", ESP_FLAG_SHARED_SD_PIN, ESP_FLAG_SHARED_SD_VALUE);
    digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
#endif // ESP_FLAG_SHARED_SD_PIN
#if defined (ESP3DLIB_ENV)
    log_esp3d("Release SD");
    card.mount();
#endif // ESP3DLIB_ENV
    _enabled = false;
#endif //SD_DEVICE_CONNECTION == ESP_SHARED_SD 
    setState(ESP_SDCARD_IDLE);
}


void ESP_SD::handle()
{

}

//helper to format size to readable string
String & ESP_SD::formatBytes (uint64_t bytes)
{
    static String res;
    if (bytes < 1024) {
        res = String ((uint16_t)bytes) + " B";
    } else if (bytes < (1024 * 1024) ) {
        res = String ((float)(bytes / 1024.0),2) + " KB";
    } else if (bytes < (1024 * 1024 * 1024) ) {
        res = String ((float)(bytes / 1024.0 / 1024.0),2) + " MB";
    } else {
        res = String ((float)(bytes / 1024.0 / 1024.0 / 1024.0),2) + " GB";
    }
    return res;
}

ESP_SDFile::ESP_SDFile(const char * name, const char * filename, bool isdir, size_t size)
{
    _isdir = isdir;
    _dirlist = "";
    _index = -1;
    _filename = filename;
    _name = name;
    _lastwrite  = 0;
    _iswritemode = false;
    _size = size;
}

ESP_SDFile::~ESP_SDFile()
{
    //log_esp3d("Destructor %s index %d",(_isdir)?"Dir":"File", _index);
}

ESP_SDFile::operator bool() const
{
    if ((_index != -1) || (_filename.length() > 0)) {
        //log_esp3d("Bool yes %d %d",_index,  _filename.length());
        return true;
    } else {
        return false;
    }
}

bool ESP_SDFile::isOpen()
{
    return !(_index == -1);
}

const char* ESP_SDFile::name() const
{
    return _name.c_str();
}

const char* ESP_SDFile::filename() const
{
    return _filename.c_str();
}

bool ESP_SDFile::isDirectory()
{
    return _isdir;
}

size_t ESP_SDFile::size()
{
    return _size;
}

time_t ESP_SDFile::getLastWrite()
{
    return _lastwrite;
}

int ESP_SDFile::available()
{
    if (_index == -1 || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].available();
}

size_t ESP_SDFile::write(uint8_t i)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].write (i);
}

size_t ESP_SDFile::write(const uint8_t *buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].write (buf, size);
}

int ESP_SDFile::read()
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tSDFile_handle[_index].read();
}

size_t ESP_SDFile::read(uint8_t* buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tSDFile_handle[_index].read(buf, size);
}

void ESP_SDFile::flush()
{
    if ((_index == -1) || _isdir) {
        return;
    }
    tSDFile_handle[_index].flush();
}

ESP_SDFile& ESP_SDFile::operator=(const ESP_SDFile & other)
{
    //log_esp3d("Copy %s", other._filename.c_str());
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

bool ESP_SD::setSPISpeedDivider(uint8_t speeddivider)
{
    if (speeddivider > 0) {
        _spi_speed_divider = speeddivider;
        return true;
    }
    return false;
}

#endif //SD_DEVICE
