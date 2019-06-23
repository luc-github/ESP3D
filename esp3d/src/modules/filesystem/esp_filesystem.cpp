/*
  esp_filesystem.cpp - ESP3D filesystem configuration class

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
#include "../../include/esp3d_config.h"
#include "esp_filesystem.h"
#include "../../core/genLinkedList.h"
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
#include <time.h>
#endif //FILESYSTEM_TIMESTAMP_FEATURE
#include <FS.h>
#ifdef ARDUINO_ARCH_ESP32
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
#include <SPIFFS.h>
#endif //ESP_FAT_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
#include "FFat.h"
#endif //ESP_FAT_FILESYSTEM
#include <esp_ota_ops.h>
#endif //ARDUINO_ARCH_ESP32

#define ESP_MAX_OPENHANDLE 4

File tFile_handle[ESP_MAX_OPENHANDLE];
#ifdef ARDUINO_ARCH_ESP8266
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
Dir tDir_handle[ESP_MAX_OPENHANDLE];
#endif //ESP_SPIFFS_FILESYSTEM
#endif //ARDUINO_ARCH_ESP8266

//constructor
ESP_FileSystem::ESP_FileSystem()
{
}

//destructor
ESP_FileSystem::~ESP_FileSystem()
{
}

//helper to format size to readable string
String & ESP_FileSystem::formatBytes (uint64_t bytes)
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

bool ESP_FileSystem::begin()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
#if defined(ARDUINO_ARCH_ESP8266)
    return SPIFFS.begin();
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    return SPIFFS.begin(true);
#endif //ARDUINO_ARCH_ESP32
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return FFat.begin();
#endif //ESP_FAT_FILESYSTEM
}
void ESP_FileSystem::end()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    SPIFFS.end();
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    FFat.end();
#endif //ESP_FAT_FILESYSTEM
}

size_t ESP_FileSystem::totalBytes()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
#if defined (ARDUINO_ARCH_ESP8266)
    fs::FSInfo info;
    SPIFFS.info (info);
    return info.totalBytes;
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    return SPIFFS.totalBytes();
#endif //ARDUINO_ARCH_ESP32
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return FFat.totalBytes();
#endif //ESP_FAT_FILESYSTEM
}

size_t ESP_FileSystem::usedBytes()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
#if defined (ARDUINO_ARCH_ESP8266)
    fs::FSInfo info;
    SPIFFS.info (info);
    return info.usedBytes;
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    return SPIFFS.usedBytes();
#endif //ARDUINO_ARCH_ESP32
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return (FFat.totalBytes() - FFat.freeBytes());
#endif //ESP_FAT_FILESYSTEM
}

size_t ESP_FileSystem::max_update_size()
{
    size_t  flashsize = 0;
#if defined (ARDUINO_ARCH_ESP8266)
    flashsize = ESP.getFlashChipSize();
    //if higher than 1MB take out SPIFFS
    if (flashsize > 1024 * 1024) {
        flashsize = (1024 * 1024)-ESP.getSketchSize()-1024;
    } else {
        fs::FSInfo info;
        SPIFFS.info (info);
        flashsize = flashsize - ESP.getSketchSize()-info.totalBytes-1024;
    }
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    //Is OTA available ?
    if (esp_ota_get_running_partition()) {
        flashsize = ESP.getFreeSketchSpace() + ESP.getSketchSize();
    }  else {
        flashsize = 0;
    }
#endif //ARDUINO_ARCH_ESP32
    return flashsize;
}

const char * ESP_FileSystem::FilesystemName()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    return "SPIFFS";
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return "FAT";
#endif //ESP_FAT_FILESYSTEM
    return "None";
}

bool ESP_FileSystem::format()
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    return SPIFFS.format();
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    /* FFat.end();*/
    return FFat.format();
#endif //ESP_FAT_FILESYSTEM
    return false;
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
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    if (mode != ESP_FILE_READ) {
        //check container exists
        String p = path;
        p.remove(p.lastIndexOf('/') +1);
        if (!exists(p.c_str())) {
            //log_esp3d("Error opening: %s", path);
            return ESP_File();
        }
    }
#endif
#if defined (ARDUINO_ARCH_ESP8266)
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    File ftmp = SPIFFS.open(path, (mode == ESP_FILE_READ)?"r":(mode == ESP_FILE_WRITE)?"w":"a");
    if(ftmp) {
        //log_esp3d("Success openening: %s", path);
        ESP_File esptmp(&ftmp, false,(mode == ESP_FILE_READ)?false:true, path);
        return esptmp;
    }
    (void)mode;
    Dir dtmp = SPIFFS.openDir(path);
    ESP_File esptmp(&dtmp, true, false, path);
    return esptmp;
#endif //ESP_SPIFFS_FILESYSTEM
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    //TODO add support if path = /DIR1/ <- with last /
    File tmp = SPIFFS.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    File tmp = FFat.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
#endif //ESP_FAT_FILESYSTEM
    ESP_File esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
    return esptmp;
