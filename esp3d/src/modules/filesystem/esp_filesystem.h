/*
  esp_filesystem.h - ESP3D filesystem configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

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
*/

#define ESP_SPIFFS_FILESYSTEM       0
#define ESP_FAT_FILESYSTEM          1
#define ESP_LITTLEFS_FILESYSTEM     2

#ifndef _ESP_FILESYSTEM_H
#define _ESP_FILESYSTEM_H
#include "../../include/esp3d_config.h"
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
#include <time.h>
#endif //FILESYSTEM_TIMESTAMP_FEATURE
#define ESP_FILE_READ       0
#define ESP_FILE_WRITE      1
#define ESP_FILE_APPEND     2

class ESP_File
{
public:
    ESP_File(void *  handle = nullptr, bool isdir =false, bool iswritemode = false, const char * path = nullptr);
    ESP_File(const char * name, const char * filename, bool isdir = true);
    ~ESP_File();
    operator bool() const;
    bool isDirectory();
    const char* name() const;
    const char* filename() const;
    void close();
    bool isOpen();
    ESP_File & operator=(const ESP_File & other);
    size_t size();
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
    time_t getLastWrite();
#endif //FILESYSTEM_TIMESTAMP_FEATURE
    int available();
    size_t write(uint8_t i);
    size_t write(const uint8_t *buf, size_t size);
    int read();
    size_t read(uint8_t* buf, size_t size);
    void flush();
    ESP_File openNextFile();
private:
    String _dirlist;
    bool _isdir;
    bool _isfakedir;
    bool _iswritemode;
    int8_t _index;
    String _filename;
    String _name;
    size_t _size;
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
    time_t _lastwrite;
#endif //FILESYSTEM_TIMESTAMP_FEATURE
};

class ESP_FileSystem
{
public:
    static String & formatBytes (uint32_t bytes);
    ESP_FileSystem();
    ~ESP_FileSystem();
    static bool begin();
    static void end();
    static size_t totalBytes();
    static size_t usedBytes();
    static size_t max_update_size();
    static const char * FilesystemName();
    static bool format();
    static ESP_File open(const char* path, uint8_t mode = ESP_FILE_READ);
    static bool exists(const char* path);
    static bool remove(const char *path);
    static bool mkdir(const char *path);
    static bool rmdir(const char *path);
    static void closeAll();
private:
};


#endif //ESP_FILESYSTEM_H
