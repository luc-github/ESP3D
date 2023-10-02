/*
  esp_filesystem.h - ESP3D filesystem configuration class

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

#ifndef _ESP_FILESYSTEM_H
#define _ESP_FILESYSTEM_H
#include "../../include/esp3d_config.h"
#include <time.h>

#define ESP_FLASH_FS_HEADER "/FS"

#define ESP_MAX_OPENHANDLE 4

class ESP_File
{
public:
    ESP_File(void *  handle = nullptr, bool isdir =false, bool iswritemode = false, const char * path = nullptr);
    ESP_File(const char * name, const char * filename, bool isdir = true, size_t size =0);
    ~ESP_File();
    operator bool() const;
    bool isDirectory();
    bool seek(uint32_t pos, uint8_t mode = ESP_SEEK_SET);
    const char* name() const;
    const char* filename() const;
    void close();
    bool isOpen();
    ESP_File & operator=(const ESP_File & other);
    size_t size();
    time_t getLastWrite();
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
    time_t _lastwrite;
    uint64_t _timeout;
};

class ESP_FileSystem
{
public:
    static String & formatBytes (uint64_t bytes);
    static bool begin();
    static bool  accessFS(uint8_t FS = FS_FLASH);
    static void  releaseFS(uint8_t FS = FS_FLASH);
    static void end();
    static size_t totalBytes();
    static size_t usedBytes();
    static size_t freeBytes();
    static uint maxPathLength();
    static size_t max_update_size();
    static const char * FilesystemName();
    static bool format();
    static ESP_File open(const char* path, uint8_t mode = ESP_FILE_READ);
    static bool exists(const char* path);
    static bool remove(const char *path);
    static bool mkdir(const char *path);
    static bool rmdir(const char *path);
    static bool rename(const char *oldpath, const char *newpath);
    static void closeAll();
    static bool started()
    {
        return _started;
    }
    static uint8_t getFSType(const char * path=nullptr);
private:
    static bool _started;
};


#endif //ESP_FILESYSTEM_H
