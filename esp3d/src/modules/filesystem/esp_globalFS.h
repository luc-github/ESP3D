/*
  esp_globalFS.h - ESP3D FS support class

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

#ifndef _ESP_GLOBAL_FS_H
#define _ESP_GLOBAL_FS_H
#include "../../include/esp3d_config.h"
#include "../../core/esp3doutput.h"
#include <time.h>
#ifdef FILESYSTEM_FEATURE
#include "esp_filesystem.h"
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
#include "esp_sd.h"
#endif //SD_DEVICE


class ESP_GBFile
{
public:
    ESP_GBFile();
    ESP_GBFile(uint8_t FS, const char *name=nullptr);
#ifdef FILESYSTEM_FEATURE
    ESP_GBFile(ESP_File & flashFile);
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    ESP_GBFile(ESP_SDFile & sdFile);
#endif //SD_DEVICE
    ~ESP_GBFile();
    operator bool() const;
    bool isDirectory();
    bool seek(uint32_t pos, uint8_t mode = ESP_SEEK_SET);
    const char* name() const;
    const char* shortname() const;
    const char* filename() const;
    void close();
    bool isOpen();
    ESP_GBFile & operator=(const ESP_GBFile & other);
#ifdef FILESYSTEM_FEATURE
    ESP_GBFile & operator=(const ESP_File & other);
#endif //FILESYSTEM_FEATURE    
#ifdef SD_DEVICE
    ESP_GBFile & operator=(const ESP_SDFile & other);
#endif //SD_DEVICE    
    size_t size();
    time_t getLastWrite();
    int available();
    size_t write(uint8_t i);
    size_t write(const uint8_t *buf, size_t size);
    int read();
    size_t read(uint8_t* buf, size_t size);
    void flush();
    ESP_GBFile openNextFile();
private:

#ifdef FILESYSTEM_FEATURE
    ESP_File _flashFile;
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    ESP_SDFile _sdFile;
#endif //SD_DEVICE 
    uint8_t _type;
    String _name;
};

class ESP_GBFS
{
public:
    static bool  accessFS(uint8_t FS);
    static void  releaseFS(uint8_t FS);
    static bool isavailable(uint8_t FS=FS_UNKNOWN);
    static uint64_t totalBytes(uint8_t FS=FS_UNKNOWN);
    static uint64_t usedBytes(uint8_t FS=FS_UNKNOWN);
    static uint64_t freeBytes(uint8_t FS=FS_UNKNOWN);
    static uint maxPathLength();
    static bool format(uint8_t FS, ESP3DOutput * output = nullptr);
    static ESP_GBFile open(const char* path, uint8_t mode = ESP_FILE_READ);
    static bool exists(const char* path);
    static bool remove(const char *path);
    static bool mkdir(const char *path);
    static bool rmdir(const char *path);
    static bool rename(const char *oldpath, const char *newpath);
    static void closeAll();
    static String & formatBytes (uint64_t bytes);
    static const char * getNextFS(bool reset = false);
    static uint8_t getFSType(const char * path);
private:
    static const char * getRealPath(const char * path);
    static uint8_t _nbFS;
    static String _rootlist[MAX_FS];
};


#endif //_ESP_GLOBAL_FS_H
