/*
  bmx280.cpp -  sensor functions class

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
#if SENSOR_DEVICE == BMP280_DEVICE || SENSOR_DEVICE == BME280_DEVICE
#include <BMx280I2C.h>
#include <Wire.h>

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "bmx280.h"

#define NB_TYPE_SENSOR 2
const char *SENSOR_NAME[NB_TYPE_SENSOR] = {"BMP280", "BME280"};
const uint8_t SENSOR_ID[NB_TYPE_SENSOR] = {BMP280_DEVICE, BME280_DEVICE};
BMx280I2C *bmx280_device;

BMX280SensorDevice::BMX280SensorDevice() { bmx280_device = nullptr; }

BMX280SensorDevice::~BMX280SensorDevice() { end(); }

bool BMX280SensorDevice::begin() {
  end();
  uint8_t sensortype = ESP3DSettings::readByte(ESP_SENSOR_TYPE);
  if (sensortype == 0) {
    esp3d_log("No Sensor active");
    return true;
  }
  if (!isModelValid(sensortype)) {
    esp3d_log_e("No valid id ");
    return false;
  }
  // Setup Wire pins first as lib does setup wire
  Wire.begin(ESP_SDA_PIN, ESP_SCL_PIN);
  esp3d_log("Starting wire SDA:%d SCL:%d", ESP_SDA_PIN, ESP_SCL_PIN);
  bmx280_device = new BMx280I2C(SENSOR_ADDR);
  if (!bmx280_device) {
    esp3d_log_e("Cannot instanciate sensor");
    return false;
  }
  if (!bmx280_device->begin()) {
    esp3d_log("No valid sensor status");
    return false;
  }
  // reset sensor to default parameters.
  bmx280_device->resetToDefaults();

  // by default sensing is disabled and must be enabled by setting a non-zero
  // oversampling setting.
  // set an oversampling setting for pressure and temperature measurements.
  bmx280_device->writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmx280_device->writeOversamplingTemperature(BMx280MI::OSRS_T_x16);

  // if sensor is a BME280, set an oversampling setting for humidity
  // measurements.
  if (bmx280_device->isBME280()) {
    bmx280_device->writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
  }
  return true;
}

void BMX280SensorDevice::end() {
  if (bmx280_device) {
    delete bmx280_device;
  }
  bmx280_device = nullptr;
}

bool BMX280SensorDevice::isModelValid(uint8_t model) {
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    if (model == SENSOR_ID[i]) {
      return true;
    }
  }
  return false;
}

uint8_t BMX280SensorDevice::getIDFromString(const char *s) {
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    esp3d_log("checking %s with %s", s, SENSOR_NAME[i]);
    if (strcmp(s, SENSOR_NAME[i]) == 0) {
      esp3d_log("found %d", SENSOR_ID[i]);
      return SENSOR_ID[i];
    }
  }

  return 0;
}

uint8_t BMX280SensorDevice::nbType() { return NB_TYPE_SENSOR; }

uint8_t BMX280SensorDevice::GetModel(uint8_t i) {
  if (i < NB_TYPE_SENSOR) {
    return SENSOR_ID[i];
  }
  return 0;
}

const char *BMX280SensorDevice::GetCurrentModelString() {
  uint8_t sensortype = ESP3DSettings::readByte(ESP_SENSOR_TYPE);
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    if (sensortype == SENSOR_ID[i]) {
      return SENSOR_NAME[i];
    }
  }
  return "NONE";
}

const char *BMX280SensorDevice::GetModelString(uint8_t i) {
  if (i < NB_TYPE_SENSOR) {
    return SENSOR_NAME[i];
  }
  return "NONE";
}

// helper function
float toFahrenheit(float fromCelcius) { return 1.8 * fromCelcius + 32.0; };

const char *BMX280SensorDevice::GetData() {
  static String s;
  if (bmx280_device) {
    if (!bmx280_device->measure()) {
      s = "BUSY";
      esp3d_log("sensor is busy");
    } else {
      uint8_t nbtry = 0;
      do {
        esp3d_log("try sensor %d", nbtry);
        ESP3DHal::wait(100);
        nbtry++;
      } while (!bmx280_device->hasValue() && nbtry < 3);
      if (bmx280_device->hasValue()) {
        float temperature = bmx280_device->getTemperature();
        float pressure = bmx280_device->getPressure();
        float humidity = 0;
        if (bmx280_device->isBME280()) {
          humidity = bmx280_device->getHumidity();
        }
        esp3d_log("T %f P %f H %f", temperature, pressure, humidity);
        if (String(temperature, 1) != "nan") {
          if (strcmp(SENSOR__UNIT, "F") == 0) {
            temperature = toFahrenheit(temperature);
          }
          s = String(temperature, 1);
          s += "[";
          s += SENSOR__UNIT;
          s += "] " + String(pressure, 1);
          s += "[Pa]";
          if (bmx280_device->isBME280()) {
            s += " " + String(humidity, 1) + "[%]";
          }
        } else {
          s = "DISCONNECTED";
          esp3d_log_e("No valid data");
        }
      } else {
        s = "DISCONNECTED";
        esp3d_log_e("No valid data");
      }
    }
  } else {
    s = "DISCONNECTED";
    esp3d_log_e("No device");
  }
  return s.c_str();
}

#endif  // BMP280_DEVICE || BME280_DEVICE
#endif  // SENSOR_DEVICE