#endif //ARDUINO_ARCH_ESP32
}

bool ESP_FileSystem::exists(const char* path)
{
    bool res = false;
    //log_esp3d("Check %s", path);
    //root should always be there
    if (strcmp(path, "/") == 0) {
        return true;
    }
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    res = FFat.exists(path);
    if (!res) {
        ESP_File root = ESP_FileSystem::open(path, ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
    }
#endif //ESP_FAT_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
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
        //log_esp3d("Check %s", newpath.c_str());
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
#endif //ESP_SPIFFS_FILESYSTEM
    return res;
}

bool ESP_FileSystem::remove(const char *path)
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    return SPIFFS.remove(path);
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return FFat.remove(path);
#endif //ESP_FAT_FILESYSTEM
}

bool ESP_FileSystem::mkdir(const char *path)
{
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
    //Use file named . to simulate directory
    String p = path;
    if (p[p.length()-1] != '/') {
        p+="/";
    }
    p+=".";
    //log_esp3d("Dir create : %s", p.c_str());
    ESP_File f = open(p.c_str(), ESP_FILE_WRITE);
    if (f) {
        f.close();
        return true;
    } else {
        return false;
    }
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
    return FFat.mkdir(path);
#endif //ESP_FAT_FILESYSTEM
}

bool ESP_FileSystem::rmdir(const char *path)
{
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
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
#endif //ESP_FAT_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
#if defined (ARDUINO_ARCH_ESP8266)
    Dir dtmp = SPIFFS.openDir(path);
    while (dtmp.next()) {
        if (!SPIFFS.remove(dtmp.fileName().c_str())) {
            return false;
        }
    }
    return true;
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    String spath = path;
    spath.trim();
    if (spath[spath.length()-1] == '/') {
        if (spath!="/") {
            spath.remove(spath.length()-1);
        }
    }
    log_esp3d("Deleting : %s",spath.c_str());
    File ftmp = SPIFFS.open(spath.c_str());
    if (ftmp) {
        File pfile = ftmp.openNextFile();
        while (pfile) {
            //log_esp3d("File: %s",pfile.name());
            if (!SPIFFS.remove(pfile.name())) {
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
#endif //ARDUINO_ARCH_ESP32
#endif //ESP_SPIFFS_FILESYSTEM
}

void ESP_FileSystem::closeAll()
{
    for (uint8_t i = 0; i < ESP_MAX_OPENHANDLE; i++) {
#if defined (ARDUINO_ARCH_ESP8266)
        tDir_handle[i] = Dir();
#endif //ARDUINO_ARCH_ESP8266
        tFile_handle[i].close();
        tFile_handle[i] = File();
    }
}

ESP_File::ESP_File(const char * name, const char * filename, bool isdir)
{
    _isdir = isdir;
    _dirlist = "";
    _isfakedir = isdir;
    _index = -1;
    _filename = filename;
    _name = name;
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
    memset (&_lastwrite,0,sizeof(time_t));
#endif //FILESYSTEM_TIMESTAMP_FEATURE 
    _iswritemode = false;
    _size = 0;
}

ESP_File::ESP_File(void* handle, bool isdir, bool iswritemode, const char * path)
{
    _isdir = isdir;
    _dirlist = "";
    _isfakedir = false;
    _index = -1;
    _filename = "";
    _name = "";
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
    memset (&_lastwrite,0,sizeof(time_t));
#endif //FILESYSTEM_TIMESTAMP_FEATURE 
    _iswritemode = iswritemode;
    _size = 0;
    if (!handle) {
        return ;
    }
    bool set =false;
#if defined (ARDUINO_ARCH_ESP8266)
    if (_isdir) {
        for (uint8_t i=0; (i < ESP_MAX_OPENHANDLE) && !set; i++) {
            if (tDir_handle[i].fileName().length() == 0) {
                tDir_handle[i] = *((Dir *)handle);
                _index = i;
                //Path = filename
                if (path) {
                    _filename = path;
                    if (_filename == "/") {
                        _filename = "/.";
                    }
                    if (_filename[_filename.length()-1] != '.') {
                        if (_filename[_filename.length()-2] != '/') {
                            _filename+="/";
                        }
                        _filename+=".";
                    }
                    //log_esp3d("Filename: %s", _filename.c_str());
                    //Name
                    if (_filename == "/.") {
                        _name = "/";
                    } else {
                        _name = _filename;
                        if (_name.length() >=2) {
                            if ((_name[_name.length() - 1] == '.') && (_name[_name.length() - 2] == '/')) {
                                _name.remove( _name.length() - 2,2);
                            }
                        }
                        _name.remove( 0, _name.lastIndexOf('/')+1);
                    }
                }
                //log_esp3d("Name: %s index: %d", _name.c_str(), _index);
                set = true;
            }
        }
        return;
    }
#endif //ARDUINO_ARCH_ESP8266

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
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
            _lastwrite =  tFile_handle[i].getLastWrite();
#endif //FILESYSTEM_TIMESTAMP_FEATURE
            _index = i;
            //log_esp3d("Opening File at index %d",_index);
            set = true;
        }
    }
}

void ESP_File::close()
{
    if (_index != -1) {
#if defined (ARDUINO_ARCH_ESP8266)
        if (_isdir && !_isfakedir) {
            //log_esp3d("Closing Dir at index %d", _index);
            tDir_handle[_index] = Dir();
            _index = -1;
            return;
        }
#endif //ARDUINO_ARCH_ESP8266
        //log_esp3d("Closing File at index %d", _index);
        tFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
#if defined (ARDUINO_ARCH_ESP8266)
            File ftmp = SPIFFS.open(_filename.c_str(), "r");
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
#if (FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM)
            File ftmp = SPIFFS.open(_filename.c_str());
#endif //ESP_SPIFFS_FILESYSTEM
#if (FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM)
            File ftmp = FFat.open(_filename.c_str());
#endif //ESP_FAT_FILESYSTEM
#endif //ARDUINO_ARCH_ESP32

            if (ftmp) {
                _size = ftmp.size();
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
                _lastwrite = ftmp.getLastWrite();
#endif //FILESYSTEM_TIMESTAMP_FEATURE
                ftmp.close();
            }
        }
#if defined (ARDUINO_ARCH_ESP32)
        tFile_handle[_index] = File();
#endif //ARDUINO_ARCH_ESP32
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
#if defined (ARDUINO_ARCH_ESP8266)
    if(tDir_handle[_index].next()) {
        //log_esp3d("Getting next file from %s", _filename.c_str());
        File tmp = tDir_handle[_index].openFile("r");
        while (tmp) {
            ESP_File esptmp(&tmp);
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
                    if(!tDir_handle[_index].next()) {
                        return ESP_File();
                    } else {
                        tmp = tDir_handle[_index].openFile("r");
                    }
                }
            } else { //is file
                //log_esp3d("file name:%s name: %s %s  sub:%s root:%s", esptmp.filename(), esptmp.name(), (esptmp.isDirectory())?"isDir":"isFile", sub.c_str(), _filename.c_str());
                if (sub == ".") {
                    //log_esp3d("Dir tag, ignore it");
                    if(!tDir_handle[_index].next()) {
                        return ESP_File();
                    } else {
                        tmp = tDir_handle[_index].openFile("r");
                    }
                } else {
                    return esptmp;
                }
            }
        }
    }
    return  ESP_File();
