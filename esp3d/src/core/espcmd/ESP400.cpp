/*
 ESP400.cpp - ESP3D command class

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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/serial/serial_service.h"
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/authentication/authentication_service.h"
#if defined(SENSOR_DEVICE)
#include "../../modules/sensor/sensor.h"
#endif  // SENSOR_DEVICE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE
#define COMMANDID 400

const char* YesNoLabels[] = {"yes", "no"};
const char* YesNoValues[] = {"1", "0"};
const char* RadioModeLabels[] = {"none"
#ifdef WIFI_FEATURE
                                 ,
                                 "sta",
                                 "ap",
                                 "setup"
#endif  // WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
                                 ,
                                 "bt"
#endif  // BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
                                 ,
                                 "eth-sta"
#endif  // ETH_FEATURE
};
const char* RadioModeValues[] = {"0"
#ifdef WIFI_FEATURE
                                 ,
                                 "1",
                                 "2",
                                 "5"
#endif  // WIFI_FEATURE

#ifdef BLUETOOTH_FEATURE
                                 ,
                                 "3"

#endif  // BLUETOOTH_FEATURE

#ifdef ETH_FEATURE
                                 ,
                                 "4"
#endif  // ETH_FEATURE
};

const char* FallbackLabels[] = {"none"
#ifdef WIFI_FEATURE
                                ,
                                "setup"
#endif  // WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
                                ,
                                "bt"
#endif  // BLUETOOTH_FEATURE
};
const char* FallbackValues[] = {"0"
#ifdef WIFI_FEATURE
                                ,
                                "5"
#endif  // WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
                                ,
                                "3"
#endif  // BLUETOOTH_FEATURE
};

const char* FirmwareLabels[] = {"Unknown", "Grbl", "Marlin", "Smoothieware",
                                "Repetier"};

const char* FirmwareValues[] = {"0", "10", "20", "40", "50"};
#ifdef NOTIFICATION_FEATURE
const char* NotificationsLabels[] = {"none", "pushover", "email",
                                     "line", "telegram", "ifttt"};

const char* NotificationsValues[] = {"0", "1", "2", "3", "4", "5"};
#endif  // NOTIFICATION_FEATURE

const char* IpModeLabels[] = {"static", "dhcp"};
const char* IpModeValues[] = {"0", "1"};

const char* SupportedApChannelsStr[] = {"1", "2", "3",  "4",  "5",  "6",  "7",
                                        "8", "9", "10", "11", "12", "13", "14"};

const char* SupportedBaudListSizeStr[] = {
    "9600",   "19200",  "38400",  "57600",  "74880",  "115200",
    "230400", "250000", "500000", "921600", "1958400"};

#ifdef SENSOR_DEVICE

const char* SensorLabels[] = {"none"
#if SENSOR_DEVICE == DHT11_DEVICE || SENSOR_DEVICE == DHT22_DEVICE
                              ,
                              "DHT11",
                              "DHT22"

#endif  // SENSOR_DEVICE == DHT11_DEVICE || SENSOR_DEVICE == DHT22_DEVICE
#if SENSOR_DEVICE == ANALOG_DEVICE
                              ,
                              "Analog"
#endif  // SENSOR_DEVICE == ANALOG_DEVICE
#if SENSOR_DEVICE == BMP280_DEVICE || SENSOR_DEVICE == BME280_DEVICE
                              ,
                              "BMP280",
                              "BME280"
#endif  // SENSOR_DEVICE == BMP280_DEVICE || SENSOR_DEVICE == BME280_DEVICE
};
const char* SensorValues[] = {"0"
#if SENSOR_DEVICE == DHT11_DEVICE || SENSOR_DEVICE == DHT22_DEVICE
                              ,
                              "1",
                              "2"
#endif  // SENSOR_DEVICE == DHT11_DEVICE || SENSOR_DEVICE == DHT22_DEVICE

#if SENSOR_DEVICE == ANALOG_DEVICE
                              ,
                              "3"
#endif  // SENSOR_DEVICE == ANALOG_DEVICE
#if SENSOR_DEVICE == BMP280_DEVICE || SENSOR_DEVICE == BME280_DEVICE
                              ,
                              "4",
                              "5"
#endif  // SENSOR_DEVICE == BMP280_DEVICE || SENSOR_DEVICE == BME280_DEVICE

};

#endif  // SENSOR_DEVICE

#ifdef SD_DEVICE
#if SD_DEVICE != ESP_SDIO
const char* SupportedSPIDividerStr[] = {"1", "2", "4", "6", "8", "16", "32"};
const uint8_t SupportedSPIDividerStrSize =
    sizeof(SupportedSPIDividerStr) / sizeof(char*);
#endif  // SD_DEVICE != ESP_SDIO
#endif  // SD_DEVICE

// Get full ESP3D settings
//[ESP400]<pwd=admin>
bool Commands::ESP400(const char* cmd_params, level_authenticate_type auth_type,
                      ESP3DOutput* output) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  uint8_t count = 0;
  const long* bl = NULL;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead
#ifdef AUTHENTICATION_FEATURE
  if (auth_type != LEVEL_ADMIN) {
    response =
        format_response(COMMANDID, json, false, "Wrong authentication level");
    noError = false;
    errorCode = 401;
    if (json) {
      output->printLN(response.c_str());
    } else {
      if (noError) {
        output->printERROR(response.c_str(), errorCode);
      }
    }
    return false;
  }
#else
  (void)auth_type;
#endif  // AUTHENTICATION_FEATURE
  if (json) {
    output->print("{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[");
  } else {
    output->println("Settings:");
  }
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  // Hostname network/network
  _dispatchSetting(json, "network/network", ESP_HOSTNAME, "hostname", NULL,
                   NULL, 32, 1, 1, -1, NULL, true, output, true);
#endif  // WIFI_FEATURE || ETH_FEATURE || BT_FEATURE

  // radio mode network/network
  _dispatchSetting(json, "network/network", ESP_RADIO_MODE, "radio mode",
                   RadioModeValues, RadioModeLabels,
                   sizeof(RadioModeValues) / sizeof(char*), -1, -1, -1, NULL,
                   true, output);

  // Radio State at Boot
  _dispatchSetting(json, "network/network", ESP_BOOT_RADIO_STATE, "radio_boot",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, NULL, true,
                   output);
#ifdef WIFI_FEATURE
  // STA SSID network/sta
  _dispatchSetting(json, "network/sta", ESP_STA_SSID, "SSID", nullptr, nullptr,
                   32, 1, 1, -1, nullptr, true, output);

  // STA Password network/sta
  _dispatchSetting(json, "network/sta", ESP_STA_PASSWORD, "pwd", nullptr,
                   nullptr, 64, 8, 0, -1, nullptr, true, output);

#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  // STA IP mode
  _dispatchSetting(json, "network/sta", ESP_STA_IP_MODE, "ip mode",
                   IpModeValues, IpModeLabels,
                   sizeof(IpModeLabels) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
  // STA static IP
  _dispatchSetting(json, "network/sta", ESP_STA_IP_VALUE, "ip", nullptr,
                   nullptr, -1, -1, -1, -1, nullptr, true, output);

  // STA static Gateway
  _dispatchSetting(json, "network/sta", ESP_STA_GATEWAY_VALUE, "gw", nullptr,
                   nullptr, -1, -1, -1, -1, nullptr, true, output);
  // STA static Mask
  _dispatchSetting(json, "network/sta", ESP_STA_MASK_VALUE, "msk", nullptr,
                   nullptr, -1, -1, -1, -1, nullptr, true, output);
  // STA static DNS
  _dispatchSetting(json, "network/sta", ESP_STA_DNS_VALUE, "DNS", nullptr,
                   nullptr, -1, -1, -1, -1, nullptr, true, output);

#endif  // WIFI_FEATURE || ETH_FEATURE

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  // Sta fallback mode
  _dispatchSetting(json, "network/sta", ESP_STA_FALLBACK_MODE,
                   "sta fallback mode", FallbackValues, FallbackLabels,
                   sizeof(FallbackValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
  // AP SSID network/ap
  _dispatchSetting(json, "network/ap", ESP_AP_SSID, "SSID", nullptr, nullptr,
                   32, 1, 1, -1, nullptr, true, output);

  // AP password
  _dispatchSetting(json, "network/ap", ESP_AP_PASSWORD, "pwd", nullptr, nullptr,
                   64, 8, 0, -1, nullptr, true, output);
  // AP static IP
  _dispatchSetting(json, "network/ap", ESP_AP_IP_VALUE, "ip", nullptr, nullptr,
                   -1, -1, -1, -1, nullptr, true, output);

  // AP Channel
  _dispatchSetting(json, "network/ap", ESP_AP_CHANNEL, "channel",
                   SupportedApChannelsStr, SupportedApChannelsStr,
                   sizeof(SupportedApChannelsStr) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
#endif  // WIFI_FEATURE

#ifdef AUTHENTICATION_FEATURE
  // Admin password
  _dispatchSetting(json, "security/security", ESP_ADMIN_PWD, "adm pwd", nullptr,
                   nullptr, 20, 0, -1, -1, nullptr, true, output);
  // User password
  _dispatchSetting(json, "security/security", ESP_USER_PWD, "user pwd", nullptr,
                   nullptr, 20, 0, -1, -1, nullptr, true, output);

  // session timeout
  _dispatchSetting(json, "security/security", ESP_SESSION_TIMEOUT,
                   "session timeout", nullptr, nullptr, 255, 0, -1, -1, nullptr,
                   true, output);

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // Secure Serial
  _dispatchSetting(json, "security/security", ESP_SECURE_SERIAL, "serial",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

#endif  // COMMUNICATION_PROTOCOL
#endif  // AUTHENTICATION_FEATURE
#ifdef HTTP_FEATURE
  // HTTP On service/http
  _dispatchSetting(json, "service/http", ESP_HTTP_ON, "enable", YesNoValues,
                   YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
  // HTTP port
  _dispatchSetting(json, "service/http", ESP_HTTP_PORT, "port", nullptr,
                   nullptr, 65535, 1, -1, -1, nullptr, true, output);
#endif  // HTTP_FEATURE

#ifdef TELNET_FEATURE
  // TELNET On service/telnet
  _dispatchSetting(json, "service/telnetp", ESP_TELNET_ON, "enable",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // TELNET Port
  _dispatchSetting(json, "service/telnetp", ESP_TELNET_PORT, "port", nullptr,
                   nullptr, 65535, 1, -1, -1, nullptr, true, output);
#endif  // TELNET_FEATURE

#ifdef WS_DATA_FEATURE
  // Websocket On service
  _dispatchSetting(json, "service/websocketp", ESP_WEBSOCKET_ON, "enable",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // Websocket Port
  _dispatchSetting(json, "service/websocketp", ESP_WEBSOCKET_PORT, "port",
                   nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, output);
#endif  // WS_DATA_FEATURE

#ifdef WEBDAV_FEATURE
  // WebDav On service
  _dispatchSetting(json, "service/webdavp", ESP_WEBDAV_ON, "enable",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // WebDav Port
  _dispatchSetting(json, "service/webdavp", ESP_WEBDAV_PORT, "port", nullptr,
                   nullptr, 65535, 1, -1, -1, nullptr, true, output);
#endif  // WEBDAV_FEATURE

#ifdef FTP_FEATURE
  // FTP On service/ftp
  _dispatchSetting(json, "service/ftp", ESP_FTP_ON, "enable", YesNoValues,
                   YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);

  // FTP Ports
  // CTRL Port
  _dispatchSetting(json, "service/ftp", ESP_FTP_CTRL_PORT, "control port",
                   nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, output);

  // Active Port
  _dispatchSetting(json, "service/ftp", ESP_FTP_DATA_ACTIVE_PORT, "active port",
                   nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, output);

  // Passive Port
  _dispatchSetting(json, "service/ftp", ESP_FTP_DATA_PASSIVE_PORT,
                   "passive port", nullptr, nullptr, 65535, 1, -1, -1, nullptr,
                   true, output);
#endif  // FTP_FEATURE

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  // Serial bridge On service
  _dispatchSetting(json, "service/serial_bridge", ESP_SERIAL_BRIDGE_ON,
                   "enable", YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // Baud Rate
  _dispatchSetting(json, "service/serial_bridge", ESP_SERIAL_BRIDGE_BAUD,
                   "baud", SupportedBaudListSizeStr, SupportedBaudListSizeStr,
                   sizeof(SupportedBaudListSizeStr) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#ifdef TIMESTAMP_FEATURE

  // Internet Time
  _dispatchSetting(json, "service/time", ESP_INTERNET_TIME, "i-time",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // Time zone
  _dispatchSetting(json, "service/time", ESP_TIME_ZONE, "tzone",
                   SupportedTimeZones, SupportedTimeZones,
                   SupportedTimeZonesSize, -1, -1, -1, nullptr, true, output);

  // Time Server1
  _dispatchSetting(json, "service/time", ESP_TIME_SERVER1, "t-server", nullptr,
                   nullptr, 127, 0, -1, -1, nullptr, true, output);

  // Time Server2
  _dispatchSetting(json, "service/time", ESP_TIME_SERVER2, "t-server", nullptr,
                   nullptr, 127, 0, -1, -1, nullptr, true, output);

  // Time Server3
  _dispatchSetting(json, "service/time", ESP_TIME_SERVER3, "t-server", nullptr,
                   nullptr, 127, 0, -1, -1, nullptr, true, output);
#endif  // TIMESTAMP_FEATURE

#ifdef NOTIFICATION_FEATURE
  // Auto notification
  _dispatchSetting(json, "service/notification", ESP_AUTO_NOTIFICATION,
                   "auto notif", YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

  // Notification type
  _dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TYPE,
                   "notification", NotificationsValues, NotificationsLabels,
                   sizeof(NotificationsValues) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);

  // Token 1
  _dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TOKEN1, "t1",
                   nullptr, nullptr, 63, 1, 1, -1, nullptr, true, output);

  // Token 2
  _dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TOKEN2, "t2",
                   nullptr, nullptr, 63, 1, 1, -1, nullptr, true, output);

  // Notifications Settings
  _dispatchSetting(json, "service/notification", ESP_NOTIFICATION_SETTINGS,
                   "ts", nullptr, nullptr, 128, 0, -1, -1, nullptr, true,
                   output);
#endif  // NOTIFICATION_FEATURE

#ifdef BUZZER_DEVICE
  // Buzzer state
  _dispatchSetting(json, "device/device", ESP_BUZZER, "buzzer", YesNoValues,
                   YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
#endif  // BUZZER_DEVICE

#ifdef SENSOR_DEVICE
  // Sensor type
  _dispatchSetting(json, "device/sensor", ESP_SENSOR_TYPE, "type", SensorValues,
                   SensorLabels, sizeof(SensorValues) / sizeof(char*), -1, -1,
                   -1, nullptr, true, output);

  // Sensor interval
  _dispatchSetting(json, "device/sensor", ESP_SENSOR_INTERVAL, "intervalms",
                   nullptr, nullptr, 60000, 0, -1, -1, nullptr, true, output);

#endif  // SENSOR_DEVICE
#if defined(SD_DEVICE)
#if SD_DEVICE != ESP_SDIO
  // SPI SD Divider
  _dispatchSetting(json, "device/sd", ESP_SD_SPEED_DIV, "speedx",
                   SupportedSPIDividerStr, SupportedSPIDividerStr,
                   SupportedSPIDividerStrSize, -1, -1, -1, nullptr, true,
                   output);
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
  // SD CHECK UPDATE AT BOOT feature
  _dispatchSetting(json, "device/sd", ESP_SD_CHECK_UPDATE_AT_BOOT, "SD updater",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // SD_UPDATE_FEATURE
#endif  // SD_DEVICE

#if !defined(FIXED_FW_TARGET)
  // Target FW
  _dispatchSetting(json, "system/system", ESP_TARGET_FW, "targetfw",
                   FirmwareValues, FirmwareLabels,
                   sizeof(FirmwareValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // FIXED_FW_TARGET
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // Baud Rate
  _dispatchSetting(json, "system/system", ESP_BAUD_RATE, "baud",
                   SupportedBaudListSizeStr, SupportedBaudListSizeStr,
                   sizeof(SupportedBaudListSizeStr) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL

  // Start delay
  _dispatchSetting(json, "system/boot", ESP_BOOT_DELAY, "bootdelay", nullptr,
                   nullptr, 40000, 0, -1, -1, nullptr, true, output);

  // Verbose boot
  _dispatchSetting(json, "system/boot", ESP_VERBOSE_BOOT, "verbose",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);

// Output flag
// Serial
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || \
    COMMUNICATION_PROTOCOL == MKS_SERIAL || \
    COMMUNICATION_PROTOCOL == SOCKET_SERIAL

  _dispatchSetting(json, "system/outputmsg", ESP_SERIAL_FLAG, "serial",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL ||

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  _dispatchSetting(json, "system/outputmsg", ESP_SERIAL_BRIDGE_FLAG,
                   "serial_bridge", YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#if (defined(ESP3DLIB_ENV) && defined(HAS_DISPLAY)) || \
    defined(HAS_SERIAL_DISPLAY)
  _dispatchSetting(json, "system/outputmsg", ESP_REMOTE_SCREEN_FLAG, "M117",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // ESP3DLIB_ENV

#ifdef DISPLAY_DEVICE
  _dispatchSetting(json, "system/outputmsg", ESP_SCREEN_FLAG, "M117",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // DISPLAY_DEVICE

#ifdef WS_DATA_FEATURE
  _dispatchSetting(json, "system/outputmsg", ESP_WEBSOCKET_FLAG, "ws",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // WS_DATA_FEATURE

#ifdef BLUETOOTH_FEATURE
  _dispatchSetting(json, "system/outputmsg", ESP_BT_FLAG, "BT", YesNoValues,
                   YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                   nullptr, true, output);
#endif  // BLUETOOTH_FEATURE

#ifdef TELNET_FEATURE
  _dispatchSetting(json, "system/outputmsg", ESP_TELNET_FLAG, "telnet",
                   YesNoValues, YesNoLabels,
                   sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                   true, output);
#endif  // TELNET_FEATURE

  if (json) {
    output->print("]}");
  } else {
    output->println("ok");
  }
  return true;
}
