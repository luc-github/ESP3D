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

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
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

#if defined(GCODE_HOST_FEATURE)
#include "../gcode_host/gcode_host.h"
#endif  // GCODE_HOST_FEATURE

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
  _isInternetTime = false;
  _time_zone = "+00:00";
}
TimeService::~TimeService() { end(); }

const char* TimeService::getDateTime(time_t t) {
  static char buff[20];
  strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&t));

  return buff;
}

bool TimeService::isInternetTime(bool readfromsettings) {
  if (readfromsettings) {
    _isInternetTime = ESP3DSettings::readByte(ESP_INTERNET_TIME) ? true : false;
    esp3d_log("Internet time is %s", _isInternetTime ? "enabled" : "disabled");
  }
  return _isInternetTime;
}

bool TimeService::begin() {
  esp3d_log("Starting TimeService");
  end();
  updateTimeZone(true);
#if defined(WIFI_FEATURE)
  // no time server in AP mode
  if (WiFi.getMode() == WIFI_AP) {
    esp3d_log("No Internet time in AP mode");
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
  if (!isInternetTime(true)) {
    return true;
  }
  _server[0] = ESP3DSettings::readString(ESP_TIME_SERVER1);
  _server[1] = ESP3DSettings::readString(ESP_TIME_SERVER2);
  _server[2] = ESP3DSettings::readString(ESP_TIME_SERVER3);
#if defined(ARDUINO_ARCH_ESP32)
  configTzTime(_time_zone_config.c_str(), _server[0].c_str(),  _server[1].length() > 0 ? _server[1].c_str() : nullptr,  _server[2].length() > 0 ? _server[2].c_str() : nullptr);  
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  configTime(_time_zone_config.c_str(), _server[0].c_str(),  _server[1].length() > 0 ? _server[1].c_str() : nullptr,  _server[2].length() > 0 ? _server[2].c_str() : nullptr);
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
    return ESP3DSettings::writeString(ESP_TIME_ZONE, _time_zone.c_str());
  }
  return false;
}

bool TimeService::updateTimeZone(bool fromsettings) {
  _time_zone = ESP3DSettings::readString(ESP_TIME_ZONE);

  bool valid = false;
  for (uint8_t i = 0; i < SupportedTimeZonesSize; i++) {
    if (strcmp(_time_zone.c_str(), SupportedTimeZones[i]) == 0) {
      valid = true;
      break;
    }
  }

  if (!valid) {
    esp3d_log_e("Invalid time zone %s", _time_zone.c_str());
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

  _time_zone_config = "<";
  _time_zone_config += _time_zone[0];
  _time_zone_config += _time_zone[1];
  _time_zone_config += _time_zone[2];
  _time_zone_config += _time_zone[4];
  _time_zone_config += _time_zone[5];
  _time_zone_config += ">";
  _time_zone_config += _time_zone[0]=='+' ? "-" : "+";
  _time_zone_config += &_time_zone[1];
  esp3d_log("Time zone is %s", _time_zone_config.c_str());
  return true;
}

const char* TimeService::getCurrentTime() {
  struct tm tmstruct;
  static char buf[20];
  time_t now;
  // get current time
  time(&now);
  localtime_r(&now, &tmstruct);
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmstruct);
  esp3d_log("Time string is %s", buf);
  return buf;
}

// the string date time  need to be iso-8601
// the time zone part will be ignored
bool TimeService::setTime(const char* stime) {
  esp3d_log("Set time to %s", stime);
  String stmp = stime;
  struct tm tmstruct;
  struct timeval time_val = {0, 0};
  memset(&tmstruct, 0, sizeof(struct tm));
  if (strptime(stime, "%Y-%m-%dT%H:%M:%S", &tmstruct) == nullptr) {
    esp3d_log("Invalid time format, try without seconds");
    // allow not to set seconds for lazy guys typing command line
    if (strptime(stime, "%Y-%m-%dT%H:%M", &tmstruct) == nullptr) {
      esp3d_log("Invalid time format");
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
  _isInternetTime = false;
  _time_zone = "+00:00";
}

// currently not used
void TimeService::handle() {
  static bool isSet = false;
  if (_started) {
    // check if time is set
    time_t now = time(nullptr);
    if (now < (8 * 3600 * 2)) {
      esp3d_log("Time not set, retry");
      isSet = false;
    } else {
      if (!isSet) {
        esp3d_log("Time set");
        isSet = true;
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
#if defined(ESP_GOT_DATE_TIME_HOOK) && defined(GCODE_HOST_FEATURE)
        String dateMsg =
            esp3d_string::expandString(ESP_GOT_DATE_TIME_HOOK, true);
        esp3d_gcode_host.processScript(dateMsg.c_str());
#endif  // #if defined (ESP_GOT_IP_HOOK) && defined (GCODE_HOST_FEATURE)
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
      }
    }
  }
}

int TimeService::_get_time_zone_offset_min() {
  int offset = 0;
  int hour = atoi(_time_zone.substring(1, 1 + 2).c_str());
  int min = atoi(_time_zone.substring(4, 4 + 2).c_str());
  offset = hour * 60 + min;
  // result is in minutes west of GMT
  if (_time_zone[0] == '+' && offset > 0) {
    offset = -offset;
  }
  return offset;
}

#endif  // TimeService_DEVICE
