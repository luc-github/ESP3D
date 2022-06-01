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
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../modules/serial/serial_service.h"
#endif // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if COMMUNICATION_PROTOCOL ==SOCKET_SERIAL
#include "../modules/serial2socket/serial2socket.h"
#endif // COMMUNICATION_PROTOCOL ==SOCKET_SERIAL
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#endif //WIFI_FEATURE || ETH FEATURE
#if defined(FILESYSTEM_FEATURE)
#include "../modules/filesystem/esp_filesystem.h"
#endif //FILESYSTEM_FEATURE
#ifdef CONNECTED_DEVICES_FEATURE
#include "../modules/devices/devices_services.h"
#endif //CONNECTED_DEVICES_FEATURE
#ifdef DISPLAY_DEVICE
#include "../modules/display/display.h"
#endif //DISPLAY_DEVICE
#ifdef GCODE_HOST_FEATURE
#include "../modules/gcode_host/gcode_host.h"
#endif //GCODE_HOST_FEATURE
#ifdef ESP_LUA_INTERPRETER_FEATURE
#include "../modules/lua_interpreter/lua_interpreter_service.h"
#endif //#ifdef 
#ifdef  SD_UPDATE_FEATURE
#include "../modules/update/update_service.h"
#endif // SD_UPDATE_FEATURE
#include "esp3doutput.h"
#include "../modules/boot_delay/boot_delay.h"


bool Esp3D::restart = false;

//Contructor
Esp3D::Esp3D()
{
    _started = false;
}

//Destructor
Esp3D::~Esp3D()
{
    end();
}

//Begin which setup everything
bool Esp3D::begin()
{
    BootDelay bd;
    Hal::begin();
    DEBUG_ESP3D_INIT
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    Serial2Socket.enable();
#endif // COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    //init output
    ESP3DOutput::isOutput(ESP_ALL_CLIENTS, true);
    bool res = true;
#if defined(CONNECTED_DEVICES_FEATURE)
    if (!DevicesServices::begin()) {
        log_esp3d("Error setup connected devices");
        res = false;
    }
#endif //CONNECTED_DEVICES_FEATURE
    //delay to avoid to disturb printer
    bd.begin();
#ifdef  SD_UPDATE_FEATURE
    if (update_service.begin()) {
        log_esp3d("Need restart due to update");
        //no need to continue as there was an update
        restart_now();
    }
#endif // SD_UPDATE_FEATURE
    log_esp3d("Mode %d", WiFi.getMode());
    if (!Settings_ESP3D::begin()) {
        log_esp3d("Need reset settings");
        reset();
        //Restart ESP3D
        restart_now();
    }
    //BT do not start automaticaly so should be OK
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    //Serial service
    if (!serial_service.begin()) {
        log_esp3d("Error with serial service");
        res = false;
    }
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    //Setup Filesystem
#if defined(FILESYSTEM_FEATURE)
    if (!ESP_FileSystem::begin()) {
        log_esp3d("Error with filesystem service");
        res = false;
    }
#endif //FILESYSTEM_FEATURE
#ifdef DISPLAY_DEVICE
    esp3d_display.showScreenID(MAIN_SCREEN);
    log_esp3d("Main screen");
#endif //DISPLAY_DEVICE
    //Setup Network
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
    if (Settings_ESP3D::read_byte(ESP_BOOT_RADIO_STATE) == 1) {
        if (!NetConfig::begin()) {
            log_esp3d("Error setup network");
            res = false;
        }
    }

#endif //WIFI_FEATURE
#if defined(GCODE_HOST_FEATURE)
#if defined(ESP_AUTOSTART_SCRIPT)
    esp3d_gcode_host.processScript(ESP_AUTOSTART_SCRIPT);
#endif //ESP_AUTOSTART_FEATURE
#if defined(ESP_AUTOSTART_SCRIPT_FILE)
    esp3d_gcode_host.processFile(ESP_AUTOSTART_SCRIPT_FILE);
#endif //ESP_AUTOSTART_FEATURE
#endif //GCODE_HOST_FEATURE
    _started=true;
    return res;
}

//Process which handle all input
void Esp3D::handle()
{
    if(!_started) {
        return;
    }
    //if need restart
    if (restart) {
        restart_now();
    }
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    serial_service.handle();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL ==SOCKET_SERIAL
    Serial2Socket.handle();
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL 
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    NetConfig::handle();
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(CONNECTED_DEVICES_FEATURE)
    DevicesServices::handle();
#endif //CONNECTED_DEVICES_FEATURE
#if defined(GCODE_HOST_FEATURE)
    esp3d_gcode_host.handle();
#endif //GCODE_HOST_FEATURE
}

bool Esp3D::started()
{
    return _started;
}

//End ESP3D
bool Esp3D::end()
{
    _started = false;
#if defined(CONNECTED_DEVICES_FEATURE)
    DevicesServices::end();
#endif //CONNECTED_DEVICES_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    NetConfig::end();
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(FILESYSTEM_FEATURE)
    ESP_FileSystem::end();
#endif //FILESYSTEM_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    serial_service.end();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    return true;
}

//Reset ESP3D settings
bool Esp3D::reset()
{
    bool resetOk = true;
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    if (!serial_service.reset()) {
        resetOk = false;
        log_esp3d("Reset serial error");
    }
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    if (!serial_service.started()) {
        serial_service.begin();
    }
    serial_service.flush();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(FILESYSTEM_FEATURE)
    ESP_FileSystem::end();
#endif //FILESYSTEM_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    serial_service.swap();
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    ESP.restart();
    while (1) {
        delay (1);
    }
}

