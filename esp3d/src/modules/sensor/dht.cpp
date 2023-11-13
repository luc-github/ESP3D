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
#if SENSOR_DEVICE == DHT11_DEVICE || SENSOR_DEVICE == DHT22_DEVICE
#include <DHTesp.h>

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "dht.h"

#define NB_TYPE_SENSOR 2
const char *SENSOR_NAME[NB_TYPE_SENSOR] = {"DHT11", "DHT22"};
const uint8_t SENSOR_ID[NB_TYPE_SENSOR] = {DHT11_DEVICE, DHT22_DEVICE};
const DHTesp::DHT_MODEL_t SENSOR_TYPE[NB_TYPE_SENSOR] = {DHTesp::DHT11,
                                                         DHTesp::DHT22};
DHTesp *dht_device;

DHTSensorDevice::DHTSensorDevice() { dht_device = nullptr; }

DHTSensorDevice::~DHTSensorDevice() { end(); }

bool DHTSensorDevice::begin() {
  end();
  uint8_t dhttype = ESP3DSettings::readByte(ESP_SENSOR_TYPE);
  esp3d_log("Read %d, %s", dhttype,
            dhttype == 1   ? "DHT11"
            : dhttype == 2 ? "DHT22"
            : dhttype == 0 ? "NONE"
                           : "Unknow type");
  if (dhttype == 0) {
    esp3d_log("No Sensor active");
    return true;
  }
  if (!isModelValid(dhttype)) {
    esp3d_log_e("No valid id ");
    return false;
  }
  dht_device = new DHTesp;
  if (!dht_device) {
    esp3d_log_e("Cannot instanciate dht");
    return false;
  }
  esp3d_log("DHT PIN %d", ESP3D_SENSOR_PIN);
  dht_device->setup(ESP3D_SENSOR_PIN, (DHTesp::DHT_MODEL_t)dhttype);
  if (strcmp(dht_device->getStatusString(), "OK") != 0) {
    esp3d_log_e("No valid dht status: %d,  %s", dht_device->getStatus(),
                dht_device->getStatusString());
    return false;
  }
  esp3d_log("DHT ok");
  return true;
}

void DHTSensorDevice::end() {
  if (dht_device) {
    delete dht_device;
  }
  dht_device = nullptr;
}

bool DHTSensorDevice::isModelValid(uint8_t model) {
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    if (model == SENSOR_ID[i]) {
      return true;
    }
  }
  return false;
}

uint8_t DHTSensorDevice::getIDFromString(const char *s) {
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    esp3d_log("checking %s with %s", s, SENSOR_NAME[i]);
    if (strcmp(s, SENSOR_NAME[i]) == 0) {
      esp3d_log("found %d", SENSOR_ID[i]);
      return SENSOR_ID[i];
    }
  }

  return 0;
}

uint8_t DHTSensorDevice::nbType() { return NB_TYPE_SENSOR; }

uint8_t DHTSensorDevice::GetModel(uint8_t i) {
  if (i < NB_TYPE_SENSOR) {
    return SENSOR_ID[i];
  }
  return 0;
}

const char *DHTSensorDevice::GetCurrentModelString() {
  uint8_t dhttype = ESP3DSettings::readByte(ESP_SENSOR_TYPE);
  for (uint8_t i = 0; i < NB_TYPE_SENSOR; i++) {
    if ((DHTesp::DHT_MODEL_t)dhttype == SENSOR_TYPE[i]) {
      return SENSOR_NAME[i];
    }
  }
  return "NONE";
}

const char *DHTSensorDevice::GetModelString(uint8_t i) {
  if (i < NB_TYPE_SENSOR) {
    return SENSOR_NAME[i];
  }
  return "NONE";
}

const char *DHTSensorDevice::GetData() {
  static String s;
  if (dht_device) {
    float temperature = dht_device->getTemperature();
    float humidity = dht_device->getHumidity();
    esp3d_log("T %f H %f", temperature, humidity);
    if (strcmp(SENSOR__UNIT, "F") == 0) {
      temperature = dht_device->toFahrenheit(temperature);
    }
    if (String(humidity, 1) != "nan") {
      s = String(temperature, 1);
      s += "[";
      s += SENSOR__UNIT;
      s += "] " + String(humidity, 1) + "[%]";
    } else {
      s = "DISCONNECTED";
      esp3d_log_e("No valid data");
    }
  } else {
    s = "DISCONNECTED";
    esp3d_log_e("No device");
  }
  return s.c_str();
}

#endif  // DHT11_DEVICE || DHT22_DEVICE
#endif  // SENSOR_DEVICE
