/*
  commands.cpp - ESP3D commands class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "esp3d_commands.h"

#include "../include/esp3d_config.h"
#include "esp3d.h"
#include "esp3d_settings.h"

#if defined(ESP_LOG_FEATURE)
const char *esp3dclientstr[] = {
    "no_client",     "serial",        "usb_serial",      "stream",
    "telnet",        "http",          "webui_websocket", "websocket",
    "rendering",     "bluetooth",     "socket_serial",   "echo_serial",
    "serial_bridge", "remote_screen", "mks_serial",      "command",
    "system",        "all_clients"};
#define GETCLIENTSTR(id)                                         \
  static_cast<uint8_t>(id) >= 0 &&                               \
          static_cast<uint8_t>(id) <=                            \
              static_cast<uint8_t>(ESP3DClientType::all_clients) \
      ? esp3dclientstr[static_cast<uint8_t>(id)]                 \
      : "Out of index"

const char *esp3dmsgstr[] = {"head", "core", "tail", "unique"};
#define GETMSGTYPESTR(id)                                    \
  static_cast<uint8_t>(id) >= 0 &&                           \
          static_cast<uint8_t>(id) <=                        \
              static_cast<uint8_t>(ESP3DMessageType::unique) \
      ? esp3dmsgstr[static_cast<uint8_t>(id)]                \
      : "Out of index"

#endif  // defined(ESP_LOG_FEATURE)

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../modules/mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

#include "../modules/serial/serial_service.h"

#if defined(TELNET_FEATURE)
#include "../modules/telnet/telnet_server.h"
#endif  // TELNET_FEATURE

#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../modules/websocket/websocket_server.h"
#endif  // HTTP_FEATURE || WS_DATA_FEATURE

#if defined(HTTP_FEATURE)
#include "../modules/http/http_server.h"
#endif  // HTTP_FEATURE

#if defined(ESP_LUA_INTERPRETER_FEATURE)
#include "../modules/lua_interpreter/lua_interpreter_service.h"
#endif  // ESP_LUA_INTERPRETER_FEATURE

#ifdef BLUETOOTH_FEATURE
#include "../modules/bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE

#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include "../modules/serial2socket/serial2socket.h"
#endif  // defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL

#if defined(DISPLAY_DEVICE)
#include "../modules/display/display.h"
#endif  // DISPLAY_DEVICE

#if defined(GCODE_HOST_FEATURE)
#include "../modules/gcode_host/gcode_host.h"
#endif  // GCODE_HOST_FEATURE

#if defined(USB_SERIAL_FEATURE)
#include "../modules/usb-serial/usb_serial_service.h" 
#endif  // USB_SERIAL_FEATURE

ESP3DCommands esp3d_commands;

ESP3DCommands::ESP3DCommands() {
  //Need to sync with setting part
#if COMMUNICATION_PROTOCOL == RAW_SERIAL
  _output_client = ESP3DClientType::serial;
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  _output_client = ESP3DClientType::mks_serial;
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
  _output_client = ESP3DClientType::socket_serial;
#endif  //
}

ESP3DCommands::~ESP3DCommands() {}

// check if current line is an [ESPXXX] command
bool ESP3DCommands::is_esp_command(uint8_t *sbuf, size_t len) {
  // TODO
  // M117 should be handled here and transfered to [ESP214] if it is an host
  if (len < 5) {
    return false;
  }
  if ((char(sbuf[0]) == '[') && (char(sbuf[1]) == 'E') &&
      (char(sbuf[2]) == 'S') && (char(sbuf[3]) == 'P') &&
      ((char(sbuf[4]) == ']') || (char(sbuf[5]) == ']') ||
       (char(sbuf[6]) == ']') || (char(sbuf[7]) == ']'))) {
    return true;
  }
  // echo command header on some targeted firmware
  if (len >= 14) {
    if ((char(sbuf[0]) == 'e') && (char(sbuf[1]) == 'c') &&
        (char(sbuf[2]) == 'h') && (char(sbuf[3]) == 'o') &&
        (char(sbuf[4]) == ':') && (char(sbuf[5]) == ' ') &&
        (char(sbuf[6]) == '[') && (char(sbuf[7]) == 'E') &&
        (char(sbuf[8]) == 'S') && (char(sbuf[9]) == 'P') &&
        ((char(sbuf[4]) == ']') || (char(sbuf[5]) == ']') ||
         (char(sbuf[6]) == ']') || (char(sbuf[7]) == ']'))) {
      return true;
    }
  }
  return false;
}

const char *ESP3DCommands::format_response(uint cmdID, bool isjson, bool isok,
                                           const char *message) {
  static String res;
  res = "";
  if (!isjson) {
    res += message;
  } else {
    res = "{\"cmd\":\"";
    res += String(cmdID);
    res += "\",\"status\":\"";
    if (isok) {
      res += "ok";
    } else {
      res += "error";
    }
    res += "\",\"data\":";
    if (message[0] != '{') {
      res += "\"";
    }
    res += message;
    if (message[0] != '{') {
      res += "\"";
    }
    res += "}";
  }
  return res.c_str();
}

bool ESP3DCommands::hasTag(ESP3DMessage *msg, uint start, const char *label) {
  if (!msg) {
    esp3d_log_e("no msg for tag %s", label);
    return false;
  }
  String lbl = label;
  // esp3d_log("checking message for tag %s", label);
  uint lenLabel = strlen(label);
  lbl += "=";
  lbl = get_param(msg, start, lbl.c_str());
  if (lbl.length() != 0) {
    // esp3d_log("Label is used with parameter %s", lbl.c_str());
    // make result uppercase
    lbl.toUpperCase();
    return (lbl == "YES" || lbl == "1" || lbl == "TRUE");
  }
  bool prevCharIsEscaped = false;
  bool prevCharIsspace = true;
  // esp3d_log("Checking  label as tag");
  for (uint i = start; i < msg->size; i++) {
    char c = char(msg->data[i]);
    // esp3d_log("%c", c);
    if (c == label[0] && prevCharIsspace) {
      uint p = 0;
      while (i < msg->size && p < lenLabel && c == label[p]) {
        i++;
        p++;
        if (i < msg->size) {
          c = char(msg->data[i]);
          // esp3d_log("%c vs %c", c, char(msg->data[i]));
        }
      }
      if (p == lenLabel) {
        // end of params
        if (i == msg->size || std::isspace(c)) {
          // esp3d_log("label %s found", label);
          return true;
        }
      }
      if (std::isspace(c) && !prevCharIsEscaped) {
        prevCharIsspace = true;
      }
      if (c == '\\') {
        prevCharIsEscaped = true;
      } else {
        prevCharIsEscaped = false;
      }
    }
  }
  // esp3d_log("label %s not found", label);
  return false;
}

const char *ESP3DCommands::get_param(ESP3DMessage *msg, uint start,
                                     const char *label, bool *found) {
  if (!msg) {
    esp3d_log_e("no message");
    return "";
  }
  return get_param((const char *)msg->data, msg->size, start, label, found);
}

const char *ESP3DCommands::get_param(const char *data, uint size, uint start,
                                     const char *label, bool *found) {
  esp3d_log("get_param %s", label);
  int startPos = -1;
  uint lenLabel = strlen(label);
  static String value;
  bool prevCharIsEscaped = false;
  bool prevCharIsspace = true;
  value = "";
  uint startp = start;
  if (found) {
    *found = false;
  }
  while (char(data[startp]) == ' ' && startp < size) {
    startp++;
  }
  for (uint i = startp; i < size; i++) {
    char c = char(data[i]);
    if (c == label[0] && startPos == -1 && prevCharIsspace) {
      uint p = 0;
      while (i < size && p < lenLabel && c == label[p]) {
        i++;
        p++;
        if (i < size) {
          c = char(data[i]);
        }
      }
      if (p == lenLabel) {
        startPos = i;
        if (found) {
          *found = true;
        }
      }
    }
    if (std::isspace(c) && !prevCharIsEscaped) {
      prevCharIsspace = true;
    }
    if (startPos > -1 && i < size) {
      if (c == '\\') {
        prevCharIsEscaped = true;
      }
      if (std::isspace(c) && !prevCharIsEscaped) {
        return value.c_str();
      }

      if (c != '\\') {
        value += c;
        prevCharIsEscaped = false;
      }
    }
  }
  esp3d_log("found *%s*", value.c_str());
  return value.c_str();
}

const char *ESP3DCommands::get_clean_param(ESP3DMessage *msg, uint start) {
  if (!msg) {
    esp3d_log_e("no message");
    return "";
  }
  static String value;
  bool prevCharIsEscaped = false;
  uint startp = start;
  while (char(msg->data[startp]) == ' ' && startp < msg->size) {
    startp++;
  }
  value = "";
  for (uint i = startp; i < msg->size; i++) {
    char c = char(msg->data[i]);
    if (c == '\\') {
      prevCharIsEscaped = true;
    }
    if (std::isspace(c) && !prevCharIsEscaped) {
      esp3d_log("testing *%s*", value.c_str());
      if (value == "json" || value.startsWith("json=") ||
          value.startsWith("pwd=")) {
        esp3d_log("clearing *%s*", value.c_str());
        value = "";
      } else {
        esp3d_log("returning *%s*", value.c_str());
        return value.c_str();
      }
    }
    if (c != '\\') {
      if ((std::isspace(c) && prevCharIsEscaped) || !std::isspace(c)) {
        value += c;
      }
      prevCharIsEscaped = false;
    }
  }
  esp3d_log("testing *%s*", value.c_str());
  // for empty value
  if (value == "json" || value.startsWith("json=") ||
      value.startsWith("pwd=")) {
    esp3d_log("clearing *%s*", value.c_str());
    value = "";
  }
  esp3d_log("returning *%s*", value.c_str());
  return value.c_str();
}

bool ESP3DCommands::has_param(ESP3DMessage *msg, uint start) {
  return strlen(get_clean_param(msg, start)) != 0;
}

void ESP3DCommands::execute_internal_command(int cmd, int cmd_params_pos,
                                             ESP3DMessage *msg) {
  if (!msg) {
    esp3d_log_e("no msg for cmd %d", cmd);
    return;
  }
#ifdef AUTHENTICATION_FEATURE
  String pwd = get_param(msg, cmd_params_pos, "pwd=");
  if (pwd.length() != 0 && (msg->origin == ESP3DClientType::serial ||
                            msg->origin == ESP3DClientType::serial_bridge ||
                            msg->origin == ESP3DClientType::telnet ||
                            msg->origin == ESP3DClientType::websocket ||
                            msg->origin == ESP3DClientType::usb_serial ||
                            msg->origin == ESP3DClientType::bluetooth)) {
    msg->authentication_level =
        AuthenticationService::getAuthenticatedLevel(pwd.c_str(), msg);
    // update authentication level for current client
    switch (msg->origin) {
      case ESP3DClientType::serial:
        esp3d_serial_service.setAuthentication(msg->authentication_level);
        break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
      case ESP3DClientType::serial_bridge:
        serial_bridge_service.setAuthentication(msg->authentication_level);
        break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
#if defined(USB_SERIAL_FEATURE)
      case ESP3DClientType::usb_serial:
        esp3d_usb_serial_service.setAuthentication(msg->authentication_level);
        break;
#endif  // USB_SERIAL_FEATURE
#if defined(TELNET_FEATURE)
      case ESP3DClientType::telnet:
        telnet_server.setAuthentication(msg->authentication_level);
        break;
#endif  // TELNET_FEATURE
#if defined(WS_DATA_FEATURE)
      case ESP3DClientType::websocket:
        websocket_data_server.setAuthentication(msg->authentication_level);
        break;
#endif  // WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
      case ESP3DClientType::bluetooth:
        bt_service.setAuthentication(msg->authentication_level);
        break;
#endif  // BLUETOOTH_FEATURE
      default:
        break;
    }
  }

#endif  // AUTHENTICATION_FEATURE
  switch (cmd) {
      // ESP3D Help
    //[ESP0] or [ESP]
    case 0:
      ESP0(cmd_params_pos, msg);
      break;

#if defined(WIFI_FEATURE)
    // STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
      ESP100(cmd_params_pos, msg);
      break;
    // STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
      ESP101(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) 
    // Change STA IP mode (DHCP/STATIC)
    //[ESP102]<mode>pwd=<admin password>
    case 102:
      ESP102(cmd_params_pos, msg);
      break;
    // Change STA IP/Mask/GW
    //[ESP103]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
    case 103:
      ESP103(cmd_params_pos, msg);
      break;
    // Set fallback mode which can be BT,  WIFI-AP, OFF
    //[ESP104]<state>pwd=<admin password>
    case 104:
      ESP104(cmd_params_pos, msg);
      break;
    // AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
      ESP105(cmd_params_pos, msg);
      break;
    // AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
      ESP106(cmd_params_pos, msg);
      break;
    // Change AP IP
    //[ESP107]<IP> pwd=<admin password>
    case 107:
      ESP107(cmd_params_pos, msg);
      break;
    // Change AP channel
    //[ESP108]<channel>pwd=<admin password>
    case 108:
      ESP108(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE

#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
      // Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA,
      // OFF
      //[ESP110]<state>pwd=<admin password>
    case 110:
      ESP110(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    // Get current IP
    //[ESP111]
    case 111:
      ESP111(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
    // Get/Set hostname
    //[ESP112]<Hostname> pwd=<admin password>
    case 112:
      ESP112(cmd_params_pos, msg);
      break;
    // Get/Set boot Network (WiFi/BT/Ethernet) state which can be ON, OFF
    //[ESP114]<state>pwd=<admin password>
    case 114:
      ESP114(cmd_params_pos, msg);
      break;
      // Get/Set immediate Network (WiFi/BT/Ethernet) state which can be ON, OFF
      //[ESP115]<state>pwd=<admin password>
    case 115:
      ESP115(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE

#if defined(ETH_FEATURE) 
    // Change ETH STA IP mode (DHCP/STATIC)
    //[ESP116]<mode>pwd=<admin password>
    case 116:
      ESP102(cmd_params_pos, msg);
      break;
    // Change ETH STA IP/Mask/GW
    //[ESP117]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
    case 117:
      ESP117(cmd_params_pos, msg);
      break;
    // Set fallback mode which can be BT, OFF
    //[ESP118]<state>pwd=<admin password>
    case 118:
      ESP118(cmd_params_pos, msg);
      break;
#endif  // ETH_FEATURE  

#ifdef HTTP_FEATURE
    // Set HTTP state which can be ON, OFF
    //[ESP120]<state>pwd=<admin password>
    case 120:
      ESP120(cmd_params_pos, msg);
      break;
    // Set HTTP port
    //[ESP121]<port>pwd=<admin password>
    case 121:
      ESP121(cmd_params_pos, msg);
      break;
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
    // Set TELNET state which can be ON, OFF
    //[ESP130]<state>pwd=<admin password>
    case 130:
      ESP130(cmd_params_pos, msg);
      break;
    // Set TELNET port
    //[ESP131]<port>pwd=<admin password>
    case 131:
      ESP131(cmd_params_pos, msg);
      break;
#endif  // TELNET_FEATURE
#ifdef TIMESTAMP_FEATURE
    // Sync / Set / Get current time
    //[ESP140]<SYNC>  <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <zone=xxx>
    //<dst=YES/NO> <time=YYYY-MM-DD#H24:MM:SS> pwd=<admin password>
    case 140:
      ESP140(cmd_params_pos, msg);
      break;
#endif  // TIMESTAMP_FEATURE
        // Get/Set display/set boot delay in ms / Verbose boot
        //[ESP150]<delay=time in
        // milliseconds><verbose=YES/NO>pwd=<admin password>
    case 150:
      ESP150(cmd_params_pos, msg);
      break;
#ifdef WS_DATA_FEATURE
    // Set WebSocket state which can be ON, OFF
    //[ESP160]<state>pwd=<admin password>
    case 160:
      ESP160(cmd_params_pos, msg);
      break;
    // Set WebSocket port
    //[ESP161]<port>pwd=<admin password>
    case 161:
      ESP161(cmd_params_pos, msg);
      break;
#endif  // WS_DATA_FEATURE
#ifdef CAMERA_DEVICE
      // Get/Set Camera command value / list all values in JSON/plain
      //[ESP170]label=<value> pwd=<admin/user password>
      // label can be:
      // light / framesize / quality / contrast / brightness / saturation /
      // gainceiling / colorbar / awb / agc / aec / hmirror / vflip /
      // awb_gain / agc_gain / aec_value / aec2 / cw / bpc / wpc / raw_gma /
      // lenc / special_effect / wb_mode / ae_level
    case 170:
      ESP170(cmd_params_pos, msg);
      break;
      // Save frame to target path and filename (default target = todaydate,
      // default name=timestamp.jpg) [ESP171]path=<target path>
      // filename = < target
      // filename> pwd=<admin/user password>
    case 171:
      ESP171(cmd_params_pos, msg);
      break;
#endif  // CAMERA_DEVICE
#ifdef FTP_FEATURE
    // Set Ftp state which can be ON, OFF
    //[ESP180]<state>pwd=<admin password>
    case 180:
      ESP180(cmd_params_pos, msg);
      break;
      // Set/get ftp ports
      //[ESP181]ctrl=<port> active=<port> passive=<port> pwd=<admin password >
      // case 181:
      ESP181(cmd_params_pos, msg);
      break;
#endif  // FTP_FEATURE
#ifdef WEBDAV_FEATURE
    // Set webdav state which can be ON, OFF
    //[ESP190]<state>pwd=<admin password>
    case 190:
      ESP190(cmd_params_pos, msg);
      break;
      // Set/get webdav port
      //[ESP191]ctrl=<port> active=<port> passive=<port> pwd=<admin password >
      // case 191:
      ESP191(cmd_params_pos, msg);
      break;
#endif  // WEBDAV_FEATURE
#if defined(SD_DEVICE)
    // Get/Set SD Card Status
    //[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password >
    case 200:
      ESP200(cmd_params_pos, msg);
      break;
#if SD_DEVICE != ESP_SDIO
    // Get/Set SD card Speed factor 1 2 4 6 8 16 32
    //[ESP202]SPEED=<value>pwd=<user/admin password>
    case 202:
      ESP202(cmd_params_pos, msg);
      break;
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
    // Get/Set SD Check at boot state which can be ON, OFF
    //[ESP402]<state>pwd=<admin password>
    case 402:
      ESP402(cmd_params_pos, msg);
      break;
#endif  // #ifdef SD_UPDATE_FEATURE
#endif  // SD_DEVICE
#ifdef DIRECT_PIN_FEATURE
    // Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]pwd=<admin password>
    case 201:
      ESP201(cmd_params_pos, msg);
      break;
#endif  // DIRECT_PIN_FEATURE
#ifdef SENSOR_DEVICE
    // Get SENSOR Value / type/Set SENSOR type
    //[ESP210] <TYPE> <type=NONE/xxx> <interval=XXX in millisec>
    case 210:
      ESP210(cmd_params_pos, msg);
      break;
#endif  // #ifdef SENSOR_DEVICE
#if defined(PRINTER_HAS_DISPLAY)
    // Output to printer screen status
    //[ESP212]<Text>json=<no> pwd=<user/admin password>
    case 212:
      ESP212(cmd_params_pos, msg);
      break;
#endif  // PRINTER_HAS_DISPLAY
#if defined(DISPLAY_DEVICE)
    // Output to esp screen status
    //[ESP214]<Text>pwd=<user password>
    case 214:
      ESP214(cmd_params_pos, msg);
      break;
#if defined(DISPLAY_TOUCH_DRIVER)
    // Touch Calibration
    //[ESP215]<CALIBRATE>[pwd=<user password>]
    case 215:
      ESP215(cmd_params_pos, msg);
      break;
#endif  // DISPLAY_TOUCH_DRIVER
#endif  // DISPLAY_DEVICE
#ifdef BUZZER_DEVICE
    // Play sound
    //[ESP250]F=<frequency> D=<duration> [pwd=<user password>]
    case 250:
      ESP250(cmd_params_pos, msg);
      break;
#endif  // BUZZER_DEVICE
    // Show pins
    //[ESP220][pwd=<user password>]
    case 220:
      ESP220(cmd_params_pos, msg);
      break;
    // Delay command
    //[ESP290]<delay in ms>[pwd=<user password>]
    case 290:
      ESP290(cmd_params_pos, msg);
      break;
#if defined(ESP_LUA_INTERPRETER_FEATURE)
    // Execute Lua script
    //[ESP300]<filename>
    case 300:
      ESP300(cmd_params_pos, msg);
      break;
    // Query and Control ESP300 execution
    //[ESP301]action=<PAUSE/RESUME/ABORT>
    case 301:
      ESP301(cmd_params_pos, msg);
      break;
#endif  // ESP_LUA_INTERPRETER_FEATURE

    // Get full ESP3D settings
    //[ESP400]<pwd=admin>
    case 400:
      ESP400(cmd_params_pos, msg);
      break;
    // Set EEPROM setting
    //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
    case 401:
      ESP401(cmd_params_pos, msg);
      break;
#if defined(WIFI_FEATURE)
    // Get available AP list (limited to 30)
    // msg is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410:
      ESP410(cmd_params_pos, msg);
      break;
#endif  // WIFI_FEATURE
    // Get ESP current status
    // msg is JSON or plain text according parameter
    //[ESP420]<plain>
    case 420:
      ESP420(cmd_params_pos, msg);
      break;
    // Set ESP State
    // cmd are RESTART / RESET
    //[ESP444]<cmd><pwd=admin>
    case 444:
      ESP444(cmd_params_pos, msg);
      break;
#ifdef MDNS_FEATURE
    // Get ESP3D list
    //[ESP450] pwd=<admin/user password>
    case 450:
      ESP450(cmd_params_pos, msg);
      break;
#endif  // MDNS_FEATURE
#ifdef AUTHENTICATION_FEATURE
      // Get current authentication  level
      //[ESP500] json=<no> pwd=<admin password>
    case 500:
      ESP500(cmd_params_pos, msg);
      break;
      // set/display session time out
      //[ESP510]<password> json=<no> pwd=<admin password>
    case 510:
      ESP510(cmd_params_pos, msg);
      break;
    // Change admin password
    //[ESP550]<password>pwd=<admin password>
    case 550:
      ESP550(cmd_params_pos, msg);
      break;
    // Change user password
    //[ESP555]<password>pwd=<admin/user password>
    case 555:
      ESP555(cmd_params_pos, msg);
      break;
#endif  // AUTHENTICATION_FEATURE
#if defined(NOTIFICATION_FEATURE)
    // Send Notification
    //[ESP600]<msg>[pwd=<admin password>]
    case 600:
      ESP600(cmd_params_pos, msg);
      break;
      // Set/Get Notification settings
      //[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE/HOMEASSISTANT> T1=<token1>
      // T2=<token2>
      // TS=<Settings> pwd=<admin password> Get will give type and settings only
      // not the protected T1/T2
    case 610:
      ESP610(cmd_params_pos, msg);
      break;
    // Send Notification using URL
    //[ESP620]URL=<encoded url> pwd=<admin password>
    case 620:
      ESP620(cmd_params_pos, msg);
      break;
#endif  // NOTIFICATION_FEATURE
#if defined(FILESYSTEM_FEATURE)
    // Format ESP Filesystem
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
      ESP710(cmd_params_pos, msg);
      break;
    // List ESP Filesystem
    //[ESP720]<Root> pwd=<admin password>
    case 720:
      ESP720(cmd_params_pos, msg);
      break;
    // Action on ESP Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP730]<Action>=<path> pwd=<admin password>
    case 730:
      ESP730(cmd_params_pos, msg);
      break;
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
    // Format ESP Filesystem
    //[ESP715]FORMATSD pwd=<admin password>
    case 715:
      ESP715(cmd_params_pos, msg);
      break;
#endif  // SD_DEVICE
#if defined(GCODE_HOST_FEATURE)
    // Open local file
    //[ESP700]<filename>
    case 700:
      ESP700(cmd_params_pos, msg);
      break;
    // Get Status and Control ESP700 stream
    //[ESP701]action=<PAUSE/RESUME/ABORT>
    case 701:
      ESP701(cmd_params_pos, msg);
      break;
#endif  // GCODE_HOST_FEATURE
#if defined(SD_DEVICE)
    // List SD Filesystem
    //[ESP740]<Root> pwd=<admin password>
    case 740:
      ESP740(cmd_params_pos, msg);
      break;
    // Action on SD Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP750]<Action>=<path> pwd=<admin password>
    case 750:
      ESP750(cmd_params_pos, msg);
      break;
#endif  // SD_DEVICE
#if defined(GLOBAL_FILESYSTEM_FEATURE)
    // List Global Filesystem
    //[ESP780]<Root> pwd=<admin password>
    case 780:
      ESP780(cmd_params_pos, msg);
      break;
    // Action on Global Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP790]<Action>=<path> pwd=<admin password>
    case 790:
      ESP790(cmd_params_pos, msg);
      break;
#endif  // GLOBAL_FILESYSTEM_FEATURE
    // Get fw version firmare target and fw version
    // eventually set time with pc time
    // output is JSON or plain text according parameter
    //[ESP800]<plain><time=YYYY-MM-DD-HH-MM-SS>
    case 800:
      ESP800(cmd_params_pos, msg);
      break;

#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
    // Get state / Set Enable / Disable Serial Communication
    //[ESP900]<ENABLE/DISABLE>
    case 900:
      ESP900(cmd_params_pos, msg);
      break;
    // Get / Set Serial Baud Rate
    //[ESP901]<BAUD RATE> json=<no> pwd=<admin/user password>
    case 901:
      ESP901(cmd_params_pos, msg);
      break;
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if defined(USB_SERIAL_FEATURE)
    // Get / Set USB Serial Baud Rate
    //[ESP902]<BAUD RATE> json=<no> pwd=<admin/user password>
    case 902:
      ESP902(cmd_params_pos, msg);
      break;
    // Get / Set Client Output
    case 950:
      ESP950(cmd_params_pos, msg);
      break;
#endif  // defined(USB_SERIAL_FEATURE)
#ifdef BUZZER_DEVICE
    // Get state / Set Enable / Disable buzzer
    //[ESP910]<ENABLE/DISABLE>
    case 910:
      ESP910(cmd_params_pos, msg);
      break;
#endif  // BUZZER_DEVICE

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    // Get state / Set Enable / Disable Serial Bridge Communication
    //[ESP930]<ENABLE/DISABLE>
    case 930:
      ESP930(cmd_params_pos, msg);
      break;
    // Get / Set Serial Bridge Baud Rate
    //[ESP931]<BAUD RATE> json=<no> pwd=<admin/user password>
    case 931:
      ESP931(cmd_params_pos, msg);
      break;
#endif  // defined(ESP_SERIAL_BRIDGE_OUTPUT)
#if defined(ARDUINO_ARCH_ESP32) &&                             \
    (CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32S2 || \
     CONFIG_IDF_TARGET_ESP32C3)
    case 999:
      // Set quiet boot if strapping pin is High
      //[ESP999]<QUIETBOOT> [pwd=<admin/user password>]
      ESP999(cmd_params_pos, msg);
      break;
#endif  // ARDUINO_ARCH_ESP32

    default:
      msg->target = msg->origin;
      esp3d_log("Invalid Command: %d", cmd);
      if (hasTag(msg, cmd_params_pos, "json")) {
        String tmpstr = "{\"cmd\":\"[ESP";
        tmpstr += String(cmd);
        tmpstr += "]\",\"status\":\"error\",\"data\":\"Invalid Command\"}";
        if (!dispatch(msg, tmpstr.c_str())) {
          esp3d_log_e("Out of memory");
        }
      } else {
        String tmpstr = "Invalid Command: [ESP";
        tmpstr += String(cmd);
        tmpstr += "]\n";
        if (!dispatch(msg, tmpstr.c_str())) {
          esp3d_log_e("Out of memory");
        }
      }
  }
}

bool ESP3DCommands::dispatchSetting(bool json, const char *filter,
                                    ESP3DSettingIndex index, const char *help,
                                    const char **optionValues,
                                    const char **optionLabels, uint32_t maxsize,
                                    uint32_t minsize, uint32_t minsize2,
                                    uint8_t precision, const char *unit,
                                    bool needRestart, ESP3DClientType target,
                                    ESP3DRequest requestId, bool isFirst) {
  String tmpstr;
  String value;
  tmpstr.reserve(
      350);  // to save time and avoid several memories allocation delay
  const ESP3DSettingDescription *elementSetting =
      ESP3DSettings::getSettingPtr(index);
  if (!elementSetting) {
    return false;
  }
  switch (elementSetting->type) {
    case ESP3DSettingType::byte_t:
      value = String(ESP3DSettings::readByte(index));
      break;
    case ESP3DSettingType::integer_t:
      value = String(ESP3DSettings::readUint32(index));
      break;
    case ESP3DSettingType::ip_t:
      value = ESP3DSettings::readIPString(index);
      break;
    case ESP3DSettingType::float_t:
      // TODO Add float support ?
      value = "Not supported";
      break;
    case ESP3DSettingType::mask:
      // TODO Add Mask support ?
      value = "Not supported";
      break;
    case ESP3DSettingType::bitsfield:
      // TODO Add bitfield support ?
      value = "Not supported";
      break;
    default:  // String
      if (index == ESP_STA_PASSWORD || index == ESP_AP_PASSWORD ||
#if defined(ESP3D_NOTIFICATIONS_FEATURE)
          index == ESP_NOTIFICATION_TOKEN1 ||
          index == ESP_NOTIFICATION_TOKEN2 ||
#endif  // ESP3D_NOTIFICATIONS_FEATURE

          index == ESP_ADMIN_PWD || index == ESP_USER_PWD) {  // hide passwords
                                                              // using  ********
        value = HIDDEN_PASSWORD;
      } else {
        value = ESP3DSettings::readString(index);
      }
  }
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    tmpstr += "{\"F\":\"";
    tmpstr += filter;
    tmpstr += "\",\"P\":\"";
    tmpstr += String(static_cast<uint16_t>(index));
    tmpstr += "\",\"T\":\"";
    switch (elementSetting->type) {
      case ESP3DSettingType::byte_t:
        tmpstr += "B";
        break;
      case ESP3DSettingType::integer_t:
        tmpstr += "I";
        break;
      case ESP3DSettingType::ip_t:
        tmpstr += "A";
        break;
      case ESP3DSettingType::float_t:
        tmpstr += "F";
        break;
      case ESP3DSettingType::mask:
        tmpstr += "M";
        break;
      case ESP3DSettingType::bitsfield:
        tmpstr += "X";
        break;
      default:
        tmpstr += "S";
    }
    tmpstr += "\",\"V\":\"";
    tmpstr += value;  // TODO: need to encode string ?
    tmpstr += "\",\"H\":\"";
    tmpstr += help;
    tmpstr += "\"";
    if (needRestart) {
      tmpstr += ",\"R\":\"1\"";
    }
    if (optionValues && optionLabels) {
      tmpstr += ",\"O\":[";
      for (uint8_t i = 0; i < maxsize; i++) {
        if (i > 0) {
          tmpstr += ",";
        }
        tmpstr += "{\"";
        // be sure we have same size for both array to avoid overflow
        tmpstr += optionLabels[i];
        tmpstr += "\":\"";
        tmpstr += optionValues[i];
        tmpstr += "\"}";
      }
      tmpstr += "]";
    }
    if (unit) {
      tmpstr += ",\"R\":\"";
      tmpstr += unit;
      tmpstr += "\"";
    }
    if (precision != ((uint8_t)-1)) {
      tmpstr += ",\"E\":\"";
      tmpstr += String(precision);
      tmpstr += "\"";
    }
    if (maxsize != (uint32_t)-1 && !optionValues) {
      tmpstr += ",\"S\":\"";
      tmpstr += String(maxsize);
      tmpstr += "\"";
    }
    if (minsize != (uint32_t)-1) {
      tmpstr += ",\"M\":\"";
      tmpstr += String(minsize);
      tmpstr += "\"";
    }
    if (minsize2 != (uint32_t)-1) {
      tmpstr += ",\"MS\":\"";
      tmpstr += String(minsize2);
      tmpstr += "\"";
    }
    tmpstr += "}";
  } else {
    tmpstr = filter;
    tmpstr += "/";
    tmpstr += help;
    tmpstr += ": ";
    tmpstr += value;
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::core);
}

bool ESP3DCommands::dispatchAuthenticationError(ESP3DMessage *msg, uint cmdid,
                                                bool json) {
  String tmpstr;
  if (!msg) {
    return false;
  }
#if defined(HTTP_FEATURE) && defined(AUTHENTICATION_FEATURE)
  if (msg->target == ESP3DClientType::http) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
  }
#endif  // HTTP_FEATURE
  // answer is one message, override for safety
  msg->type = ESP3DMessageType::unique;
  if (json) {
    tmpstr = "{\"cmd\":\"";
    tmpstr += String(cmdid);
    tmpstr +=
        "\",\"status\":\"error\",\"data\":\"Wrong authentication level\"}";
  } else {
    tmpstr = "Wrong authentication level\n";
  }
  return dispatch(msg, tmpstr.c_str());
}

bool ESP3DCommands::dispatchAnswer(ESP3DMessage *msg, uint cmdid, bool json,
                                   bool hasError, const char *answerMsg) {
  String tmpstr;
  if (!msg || !answerMsg) {
    esp3d_log_e("no msg");
    return false;
  }
  // answer is one message, override for safety
  msg->type = ESP3DMessageType::unique;
  if (json) {
    tmpstr = "{\"cmd\":\"" + String(cmdid) + "\",\"status\":\"";

    if (hasError) {
      tmpstr += "error";
    } else {
      tmpstr += "ok";
    }
    tmpstr += "\",\"data\":";
    if (answerMsg[0] != '{') {
      tmpstr += "\"";
    }
    tmpstr += answerMsg;
    if (answerMsg[0] != '{') {
      tmpstr += "\"";
    }
    tmpstr += "}\n";
  } else {
    tmpstr = answerMsg;
    tmpstr += "\n";
  }
  return dispatch(msg, tmpstr.c_str());
}

bool ESP3DCommands::dispatchKeyValue(bool json, const char *key,
                                     const char *value, ESP3DClientType target,
                                     ESP3DRequest requestId, bool nested,
                                     bool isFirst) {
  String tmpstr = "";
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    if (nested) {
      tmpstr += "{";
    }
    tmpstr += "\"";
  }
  tmpstr += key;
  if (json) {
    tmpstr += "\":\"";
  } else {
    tmpstr += ": ";
  }
  tmpstr += value;
  if (json) {
    tmpstr += "\"";
    if (nested) {
      tmpstr += "}";
    }
  } else {
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::core);
}

bool ESP3DCommands::dispatchIdValue(bool json, const char *Id,
                                    const char *value, ESP3DClientType target,
                                    ESP3DRequest requestId, bool isFirst) {
  String tmpstr = "";
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    tmpstr += "{\"id\":\"";
  }
  tmpstr += Id;
  if (json) {
    tmpstr += "\",\"value\":\"";
  } else {
    tmpstr += ": ";
  }
  tmpstr += value;
  if (json) {
    tmpstr += "\"}";
  } else {
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::core);
}

bool ESP3DCommands::formatCommand(char *cmd, size_t len) {
  uint sizestr = strlen(cmd);
  if (len > sizestr + 2) {
    cmd[sizestr] = '\n';
    cmd[sizestr + 1] = 0x0;
    return true;
  }
  if (sizestr == len && cmd[sizestr - 1] == '\n') {
    return true;
  }
  return false;
}

void ESP3DCommands::process(ESP3DMessage *msg) {
  static bool lastIsESP3D = false;
  if (!msg) {
    esp3d_log_e("no msg");
    return;
  }
  esp3d_log("Processing message %d", msg->size);
  if (is_esp_command(msg->data, msg->size)) {
    esp3d_log("Detected ESP command");
    lastIsESP3D = true;
    uint cmdId = 0;
    uint espcmdpos = 0;
    bool startcmd = false;
    bool endcmd = false;
    for (uint i = 0; i < msg->size && espcmdpos == 0; i++) {
      if (char(msg->data[i]) == ']') {  // start command flag
        endcmd = true;
        espcmdpos = i + 1;
      } else if (char(msg->data[i]) == '[') {  // end command flag
        startcmd = true;
      } else if (startcmd && !endcmd &&
                 std::isdigit(static_cast<unsigned char>(
                     char(msg->data[i])))) {  // command id
        if (cmdId != 0) {
          cmdId = (cmdId * 10);
        }
        cmdId += (msg->data[i] - 48);
      }
    }
    // execute esp command
    esp3d_log("Execute internal command %d", cmdId);
    execute_internal_command(cmdId, espcmdpos, msg);
  } else {
    // Work around to avoid to dispatch single \n or \r to everyone as it is
    // part of previous ESP3D command
    if (msg->size == 1 &&
        ((char(msg->data[0]) == '\n') || (char(msg->data[0]) == '\r')) &&
        lastIsESP3D) {
      lastIsESP3D = false;
      // delete message
      esp3d_log("Delete message");
      esp3d_message_manager.deleteMsg(msg);
      return;
    }
    lastIsESP3D = false;
    esp3d_log("Dispatch message: %s", msg->data);
    dispatch(msg);
  }
}
bool ESP3DCommands::dispatch(ESP3DMessage *msg, const char *sbuf) {
  return dispatch(msg, (uint8_t *)sbuf, strlen(sbuf));
}

bool ESP3DCommands::dispatch(ESP3DMessage *msg, uint8_t *sbuf, size_t len) {
  if (!msg) {
    esp3d_log_e("no msg");
    return false;
  }
  // check is need \n at the end of the command
  if (msg->type == ESP3DMessageType::unique ||
      msg->type == ESP3DMessageType::tail) {
    esp3d_log_d("unique or tail message :*%s*", (char *)sbuf);
    if (!formatCommand((char *)sbuf, len)) {
      esp3d_log("format command failed");
      String tmpstr = "";
      tmpstr.reserve(len + 2);
      for (uint i = 0; i < len; i++) {
        tmpstr += char(sbuf[i]);
      }
      tmpstr += '\n';
      esp3d_log("update command success: *%s*", tmpstr.c_str());
      if (!esp3d_message_manager.setDataContent(msg, (uint8_t *)tmpstr.c_str(),
                                               tmpstr.length())) {
        esp3d_log_e("set data content failed");
        esp3d_message_manager.deleteMsg(msg);
        return false;
      }
    } else {
      esp3d_log_d("format command success, no need to update");
      if (!esp3d_message_manager.setDataContent(msg, sbuf, len)) {
        esp3d_log_e("set data content failed");
        esp3d_message_manager.deleteMsg(msg);
        return false;
      }
    }
  } else {
    esp3d_log("not unique or tail message");
    if (msg->type == ESP3DMessageType::realtimecmd){
      esp3d_log_d("realtime command");
    }
    if (!esp3d_message_manager.setDataContent(msg, sbuf, len)) {
      esp3d_log_e("set data content failed");
      esp3d_message_manager.deleteMsg(msg);
      return false;
    }
  }
  return dispatch(msg);
}

bool ESP3DCommands::dispatch(uint8_t *sbuf, size_t size, ESP3DClientType target,
                             ESP3DRequest requestId, ESP3DMessageType type,
                             ESP3DClientType origin,
                             ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage *newMsgPtr = esp3d_message_manager.newMsg(origin, target);
  if (newMsgPtr) {
    newMsgPtr->request_id = requestId;
    newMsgPtr->type = type;
    newMsgPtr->authentication_level = authentication_level;
    return dispatch(newMsgPtr, sbuf, size);
  }
  esp3d_log_e("no newMsgPtr");
  return false;
}
bool ESP3DCommands::dispatch(const char *sbuf, ESP3DClientType target,
                             ESP3DRequest requestId, ESP3DMessageType type,
                             ESP3DClientType origin,
                             ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage *newMsgPtr = esp3d_message_manager.newMsg(origin, target);
  if (newMsgPtr) {
    newMsgPtr->request_id = requestId;
    newMsgPtr->type = type;
    newMsgPtr->authentication_level = authentication_level;
    return dispatch(newMsgPtr, sbuf);
  }
  esp3d_log_e("no newMsgPtr");
  return false;
}

ESP3DClientType ESP3DCommands::getOutputClient(bool fromSettings) {
//It is a setting only if there is a choice between USB/Serial 
#if defined(USB_SERIAL_FEATURE)
  if (fromSettings) {
    _output_client =  (ESP3DClientType)ESP3DSettings::readByte(ESP_OUTPUT_CLIENT);
  }
  return _output_client;
#else
  (void)fromSettings;
  //if not setting, then it is the default one, which is hardcoded
#endif
  esp3d_log("OutputClient: %d %s", static_cast<uint8_t>(_output_client),
            GETCLIENTSTR(_output_client));
  return _output_client;

}

bool ESP3DCommands::dispatch(ESP3DMessage *msg) {
  bool sendOk = true;
  String tmp;
  esp3d_log("Dispatch message data: %s", (const char *)msg->data);
  if (!msg) {
    esp3d_log_e("no msg");
    return false;
  }
  // currently only echo back no test done on success
  // TODO check add is successful
  switch (msg->target) {
    case ESP3DClientType::no_client:
      esp3d_log("No client message");
      esp3d_message_manager.deleteMsg(msg);
      break;
#if COMMUNICATION_PROTOCOL == RAW_SERIAL
    case ESP3DClientType::serial:
      esp3d_log("Serial message");
      if (!esp3d_serial_service.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Serial dispatch failed");
      }
      break;
#if defined(USB_SERIAL_FEATURE)
    case ESP3DClientType::usb_serial:
      esp3d_log("USB Serial message");
      if (!esp3d_usb_serial_service.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("USB Serial dispatch failed");
      }
      break;
#endif  // USB_SERIAL_FEATURE
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL

#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    case ESP3DClientType::echo_serial:
      esp3d_log("Echo serial message");
      MYSERIAL1.write(msg->data, msg->size);
      if (msg->type == ESP3DMessageType::unique ||
          msg->type == ESP3DMessageType::tail) {
        if (msg->data[msg->size - 1] != '\n') {
          MYSERIAL1.write('\n');
        }
      }
      esp3d_message_manager.deleteMsg(msg);
      break;

    case ESP3DClientType::socket_serial:
      esp3d_log("Socket serial message");
      if (!Serial2Socket.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Socket dispatch failed");
      }
      break;
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL

#if defined(ESP_LUA_INTERPRETER_FEATURE)
    case ESP3DClientType::lua_script:
      esp3d_log("Lua script message");
      if (!esp3d_lua_interpreter.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Lua script dispatch failed");
      }
      break;
#endif  // ESP_LUA_INTERPRETER_FEATURE

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case ESP3DClientType::serial_bridge:
      esp3d_log("Serial bridge message");
      if (!serial_bridge_service.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Serial bridge dispatch failed");
      }
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#ifdef WS_DATA_FEATURE
    case ESP3DClientType::websocket:
      esp3d_log("Websocket message");
      if (!websocket_data_server.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Telnet dispatch failed");
      }
      break;
#endif  // WS_DATA_FEATURE

#ifdef TELNET_FEATURE
    case ESP3DClientType::telnet:
      esp3d_log("Telnet message");
      if (!telnet_server.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Telnet dispatch failed");
      }
      break;
#endif  // TELNET_FEATURE

#ifdef BLUETOOTH_FEATURE
    case ESP3DClientType::bluetooth:
      esp3d_log("Bluetooth message");
      if (!bt_service.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Bluetooth dispatch failed");
      }
      break;
#endif  // BLUETOOTH_FEATURE

#ifdef HTTP_FEATURE
    case ESP3DClientType::webui_websocket:
      esp3d_log("Webui websocket message");
      if (!websocket_terminal_server.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Webui websocket dispatch failed");
      }
      break;
    case ESP3DClientType::http:
      esp3d_log("Http message");
      if (!HTTP_Server::dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Webui websocket dispatch failed");
      }
      break;
#endif  // HTTP_FEATURE
#if defined(DISPLAY_DEVICE)
    case ESP3DClientType::rendering:
      esp3d_log("Rendering message");
      if (!esp3d_display.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Display dispatch failed");
      }
      break;
#endif  // defined(DISPLAY_DEVICE)

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    case ESP3DClientType::mks_serial:
      esp3d_log("MKS Serial message");
      if (!MKSService::dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("MKS Serial dispatch failed");
      }
      break;
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

#if defined(GCODE_HOST_FEATURE)
    case ESP3DClientType::stream:
      esp3d_log("Gcode host message");
      if (!esp3d_gcode_host.dispatch(msg)) {
        sendOk = false;
        esp3d_log_e("Gcode host dispatch failed");
      }
      break;
#endif  // GCODE_HOST_FEATURE

#ifdef PRINTER_HAS_DISPLAY
    case ESP3DClientType::remote_screen:
      if (ESP3DSettings::GetFirmwareTarget() == GRBL ||
      ESP3DSettings::GetFirmwareTarget() == GRBLHAL) {
        esp3d_log_e("Remote screen message is not supported for GRBL");
        sendOk = false;
        break;
      }
      esp3d_log("Remote screen message");
      // change target to output client
      msg->target = getOutputClient();
      // change text to GCODE M117
      tmp = "M117 ";
      tmp += (const char *)msg->data;
      // replace end of line with space
      tmp.replace("\n", " ");
      tmp.replace("\r", "");
      tmp += "\n";
      if (esp3d_message_manager.setDataContent(msg, (uint8_t *)tmp.c_str(),
                                              tmp.length())) {
        return dispatch(msg);
      }
      sendOk = false;
      esp3d_log_e("Cannot set data content for remote screen");
      break;
#endif  // PRINTER_HAS_DISPLAY
    case ESP3DClientType::all_clients:
      esp3d_log("All clients message");
      // Add each client one by one
#ifdef PRINTER_HAS_DISPLAY
      if (msg->origin != ESP3DClientType::remote_screen &&
          msg->origin != getOutputClient()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::remote_screen;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::remote_screen;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for remote screen");
          }
        }
      }
#endif  // PRINTER_HAS_DISPLAY

#if defined(ESP_LUA_INTERPRETER_FEATURE)
      if (msg->origin != ESP3DClientType::lua_script &&
          msg->origin == getOutputClient() && esp3d_lua_interpreter.isScriptRunning()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::lua_script;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::lua_script;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for lua script");
          }
        }
      }
#endif  // ESP_LUA_INTERPRETER_FEATURE

#if defined(GCODE_HOST_FEATURE)
      if (msg->origin != ESP3DClientType::stream && esp3d_gcode_host.getStatus() != HOST_NO_STREAM) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::stream;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::stream;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for gcode host");
          }
        }
      }
#endif

#if defined(DISPLAY_DEVICE)
      if (msg->origin != ESP3DClientType::rendering &&
          msg->origin != getOutputClient()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::rendering;
          msg->request_id.id = ESP_OUTPUT_STATUS;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::rendering;
            copy_msg->request_id.id = ESP_OUTPUT_STATUS;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for display");
          }
        }
      }
#endif  // defined(DISPLAY_DEVICE)

#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
      if (msg->origin != ESP3DClientType::echo_serial &&
          msg->origin != ESP3DClientType::socket_serial) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::echo_serial;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::echo_serial;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for echo serial");
          }
        }
      }
#endif  // COMMUNICATION_PROTOCOL == SOCKET_SERIAL

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
      if (msg->origin != ESP3DClientType::serial_bridge) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::serial_bridge;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::serial_bridge;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for serial bridge");
          }
        }
      }
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#ifdef BLUETOOTH_FEATURE
      if (msg->origin != ESP3DClientType::bluetooth &&
          bt_service.isConnected()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::bluetooth;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::bluetooth;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for bluetooth");
          }
        }
      }
#endif  // BLUETOOTH_FEATURE

#ifdef TELNET_FEATURE
      if (msg->origin != ESP3DClientType::telnet &&
          telnet_server.isConnected()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::telnet;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::telnet;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for telnet");
          }
        }
      } else {
        if (msg->origin != ESP3DClientType::telnet)
          esp3d_log("Telnet not connected");
      }
#endif  // TELNET_FEATURE

#ifdef HTTP_FEATURE  // http cannot be in all client because it depend of any
                     // connection of the server
      if (msg->origin != ESP3DClientType::webui_websocket &&
          websocket_terminal_server.isConnected()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::webui_websocket;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::webui_websocket;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for webui_websocket");
          }
        }
      } else {
        if (msg->origin != ESP3DClientType::webui_websocket)
          esp3d_log("Webui websocket not connected");
      }
#endif  // HTTP_FEATURE

#ifdef WS_DATA_FEATURE
      if (msg->origin != ESP3DClientType::websocket &&
          websocket_data_server.isConnected()) {
        if (msg->target == ESP3DClientType::all_clients) {
          // become the reference message
          msg->target = ESP3DClientType::websocket;
        } else {
          // duplicate message because current is  already pending
          ESP3DMessage *copy_msg = esp3d_message_manager.copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = ESP3DClientType::websocket;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for websocket");
          }
        }
      } else {
        if (msg->origin != ESP3DClientType::websocket)
          esp3d_log("Websocket not connected");
      }
#endif  // WS_DATA_FEATURE

      //...

      // Send pending if any or cancel message is no client did handle it
      if (msg->target == ESP3DClientType::all_clients) {
        esp3d_log("No client handled message, send pending");
        sendOk = false;
      } else {
        return dispatch(msg);
      }
      break;
    default:
      esp3d_log_e("No valid target specified %d for %s",
                  static_cast<uint8_t>(msg->target), (char *)msg->data);
      sendOk = false;
  }
  // clear message
  if (!sendOk) {
    esp3d_log_e("Send msg failed");
    esp3d_message_manager.deleteMsg(msg);
  }
  return sendOk;
}
