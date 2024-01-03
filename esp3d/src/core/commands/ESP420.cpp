/*
 ESP420.cpp - ESP3D command class

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
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "../esp3d_string.h"

#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../../modules/serial/serial_service.h"
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#ifdef FILESYSTEM_FEATURE
#include "../../modules/filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
#include "../../modules/network/netconfig.h"
#if defined(WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
#include "../../modules/ethernet/ethconfig.h"
#endif  // ETH_FEATURE
#if defined(BLUETOOTH_FEATURE)
#include "../../modules/bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE
#endif  // WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef HTTP_FEATURE
#include "../../modules/http/http_server.h"
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
#include "../../modules/telnet/telnet_server.h"
#endif  // TELNET_FEATURE
#ifdef FTP_FEATURE
#include "../../modules/ftp/FtpServer.h"
#endif  // FTP_FEATURE
#ifdef WS_DATA_FEATURE
#include "../../modules/websocket/websocket_server.h"
#endif  // WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
#include "../../modules/webdav/webdav_server.h"
#endif  // WEBDAV_FEATURE
#if defined(TIMESTAMP_FEATURE)
#include "../../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE
#if defined(SENSOR_DEVICE)
#include "../../modules/sensor/sensor.h"
#endif  // SENSOR_DEVICE
#ifdef NOTIFICATION_FEATURE
#include "../../modules/notifications/notifications_service.h"
#endif  // NOTIFICATION_FEATURE
#ifdef BUZZER_DEVICE
#include "../../modules/buzzer/buzzer.h"
#endif  // BUZZER_DEVICE
#ifdef CAMERA_DEVICE
#include "../../modules/camera/camera.h"
#endif  // CAMERA_DEVICE
#ifdef SD_DEVICE
#include "../../modules/filesystem/esp_sd.h"
#endif  // SD_DEVICE
#if defined(DISPLAY_DEVICE)
#include "../../modules/display/display.h"
#endif  // DISPLAY_DEVICE
#define COMMAND_ID 420
#if defined(AUTHENTICATION_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#endif  // AUTHENTICATION_FEATURE

// Get ESP current status
// output is JSON or plain text according parameter
//[ESP420]json=<no>
void ESP3DCommands::ESP420(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool addPreTag = hasTag(msg, cmd_params_pos, "addPreTag");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (json) {
    tmpstr = "{\"cmd\":\"420\",\"status\":\"ok\",\"data\":[";

  } else {
    if (addPreTag) {
      tmpstr = "<pre>\n";
    } else {
      tmpstr = "Configuration:\n";
    }
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  // First entry

  // Chip ID
  tmpstr = ESP3DHal::getChipID();
  if (!dispatchIdValue(json, "chip id", tmpstr.c_str(), target, requestId,
                       true)) {
    return;
  }

  // CPU Freq
  tmpstr = String(ESP.getCpuFreqMHz());
  tmpstr += "Mhz";
  if (!dispatchIdValue(json, "CPU Freq", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

  // CPU Temp
  if (ESP3DHal::has_temperature_sensor()) {
    tmpstr = String(ESP3DHal::temperature(), 1);
    tmpstr += "C";
    if (!dispatchIdValue(json, "CPU Temp", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }

  // Free Memory
  tmpstr = esp3d_string::formatBytes(ESP.getFreeHeap());

#ifdef ARDUINO_ARCH_ESP32
#ifdef BOARD_HAS_PSRAM
  tmpstr += " - PSRAM:";
  tmpstr += esp3d_string::formatBytes(ESP.getFreePsram());
#endif  // BOARD_HAS_PSRAM
#endif  // ARDUINO_ARCH_ESP32
  if (!dispatchIdValue(json, "free mem", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

  // SDK Version
  tmpstr = ESP.getSdkVersion();
  if (!dispatchIdValue(json, "SDK", tmpstr.c_str(), target, requestId, false)) {
    return;
  }

  // Flash size
  tmpstr = esp3d_string::formatBytes(ESP.getFlashChipSize());
  if (!dispatchIdValue(json, "flash size", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

#if (defined(WIFI_FEATURE) || defined(ETH_FEATURE)) && \
    (defined(OTA_FEATURE) || defined(WEB_UPDATE_FEATURE))
  // update space
  tmpstr = esp3d_string::formatBytes(ESP_FileSystem::max_update_size());
  if (!dispatchIdValue(json, "size for update", tmpstr.c_str(), target,
                       requestId, false)) {
    return;
  }
#endif  // WIFI_FEATURE || ETH_FEATURE

#if defined(FILESYSTEM_FEATURE)
  // FileSystem type
  tmpstr = ESP_FileSystem::FilesystemName();
  if (!dispatchIdValue(json, "FS type", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
  // FileSystem capacity
  tmpstr = esp3d_string::formatBytes(ESP_FileSystem::usedBytes());
  tmpstr += "/";
  tmpstr += esp3d_string::formatBytes(ESP_FileSystem::totalBytes());
  if (!dispatchIdValue(json, "FS usage", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // FILESYSTEM_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // baud rate
  tmpstr = String(esp3d_serial_service.baudRate());
  if (!dispatchIdValue(json, "baud", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() != WIFI_OFF) {
    // sleep mode
    tmpstr = WiFiConfig::getSleepModeString();
    if (!dispatchIdValue(json, "sleep mode", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE)
  // Wifi enabled
  tmpstr = (WiFi.getMode() == WIFI_OFF) ? "OFF" : "ON";
  if (!dispatchIdValue(json, "wifi", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  // Ethernet enabled
  tmpstr = (EthConfig::started()) ? "ON" : "OFF";
  if (!dispatchIdValue(json, "ethernet", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // ETH_FEATURE

#if defined(BLUETOOTH_FEATURE)
  // BT enabled
  tmpstr = (bt_service.started()) ? "ON" : "OFF";
  if (!dispatchIdValue(json, "bt", tmpstr.c_str(), target, requestId, false)) {
    return;
  }
#endif  // BLUETOOTH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  // Hostname
  tmpstr = NetConfig::hostname();
  if (!dispatchIdValue(json, "hostname", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

#if defined(HTTP_FEATURE)
  if (HTTP_Server::started()) {
    // http port
    tmpstr = String(HTTP_Server::port());
    if (!dispatchIdValue(json, "HTTP port", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // HTTP_FEATURE
#if defined(TELNET_FEATURE)
  if (telnet_server.started()) {
    // telnet port
    tmpstr = String(telnet_server.port());
    if (!dispatchIdValue(json, "Telnet port", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }

  if (telnet_server.isConnected()) {
    // telnet client IP
    tmpstr = telnet_server.clientIPAddress();
    if (!dispatchIdValue(json, "Telnet Client", tmpstr.c_str(), target,
                         requestId, false)) {
      return;
    }
  }

#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
  if (webdav_server.started()) {
    // WebDav port
    tmpstr = String(webdav_server.port());
    if (!dispatchIdValue(json, "WebDav port", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // WEBDAV_FEATURE
#if defined(FTP_FEATURE)
  if (ftp_server.started()) {
    // ftp ports
    tmpstr = String(ftp_server.ctrlport());
    tmpstr += ",";
    tmpstr += String(ftp_server.dataactiveport());
    tmpstr += ",";
    tmpstr += String(ftp_server.datapassiveport());
    if (!dispatchIdValue(json, "Ftp ports", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // FTP_FEATURE
#if defined(WS_DATA_FEATURE)
  if (websocket_data_server.started()) {
    // websocket port
    tmpstr = String(websocket_data_server.port());
    if (!dispatchIdValue(json, "Websocket port", tmpstr.c_str(), target,
                         requestId, false)) {
      return;
    }
  }
#endif  // WS_DATA_FEATURE
#if defined(CAMERA_DEVICE)
  if (esp3d_camera.started()) {
    // camera name
    tmpstr = esp3d_camera.GetModelString();
    tmpstr += "(";
    tmpstr += esp3d_camera.GetModel();
    tmpstr += ")";
    if (!dispatchIdValue(json, "camera name", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // CAMERA_DEVICE
#endif  // #if defined(WIFI_FEATURE) || defined(ETH_FEATURE)

#if defined(DISPLAY_DEVICE)
  tmpstr = esp3d_display.getModelString();
  if (!dispatchIdValue(json, "display", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // DISPLAY_DEVICE

#if defined(BLUETOOTH_FEATURE)
  if (bt_service.started()) {
    // BT mode
    tmpstr = BTService::macAddress();
    if (!dispatchIdValue(json, "bt", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // BT status
    tmpstr = (bt_service.isConnected()) ? "connected" : "disconnected";
    if (bt_service.isConnected()) {
      tmpstr += " (client: ";
      tmpstr += BTService::clientmacAddress();
      tmpstr += ")";
    }
    if (!dispatchIdValue(json, "BT Status", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // BLUETOOTH_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    // Ethernet mode
    tmpstr = ETH.macAddress();
    if (!dispatchIdValue(json, "ethernet", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // Ethernet cable
    tmpstr = (EthConfig::linkUp()) ? "connected" : "disconnected";

    if (EthConfig::linkUp()) {
      tmpstr += " (";
      tmpstr += ETH.linkSpeed();
      tmpstr += "Mbps)";
    }
    if (!dispatchIdValue(json, "cable", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // IP mode
    tmpstr = (NetConfig::isIPModeDHCP(ESP_ETH_STA)) ? "dhcp" : "static";
    if (!dispatchIdValue(json, "ip mode", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // IP value
    tmpstr = ETH.localIP().toString();
    if (!dispatchIdValue(json, "ip", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // GW value
    tmpstr = ETH.gatewayIP().toString();
    if (!dispatchIdValue(json, "gw", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // Mask value
    tmpstr = ETH.subnetMask().toString();
    if (!dispatchIdValue(json, "msk", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // DNS value
    tmpstr = ETH.dnsIP().toString();
    if (!dispatchIdValue(json, "DNS", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
  }
#endif  // ETH_FEATURE
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() != WIFI_OFF) {
    // WiFi Mode
    tmpstr = (WiFi.getMode() == WIFI_STA)      ? "sta"
             : (WiFi.getMode() == WIFI_AP)     ? "ap"
             : (WiFi.getMode() == WIFI_AP_STA) ? "mixed"
                                               : "unknown";
    if (!dispatchIdValue(json, "wifi mode", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }
    // WiFi mac
    tmpstr = (WiFi.getMode() == WIFI_STA)  ? WiFi.macAddress()
             : (WiFi.getMode() == WIFI_AP) ? WiFi.softAPmacAddress()
             : (WiFi.getMode() == WIFI_AP_STA)
                 ? WiFi.macAddress() + "/" + WiFi.softAPmacAddress()
                 : "unknown";
    if (!dispatchIdValue(json, "mac", tmpstr.c_str(), target, requestId,
                         false)) {
      return;
    }

    // WiFi Station
    if (WiFi.getMode() == WIFI_STA) {
      // Connected to SSID
      tmpstr = (WiFi.isConnected()) ? WiFi.SSID() : "";
      if (!dispatchIdValue(json, "SSID", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      if (WiFi.isConnected()) {  // in case query come from serial
        // Signal strength
        tmpstr = WiFiConfig::getSignal(WiFi.RSSI(), false);
        tmpstr += "%";
        if (!dispatchIdValue(json, "signal", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // Phy Mode
        tmpstr = WiFiConfig::getPHYModeString(WIFI_STA);
        if (!dispatchIdValue(json, "phy mode", tmpstr.c_str(), target,
                             requestId, false)) {
          return;
        }
        // Channel
        tmpstr = String(WiFi.channel());
        if (!dispatchIdValue(json, "channel", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // IP Mode
        tmpstr = (NetConfig::isIPModeDHCP(ESP_WIFI_STA)) ? "dhcp" : "static";
        if (!dispatchIdValue(json, "ip mode", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // IP value
        tmpstr = WiFi.localIP().toString();
        if (!dispatchIdValue(json, "ip", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // Gateway value
        tmpstr = WiFi.gatewayIP().toString();
        if (!dispatchIdValue(json, "gw", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // Mask value
        tmpstr = WiFi.subnetMask().toString();
        if (!dispatchIdValue(json, "msk", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
        // DNS value
        tmpstr = WiFi.dnsIP().toString();
        if (!dispatchIdValue(json, "DNS", tmpstr.c_str(), target, requestId,
                             false)) {
          return;
        }
      }
      // Disabled Mode
      tmpstr = "OFF";
      if (!dispatchIdValue(json, "ap", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // Disabled Mode
      tmpstr = WiFi.softAPmacAddress();
      if (!dispatchIdValue(json, "mac", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
    } else if (WiFi.getMode() == WIFI_AP) {
      // AP SSID
      tmpstr = WiFiConfig::AP_SSID();
      if (!dispatchIdValue(json, "SSID", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // AP Visibility
      tmpstr = (WiFiConfig::is_AP_visible()) ? "yes" : "no";
      if (!dispatchIdValue(json, "visible", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // AP Authentication
      tmpstr = WiFiConfig::AP_Auth_String();
      if (!dispatchIdValue(json, "authentication", tmpstr.c_str(), target,
                           requestId, false)) {
        return;
      }
      // DHCP Server
      tmpstr = (NetConfig::isDHCPServer(ESP_WIFI_AP)) ? "ON" : "OFF";
      if (!dispatchIdValue(json, "DHCP Server", tmpstr.c_str(), target,
                           requestId, false)) {
        return;
      }
      // IP Value
      tmpstr = WiFi.softAPIP().toString();
      if (!dispatchIdValue(json, "ip", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // Gateway Value
      tmpstr = WiFiConfig::AP_Gateway_String();
      if (!dispatchIdValue(json, "gw", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // Mask Value
      tmpstr = WiFiConfig::AP_Mask_String();
      if (!dispatchIdValue(json, "msk", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // Connected clients
      const char* entry = NULL;
      uint8_t nb = 0;
      entry = WiFiConfig::getConnectedSTA(&nb, true);

      tmpstr = String(nb);
      if (!dispatchIdValue(json, "clients", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      for (uint8_t i = 0; i < nb; i++) {
        // Client
        tmpstr = entry;
        String label = "# " + String(i);
        if (!dispatchIdValue(json, label.c_str(), tmpstr.c_str(), target,
                             requestId, false)) {
          return;
        }
        entry = WiFiConfig::getConnectedSTA();
      }

      // Disabled Mode
      tmpstr = "OFF";
      if (!dispatchIdValue(json, "sta", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
      // Disabled Mode
      tmpstr = WiFi.macAddress();
      if (!dispatchIdValue(json, "mac", tmpstr.c_str(), target, requestId,
                           false)) {
        return;
      }
    }
  }
#endif  // WIFI_FEATURE

#if defined(TIMESTAMP_FEATURE)
  // NTP enabled
  tmpstr = (timeService.started()) ? "ON" : "OFF";
  if (!dispatchIdValue(json, "ntp", tmpstr.c_str(), target, requestId, false)) {
    return;
  }
#endif  // TIMESTAMP_FEATURE

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
  // serial enabled
  tmpstr = (esp3d_serial_service.started()) ? "ON" : "OFF";
  if (!dispatchIdValue(json, "serial", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if defined(AUTHENTICATION_FEATURE)
  // authentication enabled
  tmpstr = "ON";
#else
  tmpstr = "OFF";
#endif  // AUTHENTICATION_FEATURE
  if (!dispatchIdValue(json, "authentication", tmpstr.c_str(), target,
                       requestId, false)) {
    return;
  }

#if defined(NOTIFICATION_FEATURE)
  // notification enabled
  tmpstr = (notificationsservice.started()) ? "ON" : "OFF";
  if (notificationsservice.started()) {
    tmpstr += " (";
    tmpstr += notificationsservice.getTypeString();
    tmpstr += ")";
  }
  if (!dispatchIdValue(json, "notification", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // NOTIFICATION_FEATURE
#if defined(SD_DEVICE)
  // SD enabled
  tmpstr = (ESP3DSettings::GetSDDevice() == ESP_DIRECT_SD)   ? "direct "
           : (ESP3DSettings::GetSDDevice() == ESP_SHARED_SD) ? "shared "
                                                             : "none ";
  tmpstr += "(";
  tmpstr += ESP_SD::FilesystemName();
  tmpstr += ")";
  if (!dispatchIdValue(json, "sd", tmpstr.c_str(), target, requestId, false)) {
    return;
  }

#ifdef SD_UPDATE_FEATURE
  // SD updater enabled
  tmpstr =
      ESP3DSettings::readByte(ESP_SD_CHECK_UPDATE_AT_BOOT) != 0 ? "ON" : "OFF";
  if (!dispatchIdValue(json, "SD updater", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // SD_UPDATE_FEATURE

#endif  // SD_DEVICE
#if defined(SENSOR_DEVICE)
  // sensor enabled
  tmpstr = (esp3d_sensor.started()) ? "ON" : "OFF";
  if (esp3d_sensor.started()) {
    tmpstr += " (";
    tmpstr += esp3d_sensor.GetCurrentModelString();
    tmpstr += ")";
  }
  if (!dispatchIdValue(json, "sensor", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // SENSOR_DEVICE
#if defined(BUZZER_DEVICE)
  // buzzer enabled
  tmpstr = (esp3d_buzzer.started()) ? "ON" : "OFF";
  if (!dispatchIdValue(json, "buzzer", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // BUZZER_DEVICE
#if defined(CAMERA_DEVICE)
  // camera enabled
  tmpstr = (esp3d_camera.started()) ? "ON" : "OFF";
  if (esp3d_camera.started()) {
    tmpstr += " (";

    tmpstr += esp3d_camera.GetModelString();
    tmpstr += ")";
  }
  if (!dispatchIdValue(json, "camera", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // CAMERA_DEVICE

// Serial mode
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  tmpstr = "MKS";
  if (!dispatchIdValue(json, "serial", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

  // Target Firmware
  tmpstr = ESP3DSettings::GetFirmwareTargetShortName();
  if (!dispatchIdValue(json, "targetfw", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

  // FW version

  tmpstr = "";
#if defined(SHORT_BUILD_VERSION)
  tmpstr += SHORT_BUILD_VERSION "-";
#endif  // SHORT_BUILD_VERSION
  tmpstr += FW_VERSION;

  if (!dispatchIdValue(json, "FW ver", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

  // FW architecture
  tmpstr = ESP3DSettings::TargetBoard();
  if (!dispatchIdValue(json, "FW arch", tmpstr.c_str(), target, requestId,
                       false)) {
    return;
  }

  // end of list
  if (json) {
    if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending answer to clients");
    }
  } else {
    {
      if (addPreTag) {
        tmpstr = "</pre>\n";
      } else {
        tmpstr = "ok\n";
      }
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
  }
}
