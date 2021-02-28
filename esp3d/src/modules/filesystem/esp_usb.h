/*
  esp_usb.h - ESP3D USB support class

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

#ifndef _ESP_USB_H
#define _ESP_USB_H
#include "../../include/esp3d_config.h"
#include "../../core/esp3doutput.h"
#include <time.h>

#define ESP_USB_FS_HEADER "/USB"

#define ESP_MAX_USB_OPENHANDLE 4

class ESP_USBFile
{
public:
    ESP_USBFile(void *  handle = nullptr, bool isdir =false, bool iswritemode = false, const char * path = nullptr);
    ESP_USBFile(const char * name, const char * filename, bool isdir = true, size_t size =0);
    ~ESP_USBFile();
    operator bool() const;
    bool isDirectory();
    bool seek(uint32_t pos, uint8_t mode = ESP_SEEK_SET);
    const char* name() const;
    const char* shortname() const;
    const char* filename() const;
    void close();
    bool isOpen();
    ESP_USBFile & operator=(const ESP_USBFile & other);
    size_t size();
    time_t getLastWrite();
    int available();
    size_t write(uint8_t i);
    size_t write(const uint8_t *buf, size_t size);
    int read();
    size_t read(uint8_t* buf, size_t size);
    void flush();
    ESP_USBFile openNextFile();
private:
    String _dirlist;
    bool _isdir;
    bool _iswritemode;
    int8_t _index;
    String _filename;
    String _name;
    size_t _size;
    time_t _lastwrite;
};

class ESP_SD
{
public:
    static String & formatBytes (uint64_t bytes);
    static bool begin();
    static bool  accessSD();
    static void  releaseSD();
    static void handle();
    static void end();
    static uint8_t getState(bool refresh);
    static uint8_t setState(uint8_t state);
    static uint64_t totalBytes();
    static uint64_t usedBytes();
    static uint64_t freeBytes();
    static uint maxPathLength();
    static const char * FilesystemName();
    static bool format(ESP3DOutput * output = nullptr);
    static ESP_USBFile open(const char* path, uint8_t mode = ESP_FILE_READ);
    static bool exists(const char* path);
    static bool remove(const char *path);
    static bool mkdir(const char *path);
    static bool rmdir(const char *path);
    static bool rename(const char *oldpath, const char *newpath);
    static void closeAll();
    static uint8_t getSPISpeedDivider()
    {
        return _spi_speed_divider;
    }
    static bool setSPISpeedDivider(uint8_t speeddivider);
private:
    static bool _started;
    static uint8_t _state;
    static uint8_t _spi_speed_divider;
    static bool _sizechanged;
};


#endif //_ESP_USB_H
