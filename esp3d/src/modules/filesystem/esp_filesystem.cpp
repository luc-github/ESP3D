/*
  esp_filesystem.cpp - ESP3D filesystem configuration class

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
#ifdef FILESYSTEM_FEATURE
#include "esp_filesystem.h"
#include <time.h>
#include <FS.h>
#ifdef ARDUINO_ARCH_ESP32
#include <esp_ota_ops.h>
#endif //ARDUINO_ARCH_ESP32

#define ESP_MAX_OPENHANDLE 4
File tFile_handle[ESP_MAX_OPENHANDLE];

bool ESP_FileSystem::_started = false;

uint8_t ESP_FileSystem::getFSType(const char * path)
{
    (void)path;
    return FS_FLASH;
}

//helper to format size to readable string
String & ESP_FileSystem::formatBytes (uint64_t bytes)
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

bool  ESP_FileSystem::accessFS(uint8_t FS)
{
    (void)FS;
    if (!_started) {
        _started = begin();
    }
    return _started;
}
void  ESP_FileSystem::releaseFS(uint8_t FS)
{
    //nothing to do
    (void)FS;
}

size_t ESP_FileSystem::max_update_size()
{
    size_t  flashsize = 0;
#if defined (ARDUINO_ARCH_ESP8266)
    flashsize = ESP.getFlashChipSize();
    //if higher than 1MB or not (no more support for 512KB flash)
    if (flashsize <= 1024 * 1024) {
        flashsize = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    } else {
        flashsize = flashsize - ESP.getSketchSize()-totalBytes()-1024;
        //max OTA partition is 1019Kb
        if (flashsize > 1024 * 1024) {
            flashsize = (1024 * 1024) - 1024;
        }
    }
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    //Is OTA available ?
    const esp_partition_t* mainpartition = esp_ota_get_running_partition();
    if (mainpartition) {
        const esp_partition_t* partition = esp_ota_get_next_update_partition(mainpartition);
        if (partition) {
            const esp_partition_t* partition2 = esp_ota_get_next_update_partition(partition);
            if (partition2 && (partition->address!=partition2->address)) {
                flashsize = partition2->size;
            }
        }
    }
#endif //ARDUINO_ARCH_ESP32
    return flashsize;
}

ESP_File::ESP_File(const char * name, const char * filename, bool isdir, size_t size)
{
    _isdir = isdir;
    _dirlist = "";
    _isfakedir = isdir;
    _index = -1;
    _filename = filename;
    _name = name;
    _lastwrite = 0;
    _iswritemode = false;
    _size = size;
}

ESP_File::~ESP_File()
{
    //log_esp3d("Destructor %s index %d",(_isdir)?"Dir":"File", _index);
}

ESP_File::operator bool() const
{
    if ((_index != -1) || (_filename.length() > 0)) {
        //log_esp3d("Bool yes %d %d",_index,  _filename.length());
        return true;
    } else {
        return false;
    }
}

bool ESP_File::isOpen()
{
    return !(_index == -1);
}

const char* ESP_File::name() const
{
    return _name.c_str();
}

const char* ESP_File::filename() const
{
    return _filename.c_str();
}

bool ESP_File::isDirectory()
{
    return _isdir;
}

size_t ESP_File::size()
{
    return _size;
}

time_t ESP_File::getLastWrite()
{
    return _lastwrite;
}

int ESP_File::available()
{
    if (_index == -1 || _isdir) {
        return 0;
    }
    return tFile_handle[_index].available();
}

size_t ESP_File::write(uint8_t i)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tFile_handle[_index].write (i);
}

size_t ESP_File::write(const uint8_t *buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tFile_handle[_index].write (buf, size);
}

int ESP_File::read()
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tFile_handle[_index].read();
}

size_t ESP_File::read(uint8_t* buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tFile_handle[_index].read(buf, size);
}

void ESP_File::flush()
{
    if ((_index == -1) || _isdir) {
        return;
    }
    tFile_handle[_index].flush();
}

ESP_File& ESP_File::operator=(const ESP_File & other)
{
    //log_esp3d("Copy %s", other._filename.c_str());
    _isdir = other._isdir;
    _isfakedir = other._isfakedir;
    _index = other._index;
    _filename = other._filename;
    _name = other._name;
    _size = other._size;
    _iswritemode = other._iswritemode;
    _dirlist = other._dirlist;
    _lastwrite = other._lastwrite;
    return *this;
}

#endif //FILESYSTEM_FEATURE