#endif //ARDUINO_ARCH_ESP8266
#if defined (ARDUINO_ARCH_ESP32)
    File tmp = tFile_handle[_index].openNextFile();
    while (tmp) {
        //log_esp3d("tmp name :%s %s", tmp.name(), (tmp.isDirectory())?"isDir":"isFile");
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
#endif //ARDUINO_ARCH_ESP32
}

ESP_File::~ESP_File()
{
    //log_esp3d("Destructor %s index %d",(_isdir)?"Dir":"File", _index);
}

ESP_File::operator bool() const
{
    if ((_index != -1) || (_filename.length() > 0)) {
        //log_esp3d("Bool yes %d %d",_index,  _filename.length());
        return true;
    } else {
        return false;
    }
}

bool ESP_File::isOpen()
{
    return !(_index == -1);
}

const char* ESP_File::name() const
{
    return _name.c_str();
}

const char* ESP_File::filename() const
{
    return _filename.c_str();
}

bool ESP_File::isDirectory()
{
    return _isdir;
}

size_t ESP_File::size()
{
    return _size;
}

#ifdef FILESYSTEM_TIMESTAMP_FEATURE
time_t ESP_File::getLastWrite()
{
    return _lastwrite;
}
#endif //FILESYSTEM_TIMESTAMP_FEATURE

int ESP_File::available()
{
    if (_index == -1 || _isdir) {
        return 0;
    }
    return tFile_handle[_index].available();
}

size_t ESP_File::write(uint8_t i)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tFile_handle[_index].write (i);
}

size_t ESP_File::write(const uint8_t *buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return 0;
    }
    return tFile_handle[_index].write (buf, size);
}

int ESP_File::read()
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tFile_handle[_index].read();
}

size_t ESP_File::read(uint8_t* buf, size_t size)
{
    if ((_index == -1) || _isdir) {
        return -1;
    }
    return tFile_handle[_index].read(buf, size);
}

void ESP_File::flush()
{
    if ((_index == -1) || _isdir) {
        return;
    }
    tFile_handle[_index].flush();
}

ESP_File& ESP_File::operator=(const ESP_File & other)
{
    //log_esp3d("Copy %s", other._filename.c_str());
    _isdir = other._isdir;
    _isfakedir = other._isfakedir;
    _index = other._index;
    _filename = other._filename;
    _name = other._name;
    _size = other._size;
    _iswritemode = other._iswritemode;
    _dirlist = other._dirlist;
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
    memcpy(&_lastwrite, &(other._lastwrite), sizeof (time_t));
#endif //FILESYSTEM_TIMESTAMP_FEATURE
    return *this;
}
