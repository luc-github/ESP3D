/*
sd_native_esp32.cpp - ESP3D sd support class

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
#if (SD_DEVICE == ESP_SD_NATIVE)
#include "../esp_sd.h"
#include <stack>
#include "../../../core/settings_esp3d.h"
#include "FS.h"
#include "SD.h"

//base frequency
//or use (1000000 * ESP.getCpuFreqMHz()) TBC
#define ESP_SPI_FREQ  4000000

extern File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];

uint8_t ESP_SD::getState(bool refresh)
{
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
    } else {
        _sizechanged = true;
    }
//SD is idle or not detected, let see if still the case

    SD.end();
    _state = ESP_SDCARD_NOT_PRESENT;
//using default value for speed ? should be parameter
//refresh content if card was removed
    log_esp3d("Spi : CS: %d,  Miso: %d, Mosi: %d, SCK: %d",ESP_SD_CS_PIN!=-1?ESP_SD_CS_PIN:SS, ESP_SD_MISO_PIN!=-1?ESP_SD_MISO_PIN:MISO, ESP_SD_MOSI_PIN!=-1?ESP_SD_MOSI_PIN:MOSI, ESP_SD_SCK_PIN!=-1?ESP_SD_SCK_PIN:SCK);
    if (SD.begin((ESP_SD_CS_PIN == -1)?SS:ESP_SD_CS_PIN, SPI, ESP_SPI_FREQ / _spi_speed_divider)) {
        if ( SD.cardSize() > 0 ) {
            _state = ESP_SDCARD_IDLE;
        }
    }
    return _state;
}

bool ESP_SD::begin()
{
#if (ESP_SD_CS_PIN != -1) || (ESP_SD_MISO_PIN != -1) || (ESP_SD_MOSI_PIN != -1) || (ESP_SD_SCK_PIN != -1)
    log_esp3d("Custom spi : CS: %d,  Miso: %d, Mosi: %d, SCK: %d",ESP_SD_CS_PIN, ESP_SD_MISO_PIN, ESP_SD_MOSI_PIN, ESP_SD_SCK_PIN);
    SPI.begin(ESP_SD_SCK_PIN, ESP_SD_MISO_PIN, ESP_SD_MOSI_PIN, ESP_SD_CS_PIN);
#endif
    _started = true;
    _state = ESP_SDCARD_NOT_PRESENT;
    _spi_speed_divider = Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV);
    //sanity check
    if (_spi_speed_divider <= 0) {
        _spi_speed_divider = 1;
    }
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
    SD.end();
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
        _totalBytes = SD.totalBytes();;
    }
    return _totalBytes;
}

uint64_t ESP_SD::usedBytes(bool refresh)
{
    static uint64_t _usedBytes = 0;
    if (refresh || _usedBytes==0) {
        _usedBytes = SD.usedBytes();
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
    return SD.rename(oldpath,newpath);
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
    File tmp = SD.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_APPEND);
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
    res = SD.exists(p);
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
    return SD.remove(path);
}

bool ESP_SD::mkdir(const char *path)
{
    String p = path;
    if (p.endsWith("/")) {
        p.remove( p.length() - 1,1);
    }
    return SD.mkdir(p.c_str());
}

bool ESP_SD::rmdir(const char *path)
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
    std::stack <String > pathlist;
    pathlist.push(p);
    while (pathlist.size() > 0 && res) {
        File dir = SD.open(pathlist.top().c_str());
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
                if(!SD.remove(filepath.c_str())) {
                    res = false;
                }
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.top() !="/") {
                res = SD.rmdir(pathlist.top().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.size());
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
            File ftmp = SD.open(_filename.c_str());
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
    return "SD native";
}

#endif //SD_DEVICE == ESP_SD_NATIVE
#endif //ARCH_ESP32 && SD_DEVICE
