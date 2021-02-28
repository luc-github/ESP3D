/*
  esp_usb.cpp - ESP3D USB support class

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
#ifdef USB_DEVICE
#include "esp_usb.h"
#include "../../core/genLinkedList.h"
#include <time.h>

//File tUSBFile_handle[ESP_MAX_SD_OPENHANDLE];


bool ESP_USB::_started = false;
uint8_t ESP_USB::_state = ESP_USBCARD_NOT_PRESENT;
uint8_t ESP_USB::_spi_speed_divider = 1;
bool ESP_USB::_sizechanged = true;
uint8_t ESP_USB::setState(uint8_t flag)
{
    _state =  flag;
    return _state;
}

bool  ESP_USB::accessSD()
{
    bool res = false;
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
    //need to send the current state to avoid
    res =  (digitalRead(ESP_FLAG_SHARED_SD_PIN) == ESP_FLAG_SHARED_SD_VALUE);
    if (!res) {
        digitalWrite(ESP_FLAG_SHARED_SD_PIN, ESP_FLAG_SHARED_SD_VALUE);
    }
#endif //SD_DEVICE_CONNECTION == ESP_SHARED_SD 
    return res;
}
void  ESP_USB::releaseSD()
{
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
    digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
#endif //SD_DEVICE_CONNECTION == ESP_SHARED_SD 
}


void ESP_USB::handle()
{

}

//helper to format size to readable string
String & ESP_USB::formatBytes (uint64_t bytes)
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

ESP_USBFile::ESP_USBFile(const char * name, const char * filename, bool isdir, size_t size)
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

ESP_USBFile::~ESP_USBFile()
{
    //log_esp3d("Destructor %s index %d",(_isdir)?"Dir":"File", _index);
}

ESP_USBFile::operator bool() const
{
    if ((_index != -1) || (_filename.length() > 0)) {
        //log_esp3d("Bool yes %d %d",_index,  _filename.length());
        return true;
    } else {
        return false;
    }
}

bool ESP_USBFile::isOpen()
{
    return !(_index == -1);
}

const char* ESP_USBFile::name() const
{
    return _name.c_str();
}

const char* ESP_USBFile::filename() const
{
    return _filename.c_str();
}

bool ESP_USBFile::isDirectory()
{
    return _isdir;
}

size_t ESP_USBFile::size()
{
    return _size;
}

time_t ESP_USBFile::getLastWrite()
{
    return _lastwrite;
}

int ESP_USBFile::available()
{
    if (_index == -1 || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].available();
}

size_t ESP_USBFile::write(uint8_t i)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].write (i);
}

size_t ESP_USBFile::write(const uint8_t *buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tSDFile_handle[_index].write (buf, size);
}

int ESP_USBFile::read()
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tSDFile_handle[_index].read();
}

size_t ESP_USBFile::read(uint8_t* buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tSDFile_handle[_index].read(buf, size);
}

void ESP_USBFile::flush()
{
    if ((_index == -1) || _isdir) {
        return;
    }
    tSDFile_handle[_index].flush();
}

ESP_USBFile& ESP_USBFile::operator=(const ESP_USBFile & other)
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

bool ESP_USB::setSPISpeedDivider(uint8_t speeddivider)
{
    if (speeddivider > 0) {
        _spi_speed_divider = speeddivider;
        return true;
    }
    return false;
}

#endif //SD_DEVICE
