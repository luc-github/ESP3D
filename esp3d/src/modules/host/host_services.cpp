/*
  host_services.cpp -  host services functions class

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
#include "host_services.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../serial/serial_service.h"

HostServices::HostServices()
{
    _started = false;
}
HostServices::~HostServices()
{
    end();
}

bool HostServices::begin()
{
    bool res = true;
    end();
    //Check autostart.g on SPIFFS
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}
void HostServices::end()
{
    if(!_started) {
        return;
    }
    _started = false;

}

void HostServices::handle()
{
    if (_started) {
    }
}


bool HostServices::purge_serial()
{
    uint32_t start = millis();
    uint8_t buf [51];
    serial_service.flush();
    log_esp3d("Purge Serial");
    while (serial_service.available() > 0 ) {
        if ((millis() - start ) > 2000) {
            log_esp3d("Purge timeout");
            return false;
        }
        size_t len = serial_service.readBytes (buf, 50);
        buf[len] = '\0';
        log_esp3d ("Purge: %s",(const char *)buf);
        if ( (Settings_ESP3D::GetFirmwareTarget() == REPETIER4DV) || (Settings_ESP3D::GetFirmwareTarget() == REPETIER) ) {
            String s = (const char *)buf;
            //repetier never stop sending data so no need to wait if have 'wait' or 'busy'
            if((s.indexOf ("wait") > -1) || (s.indexOf ("busy") > -1)) {
                return true;
            }
            log_esp3d("Purge interrupted");
        }
        Hal::wait (5);
    }
    Hal::wait (0);
    log_esp3d("Purge done");
    return true;
}
