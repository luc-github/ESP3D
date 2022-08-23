/*
littlefs_esp8266_filesystem.cpp - ESP3D littlefs filesystem configuration class

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
#include "../../../include/esp3d_config.h"
#if (FILESYSTEM_FEATURE == ESP_LITTLEFS_FILESYSTEM) && defined(ARDUINO_ARCH_ESP8266)
#include "../esp_filesystem.h"
#include <stack>
#include <FS.h>
#include <LittleFS.h>

Dir tDir_handle[ESP_MAX_OPENHANDLE];
extern File tFile_handle[ESP_MAX_OPENHANDLE];

bool ESP_FileSystem::begin()
{
    _started = LittleFS.begin();
    return _started;
}
void ESP_FileSystem::end()
{
    _started = false;
    LittleFS.end();
}

size_t ESP_FileSystem::freeBytes()
{
    return totalBytes() - usedBytes();
}

size_t ESP_FileSystem::totalBytes()
{
    fs::FSInfo info;
    LittleFS.info (info);
    return info.totalBytes;
}

size_t ESP_FileSystem::usedBytes()
{
    fs::FSInfo info;
    LittleFS.info (info);
    return info.usedBytes;
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
    //do some check
    if(((strcmp(path,"/") == 0) && ((mode == ESP_FILE_WRITE) || (mode == ESP_FILE_APPEND))) || (strlen(path) == 0)) {
        return ESP_File();
    }
    // path must start by '/'
    if (path[0] != '/') {
        return ESP_File();
    }
    File ftmp = LittleFS.open(path, (mode == ESP_FILE_READ)?"r":(mode == ESP_FILE_WRITE)?"w":"a");
    if(ftmp) {
        log_esp3d("Success openening: %s", path);
        if (ftmp.isFile()) {
            log_esp3d("It is a file");
            ESP_File esptmp(&ftmp, false,(mode == ESP_FILE_READ)?false:true, path);
            return esptmp;
        }
        if (ftmp.isDirectory()) {
            log_esp3d("It is a Directory");
        }
        ftmp.close();
    }
    log_esp3d("Opening as Directory");
    Dir dtmp = LittleFS.openDir(path);
    ESP_File esptmp(&dtmp, true, false, path);
    return esptmp;
}

bool ESP_FileSystem::exists(const char* path)
{
    //root should always be there if started
    if (strcmp(path, "/") == 0) {
        return _started;
    }
    String spath = path;
    spath.trim();
    if (spath[spath.length()-1] == '/') {
        if (spath!="/") {
            spath.remove(spath.length()-1);
        }
    }
    return LittleFS.exists(spath.c_str());
}

bool ESP_FileSystem::remove(const char *path)
{
    String p = path;
    if(p[0]!='/') {
        p="/"+p;
    }
    log_esp3d("delete %s", p.c_str());
    return LittleFS.remove(p);
}

bool ESP_FileSystem::mkdir(const char *path)
{
    String spath = path;
    spath.trim();
    if (spath[spath.length()-1] == '/') {
        if (spath!="/") {
            spath.remove(spath.length()-1);
        }
    }
    if (spath[0]!='/') {
        spath = "/"+spath;
    }
    return LittleFS.mkdir(spath.c_str());
}

bool ESP_FileSystem::rmdir(const char *path)
{
    String p = path;
    if (!p.endsWith("/")) {
        p+= '/';
    }
    if (!p.startsWith("/")) {
        p = '/'+p;
    }
    if (!exists(p.c_str())) {
        return false;
    }
    bool res = true;
    std::stack <String> pathlist;
    pathlist.push(p);
    while (pathlist.size() > 0) {

        bool candelete = true;
        Dir dir = LittleFS.openDir(pathlist.top().c_str());
        while (dir.next()) {
            if (dir.isDirectory()) {
                candelete = false;
                String newdir = pathlist.top() + dir.fileName() + "/";
                pathlist.push(newdir);
            } else {
                String filepath = pathlist.top()+ '/';
                filepath+= dir.fileName();
                log_esp3d("remove %s", filepath.c_str());
                LittleFS.remove(filepath.c_str());
            }
        }
        if (candelete) {
            if (pathlist.top() !="/") {
                if (LittleFS.exists(pathlist.top().c_str())) {
                    res = LittleFS.rmdir(pathlist.top().c_str());
                }
            }
            pathlist.pop();
        }
    }
    return res;
}

void ESP_FileSystem::closeAll()
{
    for (uint8_t i = 0; i < ESP_MAX_OPENHANDLE; i++) {
        tDir_handle[i] = Dir();
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
    if (_isdir) {
        for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) && !set; i++) {
            if (tDir_handle[i].fileName().length() == 0) {
                tDir_handle[i] = *((Dir *)handle);
                _index = i;
                //Path = filename
                if (path) {
                    _filename = path;
                    _filename.trim();
                    if (!((_filename[_filename.length()-1] == '/') || (_filename == "/"))) {
                        _filename+="/";
                    }
                    //Name
                    if (_filename == "/") {
                        _name = "/";
                    } else {
                        _name.remove( 0, _name.lastIndexOf('/')+1);
                    }
                }
                log_esp3d("Dir: %s index: %d", _name.c_str(), _index);
                log_esp3d("name: %s", _name.c_str());
                log_esp3d("filename: %s", _filename.c_str());
                set = true;
            }
        }
        return;
    }

    for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) && !set; i++) {
        if (!tFile_handle[i]) {
            tFile_handle[i] = *((File*)handle);
            //filename
            _filename = tFile_handle[i].fullName();
            //name
            _name = tFile_handle[i].name();
            //size
            _size = tFile_handle[i].size();
            //time
            _lastwrite =  tFile_handle[i].getLastWrite();
            _index = i;
            log_esp3d("Opening File at index %d",_index);
            log_esp3d("name: %s", _name.c_str());
            log_esp3d("filename: %s", _filename.c_str());
            set = true;
        }
    }
}

bool ESP_File::seek(uint32_t pos, uint8_t mode)
{
    return tFile_handle[_index].seek(pos, (SeekMode)mode);
}

void ESP_File::close()
{
    if (_index != -1) {
        if (_isdir) {
            log_esp3d("Closing Dir at index %d", _index);
            tDir_handle[_index] = Dir();
            _index = -1;
            return;
        }
        log_esp3d("Closing File at index %d", _index);
        log_esp3d("Size is %d", tFile_handle[_index].size());
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            log_esp3d("Updating Size of %s",_filename.c_str());
            File ftmp = LittleFS.open(_filename.c_str(), "r");
            if (ftmp) {
                _size = ftmp.size();
                log_esp3d("Updating Size to %d", ftmp.size());
                _lastwrite = ftmp.getLastWrite();
                ftmp.close();
            }
        }
        _index = -1;
    }
}

ESP_File  ESP_File::openNextFile()
{
    if ((_index == -1) || !_isdir) {
        log_esp3d("openNextFile failed");
        return ESP_File();
    }
    if(tDir_handle[_index].next()) {
        String name = tDir_handle[_index].fileName();
        log_esp3d("Getting next file from %s", _filename.c_str());
        log_esp3d("name :%s %s", name.c_str(), (tDir_handle[_index].isDirectory())?"isDir":"isFile");
        String  s = _filename;
        if(s[s.length()-1]!='/') {
            s+="/";
        }
        s+=name.c_str();
        if (tDir_handle[_index].isFile()) {
            ESP_File esptmp(name.c_str(), s.c_str(), false, tDir_handle[_index].fileSize()) ;
            return esptmp;
        } else {
            log_esp3d("Found dir  name: %s filename:%s",name.c_str(), s.c_str());
            ESP_File esptmp = ESP_File(name.c_str(), s.c_str());
            return esptmp;
        }

    }
    return  ESP_File();
}

#endif //ESP_LITTLEFS_FILESYSTEM
