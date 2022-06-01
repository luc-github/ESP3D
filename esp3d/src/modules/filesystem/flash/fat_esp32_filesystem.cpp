/*
fat_esp32_filesystem.cpp - ESP3D fat filesystem configuration class

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
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
#include "../esp_filesystem.h"
#include "../../../core/genLinkedList.h"
#include <FS.h>
#include "FFat.h"

extern File tFile_handle[ESP_MAX_OPENHANDLE];

bool ESP_FileSystem::begin()
{
    _started = FFat.begin();
    return _started;
}

void ESP_FileSystem::end()
{
    FFat.end();
    _started = false;
}

size_t ESP_FileSystem::freeBytes()
{
    return FFat.freeBytes();
}

size_t ESP_FileSystem::totalBytes()
{
    return FFat.totalBytes();
}

size_t ESP_FileSystem::usedBytes()
{
    return (FFat.totalBytes() - FFat.freeBytes());
}

uint ESP_FileSystem::maxPathLength()
{
    return 32;
}

bool ESP_FileSystem::rename(const char *oldpath, const char *newpath)
{
    return FFat.rename(oldpath,newpath);
}

const char * ESP_FileSystem::FilesystemName()
{
    return "FAT";
}

bool ESP_FileSystem::format()
{
    bool res = FFat.format();
    if (res) {
        res = begin();
    }
    return res;
}

ESP_File ESP_FileSystem::open(const char* path, uint8_t mode)
{
    log_esp3d("open %s as %s", path,(mode == ESP_FILE_WRITE?"write":"read") );
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
    File tmp = FFat.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
    if(tmp) {
        ESP_File esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
        log_esp3d("%s is a %s", path,tmp.isDirectory()?"Dir":"File");
        log_esp3d("path is %s and filename path is %s", path, tmp.path());
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
    res = FFat.exists(path);
    if (!res) {
        ESP_File root = ESP_FileSystem::open(path, ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
        root.close();
    }
    return res;
}

bool ESP_FileSystem::remove(const char *path)
{
    return FFat.remove(path);
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
    return FFat.mkdir(p);
}

bool ESP_FileSystem::rmdir(const char *path)
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

    if (!exists(p.c_str())) {
        return false;
    }
    bool res = true;
    GenLinkedList<String > pathlist;
    pathlist.push(p);
    while (pathlist.count() > 0) {
        File dir = FFat.open(pathlist.getLast().c_str());
        File f = dir.openNextFile();
        bool candelete = true;
        while (f) {
            if (f.isDirectory()) {
                candelete = false;
                String newdir = f.name();
                pathlist.push(newdir);
                f.close();
                f = File();
            } else {
                FFat.remove(f.name());
                f.close();
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.getLast() !="/") {
                res = FFat.rmdir(pathlist.getLast().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.count());
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
        }
    }
    if(!set) {
        log_esp3d("No handle available");
    }
}

bool ESP_File::seek(uint32_t pos, uint8_t mode)
{
    return tFile_handle[_index].seek(pos, (SeekMode)mode);
}

void ESP_File::close()
{
    if (_index != -1) {
        log_esp3d("Closing File %s at index %d", _filename.c_str(), _index);
        log_esp3d("name: %s", _name.c_str());
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            log_esp3d("Updating %s size", _filename.c_str());
            File ftmp = FFat.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                log_esp3d("Size is %d",_size);
                _lastwrite = ftmp.getLastWrite();
                ftmp.close();
            }
        }
        tFile_handle[_index] = File();
        _index = -1;
    }
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


#endif //ESP_FAT_FILESYSTEM
