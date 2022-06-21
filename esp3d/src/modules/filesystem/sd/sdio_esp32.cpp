/*
sdio_esp32.cpp - ESP3D sd support class

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
#if defined (ARDUINO_ARCH_ESP32) && defined(SD_DEVICE)
#if (SD_DEVICE == ESP_SDIO)
#include "../esp_sd.h"
#include "../../../core/genLinkedList.h"
#include "../../../core/settings_esp3d.h"
#include "FS.h"
#include "SD_MMC.h"


extern File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];

#define SDMMC_FORCE_BEGIN

#ifndef SDIO_BIT_MODE
#define SDIO_BIT_MODE SD_FOUR_BIT_MODE
#endif //SDIO_BIT_MODE

uint8_t ESP_SD::getState(bool refresh)
{
    static bool lastinitok = false;
#ifdef SDMMC_FORCE_BEGIN
    lastinitok = false;
#endif //SDMMC_LIGHT_CHECK
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
    //no need to go further if SD detect is not correct
    if (!((digitalRead (ESP_SD_DETECT_PIN) == ESP_SD_DETECT_VALUE) ? true : false)) {
        _state = ESP_SDCARD_NOT_PRESENT;
        return _state;
    }
#endif  //ESP_SD_DETECT_PIN
    //if busy doing something return state
    if (!((_state == ESP_SDCARD_NOT_PRESENT) || _state == ESP_SDCARD_IDLE)) {
        return _state;
    }
    if (!refresh) {
        return _state;  //to avoid refresh=true + busy to reset SD and waste time
    }
//SD is idle or not detected, let see if still the case
    _state = ESP_SDCARD_NOT_PRESENT;
//refresh content if card was removed
    if (!lastinitok) {
        log_esp3d("last init was failed try sd_mmc begin");
        //SD_MMC.end();
        if (SD_MMC.begin("/sdcard", SDIO_BIT_MODE)) {
            log_esp3d("sd_mmc begin succeed");
            if (SD_MMC.cardType() != CARD_NONE ) {
                _state = ESP_SDCARD_IDLE;
                lastinitok = true;
                log_esp3d("sd_mmc card type succeed");
            } else {
                log_esp3d("sd_mmc card type failed");
            }
        } else {
            log_esp3d("sd_mmc begin failed");
        }
    } else {
        log_esp3d("last init was ok try card type");
        if(SD_MMC.cardType() != CARD_NONE) {
            log_esp3d("checking sd_mmc card type succeed");
            _state = ESP_SDCARD_IDLE;
        } else {
            lastinitok = false;
            log_esp3d("Soft sd check failed");
            //SD_MMC.end();
            if (SD_MMC.begin("/sdcard", SDIO_BIT_MODE)) {
                log_esp3d("new sd_mmc begin succeed");
                if ( SD_MMC.cardType() != CARD_NONE ) {
                    _state = ESP_SDCARD_IDLE;
                    lastinitok = true;
                    log_esp3d("new sd_mmc card type succeed");
                } else {
                    log_esp3d("new sd_mmc card type failed");
                }
            } else {
                log_esp3d("new sd_mmc begin failed");
            }
        }
    }
    return _state;
}

bool ESP_SD::begin()
{
    log_esp3d("Begin SDIO");
    _started = true;
#ifdef SDMMC_FORCE_BEGIN
    _state = ESP_SDCARD_NOT_PRESENT;
#else
    _state = getState(true);
#endif //SDMMC_FORCE_BEGIN

    return _started;
}

void ESP_SD::end()
{
    SD_MMC.end();
    _state = ESP_SDCARD_NOT_PRESENT;
    _started = false;
}

void ESP_SD::refreshStats(bool force)
{
    if (force || _sizechanged) {
        freeBytes(true);
    }
    _sizechanged = false;
}

uint64_t ESP_SD::totalBytes(bool refresh)
{
    static uint64_t _totalBytes = 0;
    if (refresh || _totalBytes==0) {
        _totalBytes = SD_MMC.totalBytes();;
    }
    return _totalBytes;
}

uint64_t ESP_SD::usedBytes(bool refresh)
{
    static uint64_t _usedBytes = 0;
    if (refresh || _usedBytes==0) {
        _usedBytes = SD_MMC.usedBytes();
    }
    return _usedBytes;
}

uint64_t ESP_SD::freeBytes(bool refresh)
{
    return (totalBytes(refresh) - usedBytes(refresh));
}

uint ESP_SD::maxPathLength()
{
    return 255;
}

bool ESP_SD::rename(const char *oldpath, const char *newpath)
{
    return SD_MMC.rename(oldpath,newpath);
}

bool ESP_SD::format(ESP3DOutput * output)
{
    //not available yet
    if (output) {
        output->printERROR ("Not implemented!");
    }
    return false;
}

ESP_SDFile ESP_SD::open(const char* path, uint8_t mode)
{
    //do some check
    if(((strcmp(path,"/") == 0) && ((mode == ESP_FILE_WRITE) || (mode == ESP_FILE_APPEND))) || (strlen(path) == 0)) {
        log_esp3d("File open check : failed");
        return ESP_SDFile();
    }
    // path must start by '/'
    if (path[0] != '/') {
        log_esp3d("File open path is invalid");
        return ESP_SDFile();
    }
    if (mode != ESP_FILE_READ) {
        //check container exists
        String p = path;
        p.remove(p.lastIndexOf('/') +1);
        if (!exists(p.c_str())) {
            log_esp3d("Error opening: %s, %s does not exists", path,p.c_str());
            return ESP_SDFile();
        }
    }
    File tmp = SD_MMC.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
    ESP_SDFile esptmp(&tmp, tmp.isDirectory(),(mode == ESP_FILE_READ)?false:true, path);
    return esptmp;
}

bool ESP_SD::exists(const char* path)
{
    bool res = false;
    String p = path;
    //root should always be there if started
    if (p == "/")  {
        return _started;
    }

    if (p.endsWith("/")) {
        p.remove( p.length() - 1,1);
    }
    res = SD_MMC.exists(p);
    if (!res) {
        ESP_SDFile root = ESP_SD::open(p.c_str(), ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
    }
    return res;
}

bool ESP_SD::remove(const char *path)
{
    return SD_MMC.remove(path);
}

bool ESP_SD::mkdir(const char *path)
{
    String p = path;
    if (p.endsWith("/")) {
        p.remove( p.length() - 1,1);
    }
    return SD_MMC.mkdir(p.c_str());
}

bool ESP_SD::rmdir(const char *path)
{
    if (!exists(path)) {
        return false;
    }
    bool res = true;
    GenLinkedList<String > pathlist;
    String p = path;
    if (p.endsWith("/")) {
        p.remove( p.length() - 1,1);
    }
    pathlist.push(p);
    while (pathlist.count() > 0) {
        File dir = SD_MMC.open(pathlist.getLast().c_str());
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
                SD_MMC.remove(f.name());
                f.close();
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.getLast() !="/") {
                res = SD_MMC.rmdir(pathlist.getLast().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.count());
    return res;
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
            _name = tSDFile_handle[i].name();
            _filename = path;
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
            _lastwrite =  tSDFile_handle[i].getLastWrite();
            _index = i;
            //log_esp3d("Opening File at index %d",_index);
            set = true;
        }
    }
}

bool ESP_SDFile::seek(uint32_t pos, uint8_t mode)
{
    return tSDFile_handle[_index].seek(pos, (SeekMode)mode);
}

void ESP_SDFile::close()
{
    if (_index != -1) {
        //log_esp3d("Closing File at index %d", _index);
        tSDFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = SD_MMC.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                _lastwrite = ftmp.getLastWrite();
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
        log_esp3d("tmp name :%s %s %s", tmp.name(), (tmp.isDirectory())?"isDir":"isFile", _filename.c_str());
        String s = tmp.name() ;
        //if (s!="/")s+="/";
        //s += tmp.name();
        ESP_SDFile esptmp(&tmp, tmp.isDirectory(),false, s.c_str());
        esptmp.close();
        return esptmp;
    }
    return  ESP_SDFile();
}

//TODO need to find reliable way
const char* ESP_SDFile::shortname() const
{
    return _name.c_str();
}

const char * ESP_SD::FilesystemName()
{
    return "SDIO";
}
#endif //SD_DEVICE == ESP_SDIO
#endif //ARCH_ESP32 && SD_DEVICE
