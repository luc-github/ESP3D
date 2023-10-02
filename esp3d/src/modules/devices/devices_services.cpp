/*
  devices_services.cpp -  devices services functions class

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
#ifdef CONNECTED_DEVICES_FEATURE
#include "devices_services.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"

#ifdef DISPLAY_DEVICE
#include "../display/display.h"
#endif //DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
#include "../sensor/sensor.h"
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
#include "../buzzer/buzzer.h"
#endif //BUZZER_DEVICE
#ifdef RECOVERY_FEATURE
#include "../recovery/recovery_service.h"
#endif //RECOVERY_FEATURE
#ifdef CAMERA_DEVICE
#include "../camera/camera.h"
#endif //CAMERA_DEVICE
#ifdef SD_DEVICE
#include "../filesystem/esp_sd.h"
#endif //SD_DEVICE

bool DevicesServices::_started = false;

bool DevicesServices::begin()
{
    bool res = true;
    _started = false;
#ifdef SD_DEVICE
    if (!ESP_SD::begin()) {
        log_esp3d("Error sd intialization failed");
        res = false;
    }
#endif //SD_DEVICE
#ifdef DISPLAY_DEVICE
    if (!esp3d_display.begin()) {
        log_esp3d("Error starting display device");
        res = false;
    }
#endif //DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
    if (!esp3d_sensor.begin()) {
        log_esp3d("Error starting sensor device");
        res = false;
    }
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
    if (!esp3d_buzzer.begin()) {
        log_esp3d("Error starting buzzer device");
        res = false;
    }
#endif //BUZZER_DEVICE
#ifdef RECOVERY_FEATURE
    if (!recovery_service.begin()) {
        log_esp3d("Error starting recovery service");
        res = false;
    }
#endif //RECOVERY_FEATURE
#ifdef CAMERA_DEVICE
    if (!esp3d_camera.initHardware()) {
        log_esp3d("Error camera intialization failed");
        res = false;
    }
#endif //CAMERA_DEVICE
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
#ifdef SD_DEVICE
    ESP_SD::end();
#endif //SD_DEVICE
#ifdef CAMERA_DEVICE
    esp3d_camera.stopHardware();
#endif //CAMERA_DEVICE
#ifdef RECOVERY_FEATURE
    recovery_service.end();
#endif //RECOVERY_FEATURE
#ifdef BUZZER_DEVICE
    esp3d_buzzer.end();
#endif //BUZZER_DEVICE
#ifdef DISPLAY_DEVICE
    esp3d_display.end();
#endif //DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
    esp3d_sensor.end();
#endif //SENSOR_DEVICE
}

void DevicesServices::handle()
{
    if (_started) {
#ifdef CAMERA_DEVICE
        esp3d_camera.handle();
#endif //CAMERA_DEVICE
#ifdef DISPLAY_DEVICE
        esp3d_display.handle();
#endif //DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
        esp3d_sensor.handle();
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
        esp3d_buzzer.handle();
#endif //BUZZER_DEVICE
#ifdef RECOVERY_FEATURE
        recovery_service.handle();
#endif //RECOVERY_FEATURE
#ifdef SD_DEVICE
        ESP_SD::handle();
#endif //SD_DEVICE
    }
}

#endif //#CONNECTED_DEVICES_FEATURE
