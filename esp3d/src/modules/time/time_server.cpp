/*
  time_server.cpp -  time server functions class

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

#include "../../include/esp3d_config.h"
#ifdef TIMESTAMP_FEATURE
#include "time_server.h"
#include <time.h>
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#if defined (WIFI_FEATURE)
#include "../wifi/wificonfig.h"
#endif //WIFI_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#if defined (ETH_FEATURE)
#include "../ethernet/ethconfig.h"
#endif //ETH_FEATURE

TimeServer timeserver;

TimeServer::TimeServer()
{
    _started = false;
    _is_internet_time = false;
}
TimeServer::~TimeServer()
{
    end();
}

bool TimeServer::is_internet_time(bool readfromsettings)
{
    if (readfromsettings) {
        _is_internet_time = (Settings_ESP3D::read_byte (ESP_INTERNET_TIME) == 0);
    }
    return _is_internet_time;
}

bool TimeServer::begin()
{
    bool res = true;
    end();
    String s1, s2, s3;
    int8_t t1;
    byte d1;
#if defined (WIFI_FEATURE)
    //no time server in AP mode
    if (WiFi.getMode() == WIFI_AP) {
        return false;
    }
#endif //WIFI_FEATURE
#if defined (BLUETOOTH_FEATURE)
    //no time server in BT
    if (bt_service.started()) {
        return false;
    }
#endif //BLUETOOTH_FEATURE

#if defined (ETH_FEATURE)
    if (!EthConfig::started()) {
#if defined (WIFI_FEATURE)
        //no time server if no ETH started and no WiFi started
        if (WiFi.getMode() == WIFI_OFF) {
            return false;
        }
#else
        //no time server if no ETH started and no WiFi
        return false;
#endif //WIFI_FEATURE
    }
#endif //ETH_FEATURE
    if (!is_internet_time()) {
        return true;
    }
    s1 = Settings_ESP3D::read_string (ESP_TIME_SERVER1);
    s2 = Settings_ESP3D::read_string (ESP_TIME_SERVER2);
    s3 = Settings_ESP3D::read_string (ESP_TIME_SERVER3);
    t1 = (int8_t)Settings_ESP3D::read_byte (ESP_TIMEZONE);
    d1 = Settings_ESP3D::read_byte (ESP_TIME_IS_DST);
    configTime (3600 * (t1), d1 * 3600, s1.c_str(), s2.c_str(), s3.c_str() );
    time_t now = time(nullptr);
    int nb = 0;
    while ((now < (8 * 3600 * 2)) && (nb < 20)) {
        yield();
        delay(500);
        nb++;
        now = time(nullptr);
    }
    if (now < (8 * 3600 * 2)) {
        res = false;
    }
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}

const char * TimeServer::current_time(time_t t)
{
    static String stmp;
    struct tm  tmstruct;
    time_t now;
    stmp = "";
    //get current time
    if (t == 0) {
        time(&now);
        localtime_r(&now, &tmstruct);
    } else {
        localtime_r(&t, &tmstruct);
    }
    stmp = String((tmstruct.tm_year)+1900) + "-";
    if (((tmstruct.tm_mon)+1) < 10) {
        stmp +="0";
    }
    stmp += String(( tmstruct.tm_mon)+1) + "-";
    if (tmstruct.tm_mday < 10) {
        stmp +="0";
    }
    stmp += String(tmstruct.tm_mday) + " ";
    if (tmstruct.tm_hour < 10) {
        stmp +="0";
    }
    stmp += String(tmstruct.tm_hour) + ":";
    if (tmstruct.tm_min < 10) {
        stmp +="0";
    }
    stmp += String(tmstruct.tm_min) + ":";
    if (tmstruct.tm_sec < 10) {
        stmp +="0";
    }
    stmp += String(tmstruct.tm_sec);
    return stmp.c_str();
}

//the string date time  need to be iso-8601
//the time zone part will be ignored
bool TimeServer::setTime(const char* stime)
{
    String stmp = stime;
    struct tm  tmstruct;
    struct timeval time_val = {0, 0};
    memset(&tmstruct, 0, sizeof(struct tm));
    if (strptime(stime,"%Y-%m-%dT%H:%M:%S", &tmstruct)==nullptr) {
        //allow not to set seconds for lazy guys typing command line
        if (strptime(stime,"%Y-%m-%dT%H:%M", &tmstruct)==nullptr) {
            return false;
        }
    }
    tmstruct.tm_isdst = 0; //ignore dst
    time_val.tv_sec = mktime (&tmstruct);
    //try to setTime
    if(settimeofday(&time_val,0) == -1) {
        return false;
    }
    return true;
}

bool TimeServer::started()
{
    return _started;
}

//currently not used
void TimeServer::end()
{
    _started = false;
    _is_internet_time = false;
}

//currently not used
void TimeServer::handle()
{
    if (_started) {
    }
}

#endif //TimeServer_DEVICE
