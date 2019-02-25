/*
    This file is part of ESP3D Firmware for 3D printer.

    ESP3D Firmware for 3D printer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP3D Firmware for 3D printer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Firmware.  If not, see <http://www.gnu.org/licenses/>.

    This firmware is using the standard arduino IDE with module to support ESP8266/ESP32:
    https://github.com/esp8266/Arduino
    https://github.com/espressif/arduino-esp32

    Latest version of the code and documentation can be found here :
    https://github.com/luc-github/ESP3D

    Main author: luc lebosse

*/
#include "esp3d.h"
#include "../include/esp3d_config.h"
#include "settings_esp3d.h"
#include "../modules/serial/serial_service.h"
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#endif //WIFI_FEATURE || ETH FEATURE
#if defined(FILESYSTEM_FEATURE)
#include "../modules/filesystem/esp_filesystem.h"
#endif //FILESYSTEM_FEATURE

#include "esp3doutput.h"

bool Esp3D::restart = false;

//Contructor
Esp3D::Esp3D()
{

}

//Destructor
Esp3D::~Esp3D()
{
    end();
}

//Begin which setup everything
bool Esp3D::begin(uint16_t startdelayms, uint16_t recoverydelayms)
{
    Hal::begin();
    //delay() to avoid to disturb printer
    delay(startdelayms);
    (void)recoverydelayms;
    DEBUG_ESP3D_INIT
    log_esp3d("Mode %d", WiFi.getMode());
    if (!Settings_ESP3D::begin()) {
        log_esp3d("Need reset settings");
        reset();
        //Restart ESP3D
        restart_esp();
    }
    //init output
    ESP3DOutput::isOutput(ESP_ALL_CLIENTS, true);
    //BT do not start automaticaly so should be OK
    //Serial service
    if (!serial_service.begin()) {
        log_esp3d("Error with serial service");
        return false;
    }
    //Setup Filesystem
#if defined(FILESYSTEM_FEATURE)
    if (!ESP_FileSystem::begin()) {
        log_esp3d("Error with filesystem service");
        return false;
    }
#endif //FILESYSTEM_FEATURE
    //Setup Network
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    if (!NetConfig::begin()) {
        log_esp3d("Error setup network");
        return false;
    }
#endif //WIFI_FEATURE
    return true;
}

//Process which handle all input
void Esp3D::handle()
{
    //if need restart
    if (restart) {
        restart_now();
    }
    serial_service.handle();
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    NetConfig::handle();
#endif //WIFI_FEATURE || ETH_FEATURE
}

//End ESP3D
bool Esp3D::end()
{
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    NetConfig::end();
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(FILESYSTEM_FEATURE)
    ESP_FileSystem::end();
#endif //FILESYSTEM_FEATURE
    serial_service.end();
    return true;
}

//Reset ESP3D settings
bool Esp3D::reset()
{
    bool resetOk = true;
    if (!serial_service.reset()) {
        resetOk = false;
        log_esp3d("Reset serial error");
    }
    if (!Settings_ESP3D::reset()) {
        log_esp3d("Reset settings error");
        resetOk = false;
    }
    return resetOk;
}

//Set Restart flag
void Esp3D::restart_esp(bool need_restart)
{
    restart = need_restart;
}

void Esp3D::restart_now()
{
    log_esp3d("Restarting");
    end();
    serial_service.swap();
    ESP.restart();
    while (1) {
        delay (1);
    };
}

