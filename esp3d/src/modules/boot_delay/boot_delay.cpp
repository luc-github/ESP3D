/*
  boot_delay.cpp -  boot delay functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "../../include/esp3d_config.h"
#include "boot_delay.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"

BootDelay::BootDelay()
{
    _started = false;
    _callbackfn = nullptr;
    _startdelay = 0;
    _totalduration = 0;
}
BootDelay::~BootDelay()
{
    end();
}

bool BootDelay::started()
{
    return _started;
}

bool BootDelay::begin(progress_t* fn)
{
    _totalduration = Settings_ESP3D::read_uint32(ESP_BOOT_DELAY);
    log_esp3d("Boot delay %d", _totalduration);
    if (_totalduration > Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY)) {
        _totalduration = Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY);
        log_esp3d("Boot delay modified %d", _totalduration);
    }
    _callbackfn = fn;
    _started = true;
    if(_callbackfn) {
        (*_callbackfn)(0);
    }
    if (_totalduration > 0) {
        _startdelay = millis();
        handle();
    }
    if(_callbackfn) {
        (*_callbackfn)(100);
    }
    log_esp3d("Boot delay done");
    return _started;
}
void BootDelay::end()
{
}

void BootDelay::handle()
{
    uint8_t lastpercent = 0;
    uint32_t lastSent = millis();
    while ((millis() - _startdelay) < _totalduration) {
        //to avoid overfload 2x/sec is enough for progression
        if ((millis() - lastSent) > 500) {
            lastSent = millis();
            uint8_t p = (100*(millis() - _startdelay))/_totalduration;
            if (p != lastpercent) {
                lastpercent=p;
                if(_callbackfn && p!=100) {
                    (*_callbackfn)(p);
                }
            }
        }
        Hal::wait(10);
    }
}
