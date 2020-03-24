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
#ifdef DHT_DEVICE
#include "dht.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include <DHTesp.h>
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../websocket/websocket_server.h"
#endif // WIFI_FEATURE || ETH_FEATURE

DHT esp3d_DHT;
DHTesp  * dht_device;

DHT::DHT()
{
    _temperature =0;
    _humidity =0;
    _started = false;
    _interval = 0;
    dht_device = nullptr;
#if DHT_UNIT == USE_FAHRENHEIT
    _usecelsius = false;
#else
    _usecelsius = true;
#endif
}
DHT::~DHT()
{
    end();
}

bool DHT::begin()
{
    bool res = true;
    end();
    uint8_t dhttype= Settings_ESP3D::read_byte(ESP_DHT_TYPE);
    log_esp3d("DHT %d", dhttype);
    //No DHT defined - exit is not and error
    if (dhttype == 0) {
        return true;
    }
    //DHT type is  unknown - exit as error
    if (!((dhttype == DHT11_DEVICE) || (dhttype == DHT22_DEVICE))) {
        return false;
    }
    dht_device = new DHTesp;

    //dht_device = new DHTesp;
    if (!dht_device) {
        return false;
    }
    //no need to check pin because it is hard coded
    dht_device->setup(ESP3D_DHT_PIN, (DHTesp::DHT_MODEL_t)dhttype);
    _interval = Settings_ESP3D::read_uint32(ESP_DHT_INTERVAL);
    log_esp3d("DHT status %d", dht_device->getStatus());
    if (!dht_device->getStatus()) {
        res = false;
    }
    if (!res) {
        end();
    }
    _lastReadTime = millis();
    _started = res;
    return _started;
}

bool DHT::isCelsiusUnit()
{
    return _usecelsius;
}
void DHT::setCelsiusUnit(bool set)
{
    _usecelsius = set;
}

void DHT::end()
{
    if(!_started) {
        return;
    }
    if (dht_device) {
        delete dht_device;
        dht_device = nullptr;
    }
    _started = false;
    _interval = 0;
    _temperature =0;
    _humidity =0;
}

uint8_t DHT::GetModel()
{
    if (_started) {
        return dht_device->getModel();
    } else {
        return 0;
    }
}

float DHT::getHumidity()
{
    if (_started) {
        return _humidity;
    }
    return 0.0;
}

float DHT::getTemperature()
{
    if (_started) {
        if (_usecelsius) {
            return _temperature;
        } else {
            return dht_device->toFahrenheit(dht_device->getTemperature());
        }

    }
    return 0.0;
}


const char * DHT::GetModelString()
{
    if (_started) {
        if (dht_device->getModel() == 1) {
            return "DHT11";
        } else {
            return "DHT22";
        }
    }
    return "NONE";
}

bool DHT::started()
{
    return _started;
}

bool DHT::setInterval(uint interval)
{
    _interval = interval;
    return true;
}
uint DHT::interval()
{
    return _interval;
}

void DHT::handle()
{
    if (_started) {
        if ((millis() - _lastReadTime) > _interval) {
            _temperature = dht_device->getTemperature();
            _humidity = dht_device->getHumidity();
            _lastReadTime = millis();
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
            String s = "DHT:" ;
            s += String(_humidity,1);
            if (s !="nan") {
                s+="% ";
                s+= String(_temperature,1);
                s+= _usecelsius?"C ":"F ";
            } else {
                s ="DHT:Disconnected ";
            }
            websocket_terminal_server.pushMSG(s.c_str());
#endif // WIFI_FEATURE || ETH_FEATURE
        }
    }
}

#endif //DHT_DEVICE
