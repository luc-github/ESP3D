/*
  time_service.h -  time server functions class

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

#ifndef _TIME_SERVICE_H
#define _TIME_SERVICE_H

#include <time.h>

class TimeService {
 public:
  TimeService();
  ~TimeService();
  bool begin();
  void end();
  void handle();
  const char* getDateTime(time_t t = 0);
  const char* getCurrentTime();
  const char* getTimeZone();
  bool updateTimeZone(bool fromsettings = false);
  bool setTimeZone(const char* stime);
  bool setTime(const char* stime);
  bool started();
  bool is_internet_time(bool readfromsettings = false);

 private:
  int _get_time_zone_offset_min();
  bool _started;
  bool _is_internet_time;
  String _time_zone;
};

extern TimeService timeService;

extern const char* SupportedTimeZones[];

extern const uint8_t SupportedTimeZonesSize;

#endif  //_TIME_SERVICE_H
