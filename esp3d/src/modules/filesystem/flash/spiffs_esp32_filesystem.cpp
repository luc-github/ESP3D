/*
  spiffs_esp32_filesystem.cpp - ESP3D filesystem configuration class

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
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM) && defined(ARDUINO_ARCH_ESP32)
#include "../esp_filesystem.h"
#include <stack>
#include <FS.h>
#include <SPIFFS.h>
extern File tFile_handle[ESP_MAX_OPENHANDLE];

bool ESP_FileSystem::begin()
{
    _started = SPIFFS.begin(true);
    return _started;
}
void ESP_FileSystem::end()
{
    _started = false;
    SPIFFS.end();
}

size_t ESP_FileSystem::freeBytes()
{
    return totalBytes() - usedBytes();
}

size_t ESP_FileSystem::totalBytes()
{
    return SPIFFS.totalBytes();
}

size_t ESP_FileSystem::usedBytes()
{
    return SPIFFS.usedBytes();
}

uint ESP_FileSystem::maxPathLength()
{
    return 32;
}

bool ESP_FileSystem::rename(const char *oldpath, const char *newpath)
{
    return SPIFFS.rename(oldpath,newpath);
}

const char * ESP_FileSystem::FilesystemName()
{
    return "SPIFFS";
}

bool ESP_FileSystem::format()
{
    bool res = SPIFFS.format();
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
    //TODO add support if path = /DIR1/ <- with last /
    File tmp = SPIFFS.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
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
    String spath = path;
    spath.trim();
    if (spath[spath.length()-1] == '/') {
        if (spath!="/") {
            spath.remove(spath.length()-1);
        }
    }
    res = SPIFFS.exists(spath.c_str());
    if (!res) {
        String newpath = spath;
        if (newpath[newpath.length()-1] != '/') {
            newpath+="/";
        }
        newpath+=".";
        log_esp3d("Check %s", newpath.c_str());
        res = SPIFFS.exists(newpath);
        if (!res) {
            ESP_File f = ESP_FileSystem::open(path, ESP_FILE_READ);
            if (f) {
                //Check directories
                ESP_File sub = f.openNextFile();
                if (sub) {
                    sub.close();
                    res = true;
                }
                f.close();
            }
        }
    }
    return res;
}

bool ESP_FileSystem::remove(const char *path)
{
    String p = path;
    if(p[0]!='/') {
        p="/"+p;
    }
    return SPIFFS.remove(p);
}

bool ESP_FileSystem::mkdir(const char *path)
{
    //Use file named . to simulate directory
    String p = path;
    if (p[p.length()-1] != '/') {
        p+="/";
    }
    p+=".";
    log_esp3d("Dir create : %s", p.c_str());
    ESP_File f = open(p.c_str(), ESP_FILE_WRITE);
    if (f) {
        f.close();
        return true;
    } else {
        return false;
    }
}

bool ESP_FileSystem::rmdir(const char *path)
{
    String spath = path;
    spath.trim();
    if (!spath.startsWith("/")) {
        spath = '/'+spath;
    }
    if (spath!= "/") {
        if (spath.endsWith("/")) {
            spath.remove(spath.length()-1);
        }
    }
    log_esp3d("Deleting : %s",spath.c_str());
    File ftmp = SPIFFS.open(spath.c_str());
    if (ftmp) {
        File pfile = ftmp.openNextFile();
        while (pfile) {
            log_esp3d("File: %s",pfile.path());
            if (!SPIFFS.remove(pfile.path())) {
                pfile.close();
                return false;
            }
            pfile.close();
            pfile = ftmp.openNextFile();
        }
        ftmp.close();
        return true;
    } else {
        return false;
    }
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
                _name =  tFile_handle[i].name();
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
        log_esp3d("Closing File at index %d", _index);
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = SPIFFS.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
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
            log_esp3d("file name:%s name: %s %s  sub:%s root:%s", esptmp.filename(), esptmp.name(), (esptmp.isDirectory())?"isDir":"isFile", sub.c_str(), _filename.c_str());
            String tag = "*" + sub + "*";
            //test if already in directory list
            if (_dirlist.indexOf(tag) == -1) {//not in list so add it and return the info
                _dirlist+= tag;
                String fname = _filename.substring(0,_filename.length()-1) + sub + "/.";
                log_esp3d("Found dir # name: %s filename:%s", sub.c_str(), fname.c_str());
                if (sub == ".") {
                    log_esp3d("Dir tag, ignore it");
                    tmp = tFile_handle[_index].openNextFile();
                } else {
                    esptmp = ESP_File(sub.c_str(), fname.c_str());
                    return esptmp;
                }
            } else { //already in list so ignore it
                log_esp3d("Dir name: %s already in list", sub.c_str());
                tmp = tFile_handle[_index].openNextFile();
            }
        } else { //is file
            log_esp3d("file name:%s name: %s %s  sub:%s root:%s", esptmp.filename(), esptmp.name(), (esptmp.isDirectory())?"isDir":"isFile", sub.c_str(), _filename.c_str());
            if (sub == ".") {
                log_esp3d("Dir tag, ignore it");
                tmp = tFile_handle[_index].openNextFile();
            } else {
                log_esp3d("Found file #  name: %s filename:%s", esptmp.filename(), esptmp.name());
                return esptmp;
            }
        }
    }
    return  ESP_File();
}


#endif //ESP_SPIFFS_FILESYSTEM
