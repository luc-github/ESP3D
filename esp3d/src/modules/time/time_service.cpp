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
#include <time.h>

#include "../../core/esp3doutput.h"
#include "../../core/settings_esp3d.h"
#include "time_service.h"

#if defined(WIFI_FEATURE)
#include "../wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#if defined(BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE
#if defined(ETH_FEATURE)
#include "../ethernet/ethconfig.h"
#endif  // ETH_FEATURE

TimeService timeService;

const char* SupportedTimeZones[] = {
    "-12:00", "-11:00", "-10:00", "-09:00", "-08:00", "-07:00", "-06:00",
    "-05:00", "-04:00", "-03:30", "-03:00", "-02:00", "-01:00", "+00:00",
    "+01:00", "+02:00", "+03:00", "+03:30", "+04:00", "+04:30", "+05:00",
    "+05:30", "+05:45", "+06:00", "+06:30", "+07:00", "+08:00", "+08:45",
    "+09:00", "+09:30", "+10:00", "+10:30", "+11:00", "+12:00", "+12:45",
    "+13:00", "+14:00"};

const uint8_t SupportedTimeZonesSize =
    sizeof(SupportedTimeZones) / sizeof(const char*);

TimeService::TimeService() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
}
TimeService::~TimeService() { end(); }

bool TimeService::is_internet_time(bool readfromsettings) {
  if (readfromsettings) {
    _is_internet_time =
        Settings_ESP3D::read_byte(ESP_INTERNET_TIME) ? true : false;
    log_esp3d("Internet time is %s",
              _is_internet_time ? "enabled" : "disabled");
  }
  return _is_internet_time;
}

bool TimeService::begin() {
  end();
  String s1, s2, s3, t1;
  byte d1;
#if defined(WIFI_FEATURE)
  // no time server in AP mode
  if (WiFi.getMode() == WIFI_AP) {
    log_esp3d("No Internet time in AP mode");
    return false;
  }
#endif  // WIFI_FEATURE
#if defined(BLUETOOTH_FEATURE)
  // no time server in BT
  if (bt_service.started()) {
    return false;
  }
#endif  // BLUETOOTH_FEATURE

#if defined(ETH_FEATURE)
  if (!EthConfig::started()) {
#if defined(WIFI_FEATURE)
    // no time server if no ETH started and no WiFi started
    if (WiFi.getMode() == WIFI_OFF) {
      return false;
    }
#else
    // no time server if no ETH started and no WiFi
    return false;
#endif  // WIFI_FEATURE
  }
#endif  // ETH_FEATURE
  if (!is_internet_time(true)) {
    return true;
  }
  s1 = Settings_ESP3D::read_string(ESP_TIME_SERVER1);
  s2 = Settings_ESP3D::read_string(ESP_TIME_SERVER2);
  s3 = Settings_ESP3D::read_string(ESP_TIME_SERVER3);
  t1 = Settings_ESP3D::read_string(ESP_TIME_ZONE);
  d1 = Settings_ESP3D::read_byte(ESP_TIME_IS_DST);
#if defined(ARDUINO_ARCH_ESP32)
  configTzTime(t1.c_str(), s1.c_str(), s2.c_str(), s3.c_str());
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  configTime(t1, s1.c_str(), s2.c_str(), s3.c_str());
#endif  // ARDUINO_ARCH_ESP8266

  time_t now = time(nullptr);
  int nb = 0;
  while ((now < (8 * 3600 * 2)) && (nb < 20)) {
    yield();
    delay(500);
    nb++;
    now = time(nullptr);
  }
  _started = true;  // always true, time is set asynchrously
  return _started;
}

const char* TimeService::getTimeZone() { return _time_zone.c_str(); }

bool TimeService::setTimeZone(const char* stime) {
  bool valid = false;
  for (uint8_t i = 0; i < SupportedTimeZonesSize; i++) {
    if (strcmp(stime, SupportedTimeZones[i]) == 0) {
      valid = true;
      break;
    }
  }
  if (valid) {
    _time_zone = stime;
    return Settings_ESP3D::write_string(ESP_TIME_ZONE, _time_zone.c_str());
  }
  return false;
}

bool TimeService::updateTimeZone(bool fromsettings) {
  char out_str[7] = {0};
  _time_zone = Settings_ESP3D.readString(ESP_TIME_ZONE);

  bool valid = false;
  for (uint8_t i = 0; i < SupportedTimeZonesSize; i++) {
    if (strcmp(_time_zone.c_str(), SupportedTimeZones[i]) == 0) {
      valid = true;
      break;
    }
  }

  if (!valid) {
    log_esp3d_e("Invalid time zone %s", _time_zone.c_str());
    _time_zone = "+00:00";
  }
  String stmp = _time_zone;
  if (stmp[0] == '+') {
    stmp[0] = '-';
  } else if (stmp[0] == '-') {
    stmp[0] = '+';
  } else {
    return false;
  }
  stmp = "GMT" + stmp;
#if defined(ARDUINO_ARCH_ESP32)
  setenv("TZ", stmp.c_str(), 1);
  tzset();
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  setTZ(stmp.c_str());
#endif  // ARDUINO_ARCH_ESP8266

  return true;
}

const char* TimeService::getCurrentTime() {
  struct tm tmstruct;
  time_t now;
  // get current time
  time(&now);
  localtime_r(&now, &tmstruct);
  static char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmstruct);
  log_esp3d("Time string is %s", buf);
  return buf;
}

// the string date time  need to be iso-8601
// the time zone part will be ignored
bool TimeService::setTime(const char* stime) {
  log_esp3d("Set time to %s", stime);
  String stmp = stime;
  struct tm tmstruct;
  struct timeval time_val = {0, 0};
  memset(&tmstruct, 0, sizeof(struct tm));
  if (strptime(stime, "%Y-%m-%dT%H:%M:%S", &tmstruct) == nullptr) {
    log_esp3d("Invalid time format, try without seconds");
    // allow not to set seconds for lazy guys typing command line
    if (strptime(stime, "%Y-%m-%dT%H:%M", &tmstruct) == nullptr) {
      log_esp3d("Invalid time format");
      return false;
    }
  }

  time_val.tv_usec = 0;
  time_val.tv_sec = mktime(&tmstruct);

  // need to set timezone also
  int offset = _get_time_zone_offset_min();
  struct timezone tz = {offset, 0};
  // now set time to system
  if (settimeofday(&time_val, &tz) == -1) {
    return false;
  }
  return true;
}

bool TimeService::started() { return _started; }

// currently not used
void TimeService::end() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
}

// currently not used
void TimeService::handle() {
  if (_started) {
  }
}

int TimeService::_get_time_zone_offset_min() {
  int offset = 0;
  int hour = atoi(_time_zone.substr(1, 2).c_str());
  int min = atoi(_time_zone.substr(4, 2).c_str());
  offset = hour * 60 + min;
  // result is in minutes west of GMT
  if (_time_zone[0] == '+' && offset > 0) {
    offset = -offset;
  }
  return offset;
}

#endif  // TimeService_DEVICE
