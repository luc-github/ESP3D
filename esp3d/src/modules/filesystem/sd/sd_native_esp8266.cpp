/*
sd_native_esp8266.cpp - ESP3D sd support class

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
#if defined (ARDUINO_ARCH_ESP8266) && defined(SD_DEVICE)
#if (SD_DEVICE == ESP_SD_NATIVE)
#define FS_NO_GLOBALS
#include "../esp_sd.h"
#include "../../../core/genLinkedList.h"
#include "../../../core/settings_esp3d.h"
#include <SD.h>
#include <SDFS.h>
extern File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];

void dateTime (uint16_t* date, uint16_t* dtime)
{
    struct tm  tmstruct;
    time_t now;
    time (&now);
    localtime_r (&now, &tmstruct);
    *date = FAT_DATE ( (tmstruct.tm_year) + 1900, ( tmstruct.tm_mon) + 1, tmstruct.tm_mday);
    *dtime = FAT_TIME (tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
}

time_t getDateTimeFile(File & filehandle)
{
    static time_t dt = 0;
#ifdef SD_TIMESTAMP_FEATURE
    struct tm timefile;
    dir_t d;
    if(filehandle) {
        if (filehandle.dirEntry(&d)) {
            timefile.tm_year = FAT_YEAR(d.lastWriteDate) - 1900;
            timefile.tm_mon = FAT_MONTH(d.lastWriteDate) - 1;
            timefile.tm_mday = FAT_DAY(d.lastWriteDate);
            timefile.tm_hour = FAT_HOUR(d.lastWriteTime);
            timefile.tm_min = FAT_MINUTE(d.lastWriteTime);
            timefile.tm_sec = FAT_SECOND(d.lastWriteTime);
            timefile.tm_isdst = -1;
            dt =  mktime(&timefile);
            if (dt == -1) {
                log_esp3d("mktime failed");
            }
        } else {
            log_esp3d("stat file failed");
        }
    } else {
        log_esp3d("check file for stat failed");
    }
#endif //SD_TIMESTAMP_FEATURE
    return dt;
}

uint8_t ESP_SD::getState(bool refresh)
{
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
    //no need to go further if SD detect is not correct
    if (!((digitalRead (ESP_SD_DETECT_PIN) == ESP_SD_DETECT_VALUE) ? true : false)) {
        log_esp3d("No SD State %d vs %d", digitalRead (ESP_SD_DETECT_PIN), ESP_SD_DETECT_VALUE);
        _state = ESP_SDCARD_NOT_PRESENT;
        return _state;
    } else {
        log_esp3d("SD Detect Pin ok");
    }
#endif  //ESP_SD_DETECT_PIN
    //if busy doing something return state
    if (!((_state == ESP_SDCARD_NOT_PRESENT) || _state == ESP_SDCARD_IDLE)) {
        log_esp3d("Busy SD State");
        return _state;
    }
    if (!refresh) {
        log_esp3d("SD State cache is %d", _state);
        return _state;  //to avoid refresh=true + busy to reset SD and waste time
    } else {
        _sizechanged = true;
    }
    //SD is idle or not detected, let see if still the case
    _state = ESP_SDCARD_NOT_PRESENT;
    bool isactive = accessSD();
    //refresh content if card was removed
    if (SD.begin((ESP_SD_CS_PIN == -1)?SS:ESP_SD_CS_PIN, SD_SCK_HZ(F_CPU/_spi_speed_divider))) {
        log_esp3d("Init SD State ok");
        if (SD.size64()  > 0 ) {
            log_esp3d("SD available");
            _state = ESP_SDCARD_IDLE;
        } else {
            log_esp3d("Cannot get card size");
        }
    } else {
        log_esp3d("Init SD State failed");
    }
    log_esp3d("SD State is %d", _state);
    if (!isactive) {
        releaseSD();
    }
    return _state;
}

bool ESP_SD::begin()
{
    _started = true;
    _state = ESP_SDCARD_NOT_PRESENT;
    _spi_speed_divider = Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV);
    //sanity check
    if (_spi_speed_divider <= 0) {
        _spi_speed_divider = 1;
    }
#ifdef SD_TIMESTAMP_FEATURE
    //set callback to get time on files on SD
    SdFile::dateTimeCallback (dateTime);
#endif //SD_TIMESTAMP_FEATURE
    //Setup pins
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
    pinMode (ESP_SD_DETECT_PIN, INPUT);
#endif //ESP_SD_DETECT_PIN
#if SD_DEVICE_CONNECTION  == ESP_SHARED_SD
#if defined(ESP_FLAG_SHARED_SD_PIN) && ESP_FLAG_SHARED_SD_PIN != -1
    pinMode (ESP_FLAG_SHARED_SD_PIN, OUTPUT);
    digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
#endif //ESP_FLAG_SHARED_SD_PIN
#endif //SD_DEVICE_CONNECTION  == ESP_SHARED_SD
    return _started;
}

void ESP_SD::end()
{
    _state = ESP_SDCARD_NOT_PRESENT;
    _started = false;
}

uint64_t ESP_SD::totalBytes()
{
    return SD.size64();
}

uint64_t ESP_SD::usedBytes()
{
    FSInfo64 info;
    static uint64_t volUsed;
    if (_sizechanged) {
        if (!SDFS.info64(info)) {
            return 0;
        }
        volUsed = info.usedBytes;
        _sizechanged = false;
    }
    return volUsed;
}

uint64_t ESP_SD::freeBytes()
{
    if(usedBytes() >totalBytes() ) {
        _sizechanged = true;
    }
    return totalBytes() - usedBytes();
}

uint ESP_SD::maxPathLength()
{
    return 255;
}

bool ESP_SD::rename(const char *oldpath, const char *newpath)
{
    return (bool)SDFS.rename(oldpath,newpath);
}


bool ESP_SD::format(ESP3DOutput * output)
{
    if (output) {
        output->printERROR ("Not implemented!");
    }
    return false;
}

ESP_SDFile ESP_SD::open(const char* path, uint8_t mode)
{
    //do some check
    if(((strcmp(path,"/") == 0) && ((mode == ESP_FILE_WRITE) || (mode == ESP_FILE_APPEND))) || (strlen(path) == 0)) {
        _sizechanged = true;
        return ESP_SDFile();
    }
    // path must start by '/'
    if (path[0] != '/') {
        return ESP_SDFile();
    }
    if (mode != ESP_FILE_READ) {
        //check container exists
        String p = path;
        p.remove(p.lastIndexOf('/') +1);
        if (!exists(p.c_str())) {
            log_esp3d("Error opening: %s", path);
            return ESP_SDFile();
        }
    }
    File tmp = SD.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_WRITE);
    ESP_SDFile esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
    return esptmp;
}

bool ESP_SD::exists(const char* path)
{
    bool res = false;
    //root should always be there if started
    if (strcmp(path, "/") == 0) {
        return _started;
    }
    log_esp3d("%s exists ?", path);
    res = SD.exists(path);
    if (!res) {
        log_esp3d("Seems not -  trying open it");
        ESP_SDFile root = ESP_SD::open(path, ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
    }
    log_esp3d("Seems %s", res?"yes":"no");
    return res;
}

bool ESP_SD::remove(const char *path)
{
    _sizechanged = true;
    return SD.remove(path);
}

bool ESP_SD::mkdir(const char *path)
{
    return SD.mkdir(path);
}

bool ESP_SD::rmdir(const char *path)
{
    if (!exists(path)) {
        return false;
    }
    bool res = true;
    GenLinkedList<String > pathlist;
    String p = path;
    pathlist.push(p);
    while (pathlist.count() > 0) {
        File dir = SD.open(pathlist.getLast().c_str());
        dir.rewindDirectory();
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
                _sizechanged = true;
                SD.remove(f.fullName());
                f.close();
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.getLast() !="/") {
                res = SD.rmdir(pathlist.getLast().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.count());
    return res;
}

bool ESP_SDFile::seek(uint32_t pos, uint8_t mode)
{
    return tSDFile_handle[_index].seek(pos, (SeekMode)mode);
}

void ESP_SD::closeAll()
{
    for (uint8_t i = 0; i < ESP_MAX_SD_OPENHANDLE; i++) {
        tSDFile_handle[i].close();
        tSDFile_handle[i] = File();
    }
}

ESP_SDFile::ESP_SDFile(void* handle, bool isdir, bool iswritemode, const char * path)
{
    _isdir = isdir;
    _dirlist = "";
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
    for (uint8_t i=0; (i < ESP_MAX_SD_OPENHANDLE) && !set; i++) {
        if (!tSDFile_handle[i]) {
            tSDFile_handle[i] = *((File*)handle);
            //filename
            _filename = path;
            //name
            _name = tSDFile_handle[i].name();
            if (_name.endsWith("/")) {
                _name.remove( _name.length() - 1,1);
                _isdir = true;
            }
            if (_name[0] == '/') {
                _name.remove( 0, 1);
            }
            int pos = _name.lastIndexOf('/');
            if (pos != -1) {
                _name.remove( 0, pos+1);
            }
            if (_name.length() == 0) {
                _name = "/";
            }
            //size
            _size = tSDFile_handle[i].size();
            //time
            if (!_isdir) {
                _lastwrite = getDateTimeFile(tSDFile_handle[i]);

            } else {
                //no need date time for directory
                _lastwrite = 0;
            }
            _index = i;
            //log_esp3d("Opening File at index %d",_index);
            set = true;
        }
    }
}
//todo need also to add short filename
const char* ESP_SDFile::shortname() const
{
    //not supported in native so return name
    return _name.c_str();

}

void ESP_SDFile::close()
{
    if (_index != -1) {
        //log_esp3d("Closing File at index %d", _index);
        tSDFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = SD.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                _lastwrite = getDateTimeFile(ftmp);
                ftmp.close();
            }
        }
        tSDFile_handle[_index] = File();
        //log_esp3d("Closing File at index %d",_index);
        _index = -1;
    }
}

ESP_SDFile  ESP_SDFile::openNextFile()
{
    if ((_index == -1) || !_isdir) {
        log_esp3d("openNextFile failed");
        return ESP_SDFile();
    }
    File tmp = tSDFile_handle[_index].openNextFile();
    if (tmp) {
        String name = tmp.name();
        log_esp3d("tmp name :%s %s", name.c_str(), (tmp.isDirectory())?"isDir":"isFile");
        String s = _filename ;
        if (s!="/") {
            s+="/";
        }
        s += name;
        ESP_SDFile esptmp(&tmp, tmp.isDirectory(),false, s.c_str());
        esptmp.close();
        return esptmp;
    }
    return  ESP_SDFile();
}

const char * ESP_SD::FilesystemName()
{
    return "SD native";
}

#endif //SD_DEVICE == ESP_SD_NATIVE
#endif //ARCH_ESP8266 && SD_DEVICE
