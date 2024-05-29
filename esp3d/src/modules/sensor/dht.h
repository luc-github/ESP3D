/*
  dht.h -  sensor functions class

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

#ifndef _DHT_SENSOR_H
#define _DHT_SENSOR_H

#include "sensor.h"

class DHTSensorDevice : ESP3DSensorDevice {
 public:
  DHTSensorDevice();
  ~DHTSensorDevice();
  bool begin();
  void end();
  bool isModelValid(uint8_t model);
  uint8_t getIDFromString(const char *);
  uint8_t nbType();
  uint8_t GetModel(uint8_t i = 0);
  const char *GetModelString(uint8_t i = 0);
  const char *GetCurrentModelString();
  const char *GetData();
};

#endif  //_DHT_SENSOR_H
