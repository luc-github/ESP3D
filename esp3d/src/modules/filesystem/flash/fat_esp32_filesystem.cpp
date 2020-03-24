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
    return FFat.format();
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
    if (mode != ESP_FILE_READ) {
        //check container exists
        String p = path;
        p.remove(p.lastIndexOf('/') +1);
        if (!exists(p.c_str())) {
            log_esp3d("Error opening: %s", path);
            return ESP_File();
        }
    }
    File tmp = FFat.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
    ESP_File esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
    return esptmp;
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
    }
    return res;
}

bool ESP_FileSystem::remove(const char *path)
{
    return FFat.remove(path);
}

bool ESP_FileSystem::mkdir(const char *path)
{
    return FFat.mkdir(path);
}

bool ESP_FileSystem::rmdir(const char *path)
{
    if (!exists(path)) {
        return false;
    }
    bool res = true;
    GenLinkedList<String > pathlist;
    String p = path;
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
        return ;
    }
    bool set =false;
    for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) && !set; i++) {
        if (!tFile_handle[i]) {
            tFile_handle[i] = *((File*)handle);
            //filename
            _filename = tFile_handle[i].name();

            //if root
            if (_filename == "/") {
                _filename = "/.";
            }
            if (_isdir) {
                if (_filename[_filename.length()-1] != '.') {
                    if (_filename[_filename.length()-2] != '/') {
                        _filename+="/";
                    }
                    _filename+=".";
                }
            }
            //name
            if (_filename == "/.") {
                _name = "/";
            } else {
                _name = _filename;
                if (_name.endsWith("/.")) {
                    _name.remove( _name.length() - 2,2);
                    _isfakedir = true;
                    _isdir = true;
                }
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
            //log_esp3d("Opening File at index %d",_index);
            set = true;
        }
    }
}

void ESP_File::close()
{
    if (_index != -1) {
        //log_esp3d("Closing File at index %d", _index);
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = FFat.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                _lastwrite = ftmp.getLastWrite();
                ftmp.close();
            }
        }
        tFile_handle[_index] = File();
        //log_esp3d("Closing File at index %d",_index);
        _index = -1;
    }
}

ESP_File  ESP_File::openNextFile()
{
    if ((_index == -1) || !_isdir) {
        log_esp3d("openNextFile failed");
        return ESP_File();
    }
    File tmp = tFile_handle[_index].openNextFile();
    while (tmp) {
        log_esp3d("tmp name :%s %s", tmp.name(), (tmp.isDirectory())?"isDir":"isFile");
        ESP_File esptmp(&tmp, tmp.isDirectory());
        esptmp.close();
        String sub = esptmp.filename();
        sub.remove(0,_filename.length()-1);
        int pos = sub.indexOf("/");
        if (pos!=-1) {
            //is subdir
            sub = sub.substring(0,pos);
            //log_esp3d("file name:%s name: %s %s  sub:%s root:%s", esptmp.filename(), esptmp.name(), (esptmp.isDirectory())?"isDir":"isFile", sub.c_str(), _filename.c_str());
            String tag = "*" + sub + "*";
            //test if already in directory list
            if (_dirlist.indexOf(tag) == -1) {//not in list so add it and return the info
                _dirlist+= tag;
                String fname = _filename.substring(0,_filename.length()-1) + sub + "/.";
                //log_esp3d("Found dir  name: %s filename:%s", sub.c_str(), fname.c_str());
                esptmp = ESP_File(sub.c_str(), fname.c_str());
                return esptmp;
            } else { //already in list so ignore it
                //log_esp3d("Dir name: %s already in list", sub.c_str());
                tmp = tFile_handle[_index].openNextFile();
            }
        } else { //is file
            //log_esp3d("file name:%s name: %s %s  sub:%s root:%s", esptmp.filename(), esptmp.name(), (esptmp.isDirectory())?"isDir":"isFile", sub.c_str(), _filename.c_str());
            if (sub == ".") {
                //log_esp3d("Dir tag, ignore it");
                tmp = tFile_handle[_index].openNextFile();
            } else {
                return esptmp;
            }
        }

    }
    return  ESP_File();
}


#endif //ESP_FAT_FILESYSTEM
