/*
  devices_services.cpp -  devices services functions class

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
#ifdef CONNECTED_DEVICES_FEATURE
#include "devices_services.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"

#ifdef DISPLAY_DEVICE
#include "../display/display.h"
#endif //DISPLAY_DEVICE
#ifdef DHT_DEVICE
#include "../dht/dht.h"
#endif //DHT_DEVICE
#ifdef RECOVERY_FEATURE
#include "../recovery/recovery_service.h"
#endif //RECOVERY_FEATURE

bool DevicesServices::_started = false;

DevicesServices::DevicesServices()
{
}
DevicesServices::~DevicesServices()
{
    end();
}

bool DevicesServices::begin()
{
    bool res = true;
    _started = false;
#ifdef DISPLAY_DEVICE
    if (!esp3d_display.begin()) {
        log_esp3d("Error starting display device");
        res = false;
    }
#endif //DISPLAY_DEVICE
#ifdef DHT_DEVICE
    if (!esp3d_DHT.begin()) {
        log_esp3d("Error starting DHT device");
        res = false;
    }
#endif //DHT_DEVICE
#ifdef RECOVERY_FEATURE
    if (!recovery_service.begin()) {
        log_esp3d("Error starting recorery service");
        res = false;
    }
#endif //RECOVERY_FEATURE
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}
void DevicesServices::end()
{
    if(!_started) {
        return;
    }
    _started = false;
#ifdef RECOVERY_FEATURE
    recovery_service.end();
#endif //RECOVERY_FEATURE
#ifdef DISPLAY_DEVICE
    esp3d_display.end();
#endif //DISPLAY_DEVICE
#ifdef DHT_DEVICE
    esp3d_DHT.end();
#endif //DHT_DDEVICE
}

void DevicesServices::handle()
{
    if (_started) {
#ifdef DISPLAY_DEVICE
        esp3d_display.handle();
#endif //DISPLAY_DEVICE
#ifdef DHT_DEVICE
        esp3d_DHT.handle();
#endif //DHT_DEVICE
#ifdef RECOVERY_FEATURE
        recovery_service.handle();
#endif //RECOVERY_FEATURE
    }
}

#endif //#CONNECTED_DEVICES_FEATURE
