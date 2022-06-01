/*
  esp_globalFS.cpp - ESP3D global FS support class

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
#if defined(GLOBAL_FILESYSTEM_FEATURE)
#include "esp_globalFS.h"
//#include "../../core/genLinkedList.h"

//to verify FS is accessible
bool ESP_GBFS::isavailable(uint8_t FS)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::started();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        return (ESP_SD::getState(true) == ESP_SDCARD_IDLE);
    }
#endif //SD_DEVICE    
    return false;
}

uint8_t ESP_GBFS::_nbFS = 0;
String ESP_GBFS::_rootlist[MAX_FS];

//helper to format size to readable string
String & ESP_GBFS::formatBytes (uint64_t bytes)
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

//depending FS
uint64_t ESP_GBFS::totalBytes(uint8_t FS)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::totalBytes();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        return ESP_SD::totalBytes();
    }
#endif //SD_DEVICE   
    return 0;
}

uint64_t ESP_GBFS::usedBytes(uint8_t FS)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::usedBytes();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        return ESP_SD::usedBytes();
    }
#endif //SD_DEVICE   
    return 0;
}

uint ESP_GBFS::maxPathLength()
{
    uint size = 255;
#ifdef FILESYSTEM_FEATURE
    if(size >32) {
        size =32;
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(size >255) {
        size =255;
    }
#endif //SD_DEVICE   
    return size;
}

bool  ESP_GBFS::accessFS(uint8_t FS)
{
    if(FS == FS_ROOT) {
        return true;
    }
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::accessFS();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        bool canAccess =  ESP_SD::accessFS();
        if(canAccess) {
            if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
                canAccess = false;
                ESP_SD::releaseFS();
            } else {
                ESP_SD::setState(ESP_SDCARD_BUSY );
                canAccess = true;
            }

        }
        return canAccess;
    }
#endif //SD_DEVICE   
    return false;
}
void  ESP_GBFS::releaseFS(uint8_t FS)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        ESP_FileSystem::releaseFS();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        ESP_SD::releaseFS();
    }
#endif //SD_DEVICE   
}

uint64_t ESP_GBFS::freeBytes(uint8_t FS)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::freeBytes();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        return ESP_SD::freeBytes();
    }
#endif //SD_DEVICE   
    return 0;
}

//Format is not always available for all FS
bool format(uint8_t FS, ESP3DOutput * output = nullptr)
{
#ifdef FILESYSTEM_FEATURE
    if(FS == FS_FLASH) {
        return ESP_FileSystem::format();
    }
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    if(FS == FS_SD) {
        return ESP_SD::format(output);
    }
#endif //SD_DEVICE 
    output->printERROR("Not available");
    return false;
}

//check type of FS according root dir
uint8_t ESP_GBFS::getFSType(const char * path)
{
    String p = path;
    p.trim();
    if (p == "/") {
        return FS_ROOT;
    }
#if defined (FILESYSTEM_FEATURE)
    if (p.startsWith(ESP_FLASH_FS_HEADER)) {
        return FS_FLASH;
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (p.startsWith(ESP_SD_FS_HEADER)) {
        return FS_SD;
    }
#endif //SD_DEVICE
    return FS_UNKNOWN;
}

const char * ESP_GBFS::getRealPath(const char * path)
{
    static String p;
    uint8_t t = getFSType(path);
    p = "";
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        p = path;
        //remove header
        p.remove(0,strlen(ESP_FLASH_FS_HEADER));
        //if nothing it is root
        if (p.length() == 0) {
            p = "/";
        }
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        p = path;
        //remove header
        p.remove(0,strlen(ESP_SD_FS_HEADER));
        //if nothing it is root
        if (p.length() == 0) {
            p = "/";
        }
    }
#endif //SD_DEVICE
    return p.c_str();
}

//path exists on / or SD or FS
bool ESP_GBFS::exists(const char* path)
{
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(path);
    if (t == FS_ROOT) {
        return true;
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        return ESP_FileSystem::exists(getRealPath(path));
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        return ESP_SD::exists(getRealPath(path));
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return false;
}

bool ESP_GBFS::remove(const char *path)
{
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(path);
    if (t == FS_ROOT) {
        return false;
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        return ESP_FileSystem::remove(getRealPath(path));
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        return ESP_SD::remove(getRealPath(path));
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return false;
}

bool ESP_GBFS::rename(const char *oldpath, const char *newpath)
{
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(oldpath);
    if (t == FS_ROOT) {
        return false;
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        return ESP_FileSystem::rename(getRealPath(oldpath), getRealPath(newpath));
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        return ESP_SD::rename(getRealPath(oldpath), getRealPath(newpath));
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return false;
}

bool ESP_GBFS::mkdir(const char *path)
{
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(path);
    if (t == FS_ROOT) {
        return false;
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        return ESP_FileSystem::mkdir(getRealPath(path));;
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        return ESP_SD::mkdir(getRealPath(path));
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return false;
}

bool ESP_GBFS::rmdir(const char *path)
{
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(path);
    if (t == FS_ROOT) {
        return false;
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        return ESP_FileSystem::rmdir(getRealPath(path));
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        return ESP_SD::rmdir(getRealPath(path));;
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return false;
}

void ESP_GBFS::closeAll()
{
    getNextFS(true);
#if defined (FILESYSTEM_FEATURE)
    ESP_FileSystem::closeAll();
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    ESP_SD::closeAll();
#endif //SD_DEVICE
}

ESP_GBFile ESP_GBFS::open(const char* path, uint8_t mode)
{
    ESP_GBFile f;
    log_esp3d("open %s", path);
#if defined (FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    uint8_t t = getFSType(path);
    log_esp3d("type %d", t);
    if ((t == FS_ROOT) &&  (mode == ESP_FILE_READ)) {
        f = ESP_GBFile(FS_ROOT,"/" );
        log_esp3d("root");
        getNextFS(true);
    }
#if defined (FILESYSTEM_FEATURE)
    if (t == FS_FLASH) {
        log_esp3d("open flash : %s", getRealPath(path));
        f = ESP_FileSystem::open(getRealPath(path), mode);
    }
#endif //FILESYSTEM_FEATURE
#if defined (SD_DEVICE)
    if (t == FS_SD) {
        log_esp3d("open SD : %s", getRealPath(path));
        f = ESP_SD::open(getRealPath(path), mode);
    }
#endif //SD_DEVICE
#endif // FILESYSTEM_FEATURE || SD_DEVICE
    return f;
}


//Default File is no file
ESP_GBFile::ESP_GBFile()
{
    _type = FS_UNKNOWN;
}

//File handle for the root
ESP_GBFile::ESP_GBFile(uint8_t FS, const char *name)
{
    _type = FS;
    _name = name;
}

//Handle for flash file
#ifdef FILESYSTEM_FEATURE
ESP_GBFile::ESP_GBFile(ESP_File & flashFile)
{
    _type = FS_FLASH;
    _flashFile = flashFile;
#ifdef SD_DEVICE
    _sdFile = ESP_SDFile();
#endif //SD_DEVICE 
}
#endif //FILESYSTEM_FEATURE

//Handle for SD file
#ifdef SD_DEVICE
ESP_GBFile::ESP_GBFile(ESP_SDFile & sdFile)
{
    _type = FS_SD;
    _sdFile = sdFile;
#ifdef FILESYSTEM_FEATURE
    _flashFile = ESP_File();
#endif //FILESYSTEM_FEATURE
}
#endif //SD_DEVICE

//Destructor
ESP_GBFile::~ESP_GBFile()
{
}

ESP_GBFile::operator bool() const
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return true;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile;
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile;
    }
#endif //SD_DEVICE 
    return false;
}

bool ESP_GBFile::seek(uint32_t pos, uint8_t mode)
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return false;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        _flashFile.seek(pos,mode);
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.seek(pos,mode);
    }
#endif //SD_DEVICE 
    return false;
}

void ESP_GBFile::close()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        //TBD
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        _flashFile.close();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.close();
    }
#endif //SD_DEVICE 
}

bool ESP_GBFile::isOpen()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return true;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.isOpen();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.isOpen();
    }
#endif //SD_DEVICE 
    return false;
}

const char* ESP_GBFile::name() const
{
    static String s;
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return _name.c_str();
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        if (strcmp(_flashFile.name(), "/") == 0) {
            s = ESP_FLASH_FS_HEADER;
            s.remove(0,1);
            return s.c_str();
        }
        return _flashFile.name();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        if (strcmp(_sdFile.name(), "/") == 0) {
            s = ESP_SD_FS_HEADER;
            s.remove(0,1);
            return s.c_str();
        }
        return _sdFile.name();
    }
#endif //SD_DEVICE
    return "";
}

const char* ESP_GBFile::filename() const
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    static String s;
    if (_type == FS_ROOT) {
        return "/";
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        s = ESP_FLASH_FS_HEADER;
        s += _flashFile.filename();
        return s.c_str();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        s = ESP_SD_FS_HEADER;
        s += s += _sdFile.filename();
        return s.c_str();
    }
#endif //SD_DEVICE
    return "";
}

bool ESP_GBFile::isDirectory()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return true;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.isDirectory();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.isDirectory();
    }
#endif //SD_DEVICE
    return false;
}

size_t ESP_GBFile::size()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return 0;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.size();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.size();
    }
#endif //SD_DEVICE
    return 0;
}

time_t ESP_GBFile::getLastWrite()
{
    if (_type == FS_FLASH) {
        return _flashFile.getLastWrite();
    }
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.getLastWrite();
    }
#endif //SD_DEVICE
    return 0;
}

int ESP_GBFile::available()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return 0;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.available();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.available();
    }
#endif //SD_DEVICE
    return 0;
}

size_t ESP_GBFile::write(uint8_t i)
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return 0;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.write(i);
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.write(i);
    }
#endif //SD_DEVICE
    return 0;
}

size_t ESP_GBFile::write(const uint8_t *buf, size_t size)
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return 0;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.write(buf, size);
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.write(buf, size);
    }
#endif //SD_DEVICE
    return 0;
}

int ESP_GBFile::read()
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return -1;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.read();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.read();
    }
#endif //SD_DEVICE
    return -1;
}

size_t ESP_GBFile::read(uint8_t* buf, size_t size)
{
#if defined(FILESYSTEM_FEATURE) || defined(SD_DEVICE)
    if (_type == FS_ROOT) {
        return -1;
    }
#endif //FILESYSTEM_FEATURE || SD_DEVICE
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.read(buf, size);
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.read(buf, size);
    }
#endif //SD_DEVICE
    return -1;
}

void ESP_GBFile::flush()
{
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        return _flashFile.flush();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        return _sdFile.flush();
    }
#endif //SD_DEVICE
    return ;
}

ESP_GBFile& ESP_GBFile::operator=(const ESP_GBFile & other)
{
#ifdef FILESYSTEM_FEATURE
    _flashFile = other._flashFile;
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
    _sdFile = other._sdFile;
#endif //SD_DEVICE 
    _type = other._type;
    _name= other._name;
    return *this;
}

#ifdef FILESYSTEM_FEATURE
ESP_GBFile & ESP_GBFile::operator=(const ESP_File & other)
{
    _flashFile = other;
#ifdef SD_DEVICE
    _sdFile = ESP_SDFile();
#endif //SD_DEVICE 
    _type = FS_FLASH;
    return *this;
}
#endif //FILESYSTEM_FEATURE    
#ifdef SD_DEVICE
ESP_GBFile & ESP_GBFile::operator=(const ESP_SDFile & other)
{
#ifdef FILESYSTEM_FEATURE
    _flashFile = ESP_File();
#endif //FILESYSTEM_FEATURE 
    _sdFile = other;
    _type = FS_SD;
    return *this;
}
#endif //SD_DEVICE   

const char * ESP_GBFS::getNextFS(bool reset)
{
    static uint8_t index = 0;
    if (reset) {
        index = 0;
        if(_nbFS == 0) {
#ifdef FILESYSTEM_FEATURE
            _rootlist[_nbFS] = ESP_FLASH_FS_HEADER;
            _nbFS++;
#endif //FILESYSTEM_FEATURE 
#ifdef SD_DEVICE
            _rootlist[_nbFS] = ESP_SD_FS_HEADER;
            _nbFS++;
#endif //SD_DEVICE 
        }
        return "";
    }
    if (index < _nbFS) {
        uint8_t i = index;
        index++;
        return _rootlist[i].c_str();
    }
    index = 0;
    return "";
}

ESP_GBFile ESP_GBFile::openNextFile()
{
    ESP_GBFile f;
    if (_type == FS_ROOT) {
        String path = ESP_GBFS::getNextFS();
        if (path.length() > 0) {
            f = ESP_GBFile(FS_ROOT, &path[1]);
        }
    }
#ifdef FILESYSTEM_FEATURE
    if (_type == FS_FLASH) {
        f = _flashFile.openNextFile();
    }
#endif //FILESYSTEM_FEATURE
#ifdef SD_DEVICE
    if (_type == FS_SD) {
        f = _sdFile.openNextFile();
    }
#endif //SD_DEVICE 
    return f;
}

#endif //GLOBAL_FILESYSTEM_FEATURE
