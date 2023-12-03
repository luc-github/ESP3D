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
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
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
#define COMMAND_ID 400

const char* YesNoLabels[] = {"no", "yes"};
const char* YesNoValues[] = {"0", "1"};
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
                                     "line", "telegram", "ifttt", "home-assistant"};

const char* NotificationsValues[] = {"0", "1", "2", "3", "4", "5", "6"};
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
void ESP3DCommands::ESP400(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (json) {
    tmpstr = "{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[";

  } else {
    tmpstr = "Settings:\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  // Hostname network/network
  dispatchSetting(json, "network/network", ESP_HOSTNAME, "hostname", NULL, NULL,
                  32, 1, 1, -1, NULL, true, target, requestId, true);
#endif  // WIFI_FEATURE || ETH_FEATURE || BT_FEATURE

  // radio mode network/network
  dispatchSetting(json, "network/network", ESP_RADIO_MODE, "radio mode",
                  RadioModeValues, RadioModeLabels,
                  sizeof(RadioModeValues) / sizeof(char*), -1, -1, -1, NULL,
                  true, target, requestId);

  // Radio State at Boot
  dispatchSetting(json, "network/network", ESP_BOOT_RADIO_STATE, "radio_boot",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, NULL, true, target, requestId);
#ifdef WIFI_FEATURE
  // STA SSID network/sta
  dispatchSetting(json, "network/sta", ESP_STA_SSID, "SSID", nullptr, nullptr,
                  32, 1, 1, -1, nullptr, true, target, requestId);

  // STA Password network/sta
  dispatchSetting(json, "network/sta", ESP_STA_PASSWORD, "pwd", nullptr,
                  nullptr, 64, 8, 0, -1, nullptr, true, target, requestId);

#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  // STA IP mode
  dispatchSetting(json, "network/sta", ESP_STA_IP_MODE, "ip mode", IpModeValues,
                  IpModeLabels, sizeof(IpModeLabels) / sizeof(char*), -1, -1,
                  -1, nullptr, true, target, requestId);
  // STA static IP
  dispatchSetting(json, "network/sta", ESP_STA_IP_VALUE, "ip", nullptr, nullptr,
                  -1, -1, -1, -1, nullptr, true, target, requestId);

  // STA static Gateway
  dispatchSetting(json, "network/sta", ESP_STA_GATEWAY_VALUE, "gw", nullptr,
                  nullptr, -1, -1, -1, -1, nullptr, true, target, requestId);
  // STA static Mask
  dispatchSetting(json, "network/sta", ESP_STA_MASK_VALUE, "msk", nullptr,
                  nullptr, -1, -1, -1, -1, nullptr, true, target, requestId);
  // STA static DNS
  dispatchSetting(json, "network/sta", ESP_STA_DNS_VALUE, "DNS", nullptr,
                  nullptr, -1, -1, -1, -1, nullptr, true, target, requestId);

#endif  // WIFI_FEATURE || ETH_FEATURE

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  // Sta fallback mode
  dispatchSetting(json, "network/sta", ESP_STA_FALLBACK_MODE,
                  "sta fallback mode", FallbackValues, FallbackLabels,
                  sizeof(FallbackValues) / sizeof(char*), -1, -1, -1, nullptr,
                  true, target, requestId);
#endif  // WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
  // AP SSID network/ap
  dispatchSetting(json, "network/ap", ESP_AP_SSID, "SSID", nullptr, nullptr, 32,
                  1, 1, -1, nullptr, true, target, requestId);

  // AP password
  dispatchSetting(json, "network/ap", ESP_AP_PASSWORD, "pwd", nullptr, nullptr,
                  64, 8, 0, -1, nullptr, true, target, requestId);
  // AP static IP
  dispatchSetting(json, "network/ap", ESP_AP_IP_VALUE, "ip", nullptr, nullptr,
                  -1, -1, -1, -1, nullptr, true, target, requestId);

  // AP Channel
  dispatchSetting(json, "network/ap", ESP_AP_CHANNEL, "channel",
                  SupportedApChannelsStr, SupportedApChannelsStr,
                  sizeof(SupportedApChannelsStr) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);
#endif  // WIFI_FEATURE

#ifdef AUTHENTICATION_FEATURE
  // Admin password
  dispatchSetting(json, "security/security", ESP_ADMIN_PWD, "adm pwd", nullptr,
                  nullptr, 20, 0, -1, -1, nullptr, true, target, requestId);
  // User password
  dispatchSetting(json, "security/security", ESP_USER_PWD, "user pwd", nullptr,
                  nullptr, 20, 0, -1, -1, nullptr, true, target, requestId);

  // session timeout
  dispatchSetting(json, "security/security", ESP_SESSION_TIMEOUT,
                  "session timeout", nullptr, nullptr, 255, 0, -1, -1, nullptr,
                  true, target, requestId);

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // Secure Serial
  dispatchSetting(json, "security/security", ESP_SECURE_SERIAL, "serial",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, nullptr, true, target, requestId);

#endif  // COMMUNICATION_PROTOCOL
#endif  // AUTHENTICATION_FEATURE
#ifdef HTTP_FEATURE
  // HTTP On service/http
  dispatchSetting(json, "service/http", ESP_HTTP_ON, "enable", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);
  // HTTP port
  dispatchSetting(json, "service/http", ESP_HTTP_PORT, "port", nullptr, nullptr,
                  65535, 1, -1, -1, nullptr, true, target, requestId);
#endif  // HTTP_FEATURE

#ifdef TELNET_FEATURE
  // TELNET On service/telnet
  dispatchSetting(json, "service/telnetp", ESP_TELNET_ON, "enable", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);

  // TELNET Port
  dispatchSetting(json, "service/telnetp", ESP_TELNET_PORT, "port", nullptr,
                  nullptr, 65535, 1, -1, -1, nullptr, true, target, requestId);
#endif  // TELNET_FEATURE

#ifdef WS_DATA_FEATURE
  // Websocket On service
  dispatchSetting(json, "service/websocketp", ESP_WEBSOCKET_ON, "enable",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, nullptr, true, target, requestId);

  // Websocket Port
  dispatchSetting(json, "service/websocketp", ESP_WEBSOCKET_PORT, "port",
                  nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, target,
                  requestId);
#endif  // WS_DATA_FEATURE

#ifdef WEBDAV_FEATURE
  // WebDav On service
  dispatchSetting(json, "service/webdavp", ESP_WEBDAV_ON, "enable", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);

  // WebDav Port
  dispatchSetting(json, "service/webdavp", ESP_WEBDAV_PORT, "port", nullptr,
                  nullptr, 65535, 1, -1, -1, nullptr, true, target, requestId);
#endif  // WEBDAV_FEATURE

#ifdef FTP_FEATURE
  // FTP On service/ftp
  dispatchSetting(json, "service/ftp", ESP_FTP_ON, "enable", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);

  // FTP Ports
  // CTRL Port
  dispatchSetting(json, "service/ftp", ESP_FTP_CTRL_PORT, "control port",
                  nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, target,
                  requestId);

  // Active Port
  dispatchSetting(json, "service/ftp", ESP_FTP_DATA_ACTIVE_PORT, "active port",
                  nullptr, nullptr, 65535, 1, -1, -1, nullptr, true, target,
                  requestId);

  // Passive Port
  dispatchSetting(json, "service/ftp", ESP_FTP_DATA_PASSIVE_PORT,
                  "passive port", nullptr, nullptr, 65535, 1, -1, -1, nullptr,
                  true, target, requestId);
#endif  // FTP_FEATURE

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  // Serial bridge On service
  dispatchSetting(json, "service/serial_bridge", ESP_SERIAL_BRIDGE_ON, "enable",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, nullptr, true, target, requestId);

  // Baud Rate
  dispatchSetting(json, "service/serial_bridge", ESP_SERIAL_BRIDGE_BAUD, "baud",
                  SupportedBaudListSizeStr, SupportedBaudListSizeStr,
                  sizeof(SupportedBaudListSizeStr) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#ifdef TIMESTAMP_FEATURE

  // Internet Time
  dispatchSetting(json, "service/time", ESP_INTERNET_TIME, "i-time",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, nullptr, true, target, requestId);

  // Time zone
  dispatchSetting(json, "service/time", ESP_TIME_ZONE, "tzone",
                  SupportedTimeZones, SupportedTimeZones,
                  SupportedTimeZonesSize, -1, -1, -1, nullptr, true, target,
                  requestId);

  // Time Server1
  dispatchSetting(json, "service/time", ESP_TIME_SERVER1, "t-server", nullptr,
                  nullptr, 127, 0, -1, -1, nullptr, true, target, requestId);

  // Time Server2
  dispatchSetting(json, "service/time", ESP_TIME_SERVER2, "t-server", nullptr,
                  nullptr, 127, 0, -1, -1, nullptr, true, target, requestId);

  // Time Server3
  dispatchSetting(json, "service/time", ESP_TIME_SERVER3, "t-server", nullptr,
                  nullptr, 127, 0, -1, -1, nullptr, true, target, requestId);
#endif  // TIMESTAMP_FEATURE

#ifdef NOTIFICATION_FEATURE
  // Auto notification
  dispatchSetting(json, "service/notification", ESP_AUTO_NOTIFICATION,
                  "auto notif", YesNoValues, YesNoLabels,
                  sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                  true, target, requestId);

  // Notification type
  dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TYPE,
                  "notification", NotificationsValues, NotificationsLabels,
                  sizeof(NotificationsValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);

  // Token 1
  dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TOKEN1, "t1",
                  nullptr, nullptr, 250, 0, -1, -1, nullptr, true, target,
                  requestId);

  // Token 2
  dispatchSetting(json, "service/notification", ESP_NOTIFICATION_TOKEN2, "t2",
                  nullptr, nullptr, 63, 0, -1, -1, nullptr, true, target,
                  requestId);

  // Notifications Settings
  dispatchSetting(json, "service/notification", ESP_NOTIFICATION_SETTINGS, "ts",
                  nullptr, nullptr, 128, 0, -1, -1, nullptr, true, target,
                  requestId);
#endif  // NOTIFICATION_FEATURE

#ifdef BUZZER_DEVICE
  // Buzzer state
  dispatchSetting(json, "device/device", ESP_BUZZER, "buzzer", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);
#endif  // BUZZER_DEVICE

#ifdef SENSOR_DEVICE
  // Sensor type
  dispatchSetting(json, "device/sensor", ESP_SENSOR_TYPE, "type", SensorValues,
                  SensorLabels, sizeof(SensorValues) / sizeof(char*), -1, -1,
                  -1, nullptr, true, target, requestId);

  // Sensor interval
  dispatchSetting(json, "device/sensor", ESP_SENSOR_INTERVAL, "intervalms",
                  nullptr, nullptr, 60000, 0, -1, -1, nullptr, true, target,
                  requestId);

#endif  // SENSOR_DEVICE
#if defined(SD_DEVICE)
#if SD_DEVICE != ESP_SDIO
  // SPI SD Divider
  dispatchSetting(json, "device/sd", ESP_SD_SPEED_DIV, "speedx",
                  SupportedSPIDividerStr, SupportedSPIDividerStr,
                  SupportedSPIDividerStrSize, -1, -1, -1, nullptr, true, target,
                  requestId);
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
  // SD CHECK UPDATE AT BOOT feature
  dispatchSetting(json, "device/sd", ESP_SD_CHECK_UPDATE_AT_BOOT, "SD updater",
                  YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*),
                  -1, -1, -1, nullptr, true, target, requestId);
#endif  // SD_UPDATE_FEATURE
#endif  // SD_DEVICE

#if !defined(FIXED_FW_TARGET)
  // Target FW
  dispatchSetting(json, "system/system", ESP_TARGET_FW, "targetfw",
                  FirmwareValues, FirmwareLabels,
                  sizeof(FirmwareValues) / sizeof(char*), -1, -1, -1, nullptr,
                  true, target, requestId);
#endif  // FIXED_FW_TARGET
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // Baud Rate
  dispatchSetting(json, "system/system", ESP_BAUD_RATE, "baud",
                  SupportedBaudListSizeStr, SupportedBaudListSizeStr,
                  sizeof(SupportedBaudListSizeStr) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL

  // Start delay
  dispatchSetting(json, "system/boot", ESP_BOOT_DELAY, "bootdelay", nullptr,
                  nullptr, 40000, 0, -1, -1, nullptr, true, target, requestId);

  // Verbose boot
  dispatchSetting(json, "system/boot", ESP_VERBOSE_BOOT, "verbose", YesNoValues,
                  YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1, -1,
                  nullptr, true, target, requestId);

  if (json) {
    if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending response to clients");
    }
  } else {
    if (!dispatch("ok\n", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending response to clients");
    }
  }
}
