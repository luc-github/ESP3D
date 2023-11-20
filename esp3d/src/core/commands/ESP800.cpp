/*
 ESP800.cpp - ESP3D command class

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
#include "../../modules/network/netconfig.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#ifdef FILESYSTEM_FEATURE
#include "../../modules/filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
#include "../../modules/network/netconfig.h"
#if defined(WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#endif  // WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef HTTP_FEATURE
#include "../../modules/http/http_server.h"
#include "../../modules/websocket/websocket_server.h"
#endif  // HTTP_FEATURE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE
#ifdef CAMERA_DEVICE
#include "../../modules/camera/camera.h"
#endif  // CAMERA_DEVICE
#define COMMAND_ID 800
// get fw capabilities
// eventually set time with pc time
// output is JSON or plain text according parameter
//[ESP800]json=<no><time=YYYY-MM-DDTHH:mm:ss> <tz=+HH:ss> <version=3.0.0-a11>
//<setup=0/1>
void ESP3DCommands::ESP800(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  String timestr = "none";
#if defined(TIMESTAMP_FEATURE)
  String timeparam = get_param(msg, cmd_params_pos, "time=");
  String tzparam = get_param(msg, cmd_params_pos, "tz=");
#endif  // TIMESTAMP_FEATURE
  String setupparam = get_param(msg, cmd_params_pos, "setup=");
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
#if defined(TIMESTAMP_FEATURE)
  // set time if internet time is not enabled
  if (!timeService.isInternetTime()) {
    if (tzparam.length() > 0) {
      if (!timeService.setTimeZone(tzparam.c_str())) {
        // not blocking error
        esp3d_log_e("Error setting timezone");
        timestr = "Failed to set timezone";
      } else {
        timestr = "Manual";
      }
    } else {
      timestr = "Not set";
    }
    if (timeparam.length() > 0) {
      if (!timeService.setTime(timeparam.c_str())) {
        // not blocking error
        esp3d_log_e("Error setting time");
        timestr = "Failed to set time";
      }
    }
  } else {
    if (timeService.started()) {
      timestr = "Auto";
    } else {
      timestr = "Failed to set";
    }
  }
#else
  timestr = "none";
#endif  // ESP3D_TIMESTAMP_FEATURE
  if (setupparam.length() > 0) {
    if (!ESP3DSettings::writeByte(ESP_SETUP, setupparam == "1" ? 1 : 0)) {
      // not blocking error
      esp3d_log_e("Error writing setup state");
    }
  }
  if (json) {
    tmpstr = "{\"cmd\":\"800\",\"status\":\"ok\",\"data\":{";

  } else {
    tmpstr = "Capabilities:\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  // list capabilities
  tmpstr = "";
#if defined(SHORT_BUILD_VERSION)
  tmpstr += SHORT_BUILD_VERSION;
  tmpstr += "-";
#endif  // SHORT_BUILD_VERSION
  tmpstr += FW_VERSION;
  // FW version
  if (!dispatchKeyValue(json, "FWVersion", tmpstr.c_str(), target, requestId,
                        false, true)) {
    return;
  }
  // FW Target
  tmpstr = ESP3DSettings::GetFirmwareTargetShortName();
  if (!dispatchKeyValue(json, "FWTarget", tmpstr.c_str(), target, requestId)) {
    return;
  }

  // FW ID
  tmpstr = ESP3DSettings::GetFirmwareTarget();
  if (!dispatchKeyValue(json, "FWTargetID", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }

  // Setup done
  tmpstr = ESP3DSettings::readByte(ESP_SETUP) == 0 ? "Enabled" : "Disabled";
  if (!dispatchKeyValue(json, "Setup", tmpstr.c_str(), target, requestId)) {
    return;
  }

  // SD connection
  if (ESP3DSettings::GetSDDevice() == ESP_DIRECT_SD) {
    tmpstr = "direct";
  } else if (ESP3DSettings::GetSDDevice() == ESP_SHARED_SD) {
    tmpstr = "shared";
  } else {
    tmpstr = "none";
  }
  if (!dispatchKeyValue(json, "SDConnection", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }

  // Serial protocol
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  tmpstr = "MKS";
#endif  // COMMUNICATION_PROTOCOL ==  MKS_SERIAL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL
  tmpstr = "Raw";
#endif  // COMMUNICATION_PROTOCOL ==  RAW_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
  tmpstr = "Socket";
#endif  // COMMUNICATION_PROTOCOL ==  SOCKET_SERIAL
  if (!dispatchKeyValue(json, "SerialProtocol", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }

  // Authentication
#if defined(AUTHENTICATION_FEATURE)
  tmpstr = "Enabled";
#else
  tmpstr = "Disabled";
#endif  // AUTHENTICATION_FEATURE
  if (!dispatchKeyValue(json, "Authentication", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }

  // Web Communication
#if (defined(WIFI_FEATURE) || defined(ETH_FEATURE)) && defined(HTTP_FEATURE)
#if defined(ASYNCWEBSERVER_FEATURE)
  tmpstr = "Asynchronous";
#else
  tmpstr = "Synchronous";
#endif  // ASYNCWEBSERVER_FEATURE
  if (!dispatchKeyValue(json, "WebCommunication", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
  // WebSocket IP
  tmpstr = NetConfig::localIP().c_str();
  if (!dispatchKeyValue(json, "WebSocketIP", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
  // WebSocket Port
#if defined(ASYNCWEBSERVER_FEATURE)
  tmpstr = HTTP_Server::port();
#else
  tmpstr = websocket_terminal_server.getPort();
#endif  // ASYNCWEBSERVER_FEATURE
  if (!dispatchKeyValue(json, "WebSocketPort", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
#endif  // (WIFI_FEATURE) || ETH_FEATURE) && HTTP_FEATURE)
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  // Hostname
  tmpstr = NetConfig::hostname();
  if (!dispatchKeyValue(json, "Hostname", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
  if (WiFiConfig::started()) {
    // WiFi mode
    tmpstr = (WiFi.getMode() == WIFI_AP) ? "AP" : "STA";
    if (!dispatchKeyValue(json, "WiFiMode", tmpstr.c_str(), target,
                          requestId)) {
      return;
    }
  }
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
// Update
#if defined(WEB_UPDATE_FEATURE)
  if (ESP_FileSystem::max_update_size() != 0) {
    tmpstr = "Enabled";
  } else {
    tmpstr = "Disabled";
  }
#else
  tmpstr = "Disabled";
#endif  // WEB_UPDATE_FEATURE
  if (!dispatchKeyValue(json, "WebUpdate", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // WIFI_FEATURE|| ETH_FEATURE
        // FS
#if defined(FILESYSTEM_FEATURE)
  tmpstr = ESP_FileSystem::FilesystemName();
#else
  tmpstr = "none";
#endif  // FILESYSTEM_FEATURE
  if (!dispatchKeyValue(json, "FlashFileSystem", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
  // Host path
  tmpstr = ESP3D_HOST_PATH;
  if (!dispatchKeyValue(json, "HostPath", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // time server
#if defined(TIMESTAMP_FEATURE)
  tmpstr = timestr;
#else
  tmpstr = "none";
#endif  // TIMESTAMP_FEATURE
  if (!dispatchKeyValue(json, "Time", tmpstr.c_str(), target, requestId)) {
    return;
  }

#ifdef CAMERA_DEVICE
  // camera ID
  tmpstr = esp3d_camera.GetModel();
  if (!dispatchKeyValue(json, "CameraID", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // camera Name
  tmpstr = esp3d_camera.GetModelString();
  if (!dispatchKeyValue(json, "CameraName", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
#endif  // CAMERA_DEVICE

  // end of list
  if (json) {
    if (!dispatch("}}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending answer to clients");
    }
  } else {
    {
      if (!dispatch("ok\n", target, requestId, ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
  }
}
