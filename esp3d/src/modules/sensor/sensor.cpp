/*
  sensor.cpp -  sensor functions class

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
#include "sensor.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
//Include file according sensor
#if SENSOR_DEVICE==DHT11_DEVICE || SENSOR_DEVICE==DHT22_DEVICE
#include "dht.h"
#endif //DHT11_DEVICE || DHT22_DEVICE
#if SENSOR_DEVICE==ANALOG_DEVICE
#include "analogsensor.h"
#endif //ANALOG_DEVICE
#if SENSOR_DEVICE==BMP280_DEVICE || SENSOR_DEVICE==BME280_DEVICE
#include "bmx280.h"
#endif //BMP280_DEVICE || BME280_DEVICE

#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../websocket/websocket_server.h"
#endif // WIFI_FEATURE || ETH_FEATURE

ESP3DSensor esp3d_sensor;

ESP3DSensor::ESP3DSensor()
{
    _started = false;
    _interval = 0;
    _device = nullptr;
}

ESP3DSensor::~ESP3DSensor()
{
    end();
}

bool ESP3DSensor::begin()
{
    log_esp3d("Sensor Begin");
    bool res = true;
    end();
    //new _device
#if SENSOR_DEVICE==ANALOG_DEVICE
    _device = (ESP3DSensorDevice * )new AnalogSensorDevice();
#endif //ANALOG_DEVICE
#if SENSOR_DEVICE==DHT11_DEVICE || SENSOR_DEVICE==DHT22_DEVICE
    _device = (ESP3DSensorDevice * )new DHTSensorDevice();
#endif //DHT11_DEVICE || DHT22_DEVICE
#if SENSOR_DEVICE==BMP280_DEVICE || SENSOR_DEVICE==BME280_DEVICE
    _device = (ESP3DSensorDevice * )new BMX280SensorDevice();
#endif //DHT11_DEVICE || DHT22_DEVICE
    if (!_device) {
        log_esp3d("No device created");
        return false;
    }
    log_esp3d("Sensor Device created");
    uint8_t sensortype= Settings_ESP3D::read_byte(ESP_SENSOR_TYPE);
    log_esp3d("Sensor %d", sensortype);
    //No Sensor defined - exit is not an error
    if (sensortype == 0) {
        log_esp3d("Sensor Device is not active at start");
        return true;
    }
    _interval = Settings_ESP3D::read_uint32(ESP_SENSOR_INTERVAL);
    if (!_device->begin()) {
        res = false;
    }
    _lastReadTime = millis();
    _started = res;
    return _started;
}

void ESP3DSensor::end()
{
    if (_device) {
        delete _device;
        _device = nullptr;
    }
    _started = false;
    _interval = 0;
}

uint8_t ESP3DSensor::GetModel(uint8_t i)
{
    if (_device) {
        return _device->GetModel(i);
    } else {
        return 0;
    }
}

uint8_t ESP3DSensor::nbType()
{
    if (_device) {

        return _device->nbType();
    }
    return 0;
}


bool ESP3DSensor::isModelValid(uint8_t model)
{
    if (_device) {
        return _device->isModelValid(model);
    }
    return false;
}

const char * ESP3DSensor::GetModelString(uint8_t i)
{
    if (_device) {

        return _device->GetModelString(i);
    }
    return "NONE";
}

uint8_t ESP3DSensor::getIDFromString(const char * s)
{
    if (_device) {
        return _device->getIDFromString(s);
    }
    return 0;
}

bool ESP3DSensor::setInterval(uint interval)
{
    _interval = interval;
    return true;
}

const char * ESP3DSensor::GetData()
{
    if (_started && _device) {
        return  _device->GetData();
    }
    return "";
}

void ESP3DSensor::handle()
{
    if (_interval == 0) {
        return;
    }
    if (_started && _device) {
        if ((millis() - _lastReadTime) > _interval) {
            String data = _device->GetData();
            _lastReadTime = millis();
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
            String s = "SENSOR:" + data ;
            websocket_terminal_server.pushMSG(s.c_str());
#endif // WIFI_FEATURE || ETH_FEATURE
        }
    }
}

#endif //SENSOR_DEVICE
