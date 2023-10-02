/*
littlefs_esp32_filesystem.cpp - ESP3D littlefs filesystem configuration class

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
#include "../../../include/esp3d_config.h"
#if (FILESYSTEM_FEATURE == ESP_LITTLEFS_FILESYSTEM) && defined(ARDUINO_ARCH_ESP32)
#include "../esp_filesystem.h"
#include <stack>
#include <FS.h>
#include <LittleFS.h>

extern File tFile_handle[ESP_MAX_OPENHANDLE];

bool ESP_FileSystem::begin()
{
    _started = LittleFS.begin(true);
    return _started;
}

void ESP_FileSystem::end()
{
    LittleFS.end();
    _started = false;
}

size_t ESP_FileSystem::freeBytes()
{
    return (LittleFS.totalBytes() - LittleFS.usedBytes());
}

size_t ESP_FileSystem::totalBytes()
{
    return LittleFS.totalBytes();
}

size_t ESP_FileSystem::usedBytes()
{
    return LittleFS.usedBytes();
}

uint ESP_FileSystem::maxPathLength()
{
    return 32;
}

bool ESP_FileSystem::rename(const char *oldpath, const char *newpath)
{
    return LittleFS.rename(oldpath,newpath);
}

const char * ESP_FileSystem::FilesystemName()
{
    return "LittleFS";
}

bool ESP_FileSystem::format()
{
    bool res = LittleFS.format();
    if (res) {
        res = begin();
    }
    return res;
}

ESP_File ESP_FileSystem::open(const char* path, uint8_t mode)
{
    log_esp3d("open %s", path);
    //do some check
    if(((strcmp(path,"/") == 0) && ((mode == ESP_FILE_WRITE) || (mode == ESP_FILE_APPEND))) || (strlen(path) == 0)) {
        log_esp3d("reject  %s", path);
        return ESP_File();
    }
    // path must start by '/'
    if (path[0] != '/') {
        log_esp3d("%s is invalid path", path);
        return ESP_File();
    }
    File tmp = LittleFS.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
    if(tmp) {
        ESP_File esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
        log_esp3d("%s is a %s", path,tmp.isDirectory()?"Dir":"File");
        return esptmp;
    } else {
        log_esp3d("open %s failed", path);
        return  ESP_File();
    }
}

bool ESP_FileSystem::exists(const char* path)
{
    bool res = false;
    //root should always be there if started
    if (strcmp(path, "/") == 0) {
        return _started;
    }
    String p = path;
    if(p[0]!='/') {
        p="/"+p;
    }
    res = LittleFS.exists(p);
    if (!res) {
        ESP_File root = ESP_FileSystem::open(p.c_str(), ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
        root.close();
    }
    return res;
}

bool ESP_FileSystem::remove(const char *path)
{
    String p = path;
    if(p[0]!='/') {
        p="/"+p;
    }
    return LittleFS.remove(p.c_str());
}

bool ESP_FileSystem::mkdir(const char *path)
{
    String p = path;
    if(p[0]!='/') {
        p="/"+p;
    }
    if (p[p.length()-1] == '/') {
        if (p!="/") {
            p.remove(p.length()-1);
        }
    }
    return LittleFS.mkdir(p);
}

bool ESP_FileSystem::rmdir(const char *path)
{
    String p = path;
    if (!p.startsWith("/")) {
        p = '/'+p;
    }
    if (p!= "/") {
        if (p.endsWith("/")) {
            p.remove(p.length()-1);
        }
    }
    if (!exists(p.c_str())) {
        return false;
    }
    bool res = true;
    std::stack <String> pathlist;
    pathlist.push(p);
    while (pathlist.size() > 0 && res) {
        File dir = LittleFS.open(pathlist.top().c_str());
        File f = dir.openNextFile();
        bool candelete = true;
        while (f && res) {
            if (f.isDirectory()) {
                candelete = false;
                String newdir = pathlist.top()+ '/';
                newdir+= f.name();
                pathlist.push(newdir);
                f.close();
                f = File();
            } else {
                String filepath = pathlist.top()+ '/';
                filepath+= f.name();
                f.close();
                if (!LittleFS.remove(filepath.c_str())) {
                    res = false;
                }
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.top() !="/") {
                res = LittleFS.rmdir(pathlist.top().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.size());
    return res;
}

void ESP_FileSystem::closeAll()
{
    for (uint8_t i = 0; i < ESP_MAX_OPENHANDLE; i++) {
        tFile_handle[i].close();
        tFile_handle[i] = File();
    }
}

ESP_File::ESP_File(void* handle, bool isdir, bool iswritemode, const char * path)
{
    _isdir = isdir;
    _dirlist = "";
    _isfakedir = false;
    _index = -1;
    _filename = "";
    _name = "";
    _lastwrite = 0;
    _iswritemode = iswritemode;
    _size = 0;
    if (!handle) {
        log_esp3d("No handle");
        return ;
    }
    bool set =false;
    for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) && !set; i++) {
        if (!tFile_handle[i]) {
            tFile_handle[i] = *((File*)handle);
            //filename
            _filename = tFile_handle[i].path();
            //name
            if (_filename == "/") {
                _name = "/";
            } else {
                _name = tFile_handle[i].name();
                if (_name[0] == '/') {
                    _name.remove( 0, 1);
                }
                int pos = _name.lastIndexOf('/');
                if (pos != -1) {
                    _name.remove( 0, pos+1);
                }
            }
            //size
            _size = tFile_handle[i].size();
            //time
            _lastwrite =  tFile_handle[i].getLastWrite();
            _index = i;
            log_esp3d("Opening File at index %d",_index);
            log_esp3d("name: %s", _name.c_str());
            log_esp3d("filename: %s", _filename.c_str());
            log_esp3d("path: %s", tFile_handle[i].path());
            set = true;
        } else {
            log_esp3d("File %d busy", i);
            log_esp3d("%s", tFile_handle[i].name());
        }
    }
    if(!set) {
        log_esp3d("No handle available");
#if defined(ESP_DEBUG_FEATURE)
        for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) ; i++) {
            log_esp3d("%s", tFile_handle[i].name());
        }
#endif
    }
}

void ESP_File::close()
{
    if (_index != -1) {
        log_esp3d("Closing File at index %d", _index);
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = LittleFS.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                _lastwrite = ftmp.getLastWrite();
                ftmp.close();
            } else {
                log_esp3d("Error opening %s", _filename.c_str());
            }
        }
        tFile_handle[_index] = File();
        _index = -1;
    }
}
bool ESP_File::seek(uint32_t pos, uint8_t mode)
{
    return tFile_handle[_index].seek(pos, (SeekMode)mode);
}

ESP_File  ESP_File::openNextFile()
{
    if ((_index == -1) || !_isdir) {
        log_esp3d("openNextFile %d failed", _index);
        return ESP_File();
    }
    File tmp = tFile_handle[_index].openNextFile();
    while (tmp) {
        log_esp3d("tmp name :%s %s", tmp.name(), (tmp.isDirectory())?"isDir":"isFile");
        ESP_File esptmp(&tmp, tmp.isDirectory());
        esptmp.close();
        return esptmp;
    }
    return  ESP_File();
}


#endif //ESP_LITTLEFS_FILESYSTEM
