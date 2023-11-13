/*
  dht.cpp -  dht functions class

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
#ifdef SENSOR_DEVICE
#if SENSOR_DEVICE == ANALOG_DEVICE
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "analogsensor.h"

AnalogSensorDevice::AnalogSensorDevice() {}

AnalogSensorDevice::~AnalogSensorDevice() {}

bool AnalogSensorDevice::begin() { return true; }

void AnalogSensorDevice::end() {}

bool AnalogSensorDevice::isModelValid(uint8_t model) {
  if (model == ANALOG_DEVICE) {
    return true;
  }
  return false;
}

uint8_t AnalogSensorDevice::getIDFromString(const char *s) {
  if (strcmp(s, "ANALOG") == 0) {
    return ANALOG_DEVICE;
  } else {
    return 0;
  }
}

uint8_t AnalogSensorDevice::nbType() { return 1; }

uint8_t AnalogSensorDevice::GetModel(uint8_t i) { return ANALOG_DEVICE; }

const char *AnalogSensorDevice::GetCurrentModelString() {
  uint8_t sensortype = ESP3DSettings::readByte(ESP_SENSOR_TYPE);
  if (sensortype == ANALOG_DEVICE) {
    return GetModelString();
  }
  return "NONE";
}

const char *AnalogSensorDevice::GetModelString(uint8_t i) { return "ANALOG"; }

const char *AnalogSensorDevice::GetData() {
  static String s;
  s = String(SENSOR_CONVERTER(analogRead(ESP3D_SENSOR_PIN))) + "[";
  s += SENSOR__UNIT;
  s += "]";
  return s.c_str();
}

#endif  // ANALOG_DEVICE
#endif  // SENSOR_DEVICE
