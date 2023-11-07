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
#include "commands.h"

#include "../include/esp3d_config.h"
#include "esp3d.h"
#include "esp3doutput.h"
#include "settings_esp3d.h"

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../modules/mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

Commands esp3d_commands;

Commands::Commands() {}

Commands::~Commands() {}

// dispatch the command
void Commands::process(uint8_t *sbuf, size_t len, ESP3DOutput *output,
                       level_authenticate_type auth, ESP3DOutput *outputonly,
                       uint8_t outputignore) {
  static bool lastIsESP3D = false;
  log_esp3d("Client is %d, has only %d, has ignore %d",
            output ? output->client() : 0,
            outputonly ? outputonly->client() : 0, outputignore);
  if (is_esp_command(sbuf, len)) {
    lastIsESP3D = true;
    size_t slen = len;
    String tmpbuf = (const char *)sbuf;
    if (tmpbuf.startsWith("echo:")) {
      tmpbuf.replace("echo: ", "");
      tmpbuf.replace("echo:", "");
      slen = tmpbuf.length();
    }

    uint8_t cmd[4] = {0, 0, 0, 0};
    cmd[0] = tmpbuf[4] == ']' ? 0 : tmpbuf[4];
    cmd[1] = tmpbuf[5] == ']' ? 0 : tmpbuf[5];
    cmd[2] = tmpbuf[6] == ']' ? 0 : tmpbuf[6];
    cmd[3] = 0x0;
    log_esp3d("It is ESP command %s", cmd);
    log_esp3d("Respond to client  %d", (outputonly == nullptr)
                                           ? output->client()
                                           : outputonly->client());
    execute_internal_command(
        String((const char *)cmd).toInt(),
        (slen > (strlen((const char *)cmd) + 5))
            ? (const char *)&tmpbuf[strlen((const char *)cmd) + 5]
            : "",
        auth, (outputonly == nullptr) ? output : outputonly);
  } else {
    // Work around to avoid to dispatch single \n to everyone as it is part of
    // previous ESP3D command
    if (lastIsESP3D && len == 1 && sbuf[0] == '\n') {
      lastIsESP3D = false;
      return;
    }
    lastIsESP3D = false;
    // Dispatch to all clients but current or to define output
#if defined(HTTP_FEATURE)
    // the web command will never get answer as answer go to websocket
    // This is sanity check as the http client should already answered
    if (output->client() == ESP_HTTP_CLIENT && !output->footerSent()) {
      if (auth != LEVEL_GUEST) {
        output->printMSG("");
      } else {
        output->printERROR("Wrong authentication!", 401);
        return;
      }
    }
#endif  // HTTP_FEATURE
    if (outputonly == nullptr) {
      log_esp3d("Dispatch from %d, but %d", output->client(), outputignore);
      output->dispatch(sbuf, len, outputignore);
    } else {
      log_esp3d("Dispatch from %d to only  %d", output->client(),
                outputonly->client());
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
      if (outputonly->client() == ESP_SERIAL_CLIENT) {
        MKSService::sendGcodeFrame((const char *)sbuf);
      } else {
        outputonly->write(sbuf, len);
      }
#else
      outputonly->write(sbuf, len);
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
    }
  }
}

// check if current line is an [ESPXXX] command
bool Commands::is_esp_command(uint8_t *sbuf, size_t len) {
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
  if ((char(sbuf[0]) == 'e') && (char(sbuf[1]) == 'c') &&
      (char(sbuf[2]) == 'h') && (char(sbuf[3]) == 'o') &&
      (char(sbuf[4]) == ':') && (char(sbuf[5]) == ' ') &&
      (char(sbuf[6]) == '[') && (char(sbuf[7]) == 'E')) {
    if (len >= 14) {
      if ((char(sbuf[8]) == 'S') && (char(sbuf[9]) == 'P') &&
          ((char(sbuf[4]) == ']') || (char(sbuf[5]) == ']') ||
           (char(sbuf[6]) == ']') || (char(sbuf[7]) == ']'))) {
        return true;
      }
    }
  }
  return false;
}

// find space in string
// if space is has \ before it is ignored
int Commands::get_space_pos(const char *string, uint from) {
  uint len = strlen(string);
  if (len < from) {
    return -1;
  }
  for (uint i = from; i < len; i++) {
    if (string[i] == ' ') {
      // if it is first char
      if (i == from) {
        return from;
      }
      // if not first one and previous char is not '\'
      if (string[i - 1] != '\\') {
        return (i);
      }
    }
  }
  return -1;
}

// return first label but pwd using labelseparator (usualy =) like
// mylabel=myvalue will return mylabel
const char *Commands::get_label(const char *cmd_params,
                                const char *labelseparator,
                                uint8_t startindex) {
  static String res;
  String tmp = "";
  res = "";
  int start = 1;
  int end = -1;
  res = cmd_params;
  res.replace("\r ", "");
  res.replace("\n ", "");
  res.trim();
  if ((res.length() == 0) || (startindex >= res.length())) {
    return res.c_str();
  }

  if (strlen(labelseparator) > 0) {
    end = res.indexOf(labelseparator, startindex);
    if (end == -1) {
      return "";
    }
    start = end;
    for (int8_t p = end; p >= startindex; p--, start--) {
      if (res[p] == ' ') {
        p = -1;
        start += 2;
      }
    }
    if (start == -1) {
      start = 0;
    }
    if (start > end) {
      return "";
    }
    tmp = res.substring(start, end);
    if (tmp == "pwd") {
      res = get_label(cmd_params, labelseparator, end + 1);
    } else {
      res = tmp;
    }
  }
  return res.c_str();
}

const char *Commands::format_response(uint cmdID, bool isjson, bool isok,
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

const char *Commands::clean_param(const char *cmd_params) {
  static String res;
  res = cmd_params;
  if (strlen(cmd_params) == 0) {
    return "";
  }
  String tmp = cmd_params;
  tmp.trim();
  if (tmp == "json" || tmp.startsWith("json ")) {
    return "";
  }
  if (tmp.indexOf("json=") != -1) {
    // remove formating flag
    res = tmp.substring(0, tmp.indexOf("json="));
  } else {
    if (tmp.endsWith(" json")) {
      // remove formating flag
      res = tmp.substring(0, tmp.length() - 5);
    }
  }
  return res.c_str();
}

// extract parameter with corresponding label
// if label is empty give whole line without authentication label/parameter
const char *Commands::get_param(const char *cmd_params, const char *label) {
  static String res;
  res = "";
  int start = 1;
  int end = -1;
  String tmp = "";
  String slabel = " ";
  res = cmd_params;
  res.replace("\r ", "");
  res.replace("\n ", "");
  res.trim();
  if (res.length() == 0) {
    return res.c_str();
  }

  tmp = " " + res;
  slabel += label;
  if (strlen(label) > 0) {
    start = tmp.indexOf(slabel);
    if (start == -1) {
      return "";
    }
    start += slabel.length();
    end = get_space_pos(tmp.c_str(), start);
  }
  if (end == -1) {
    end = tmp.length();
  }
  // extract parameter
  res = tmp.substring(start, end);

#ifdef AUTHENTICATION_FEATURE
  // if no label remove authentication parameters
  if (strlen(label) == 0) {
    tmp = " " + res;
    start = tmp.indexOf(" pwd=");
    if (start != -1) {
      end = get_space_pos(tmp.c_str(), start + 1);
      res = "";
      if (start != 0) {
        res = tmp.substring(0, start);
      }
      if (end != -1) {
        res += " " + tmp.substring(end + 1, tmp.length());
      }
    }
  }
#endif  // AUTHENTICATION_FEATURE
  // remove space format
  res.replace("\\ ", " ");
  // be sure no extra space
  res.trim();
  return res.c_str();
}

bool Commands::has_tag(const char *cmd_params, const char *tag) {
  log_esp3d("Checking for tag: %s, in %s", tag, cmd_params);
  String tmp = "";
  String stag = " ";
  if ((strlen(cmd_params) == 0) || (strlen(tag) == 0)) {
    log_esp3d_e("No value provided for tag");
    return false;
  }
  stag += tag;
  tmp = cmd_params;
  tmp.trim();
  tmp = " " + tmp;
  if (tmp.indexOf(stag) == -1) {
    log_esp3d("No tag detected");
    return false;
  }
  log_esp3d("Tag detected");
  // to support plain , plain=yes , plain=no
  String param = String(tag) + "=";
  log_esp3d("Checking  %s , in %s", param.c_str(), cmd_params);
  String parameter = get_param(cmd_params, param.c_str());
  if (parameter.length() != 0) {
    log_esp3d("Parameter is %s", parameter.c_str());
    if (parameter == "YES" || parameter == "true" || parameter == "TRUE" ||
        parameter == "yes" || parameter == "1") {
      return true;
    }
    log_esp3d("No parameter to enable  %s ", param.c_str());
    return false;
  }
  log_esp3d("No parameter for %s but tag detected", param.c_str());
  return true;
}

// execute internal command
bool Commands::execute_internal_command(int cmd, const char *cmd_params,
                                        level_authenticate_type auth_level,
                                        ESP3DOutput *output) {
#ifndef SERIAL_COMMAND_FEATURE
  if (output->client() == ESP_SERIAL_CLIENT) {
    output->printMSG("Feature disabled");
    return false;
  }
#endif  // SERIAL_COMMAND_FEATURE
  bool response = true;
  level_authenticate_type auth_type = auth_level;
  // log_esp3d("Authentication = %d", auth_type);
// override if parameters
#ifdef AUTHENTICATION_FEATURE

  // do not overwrite previous authetic <time=YYYY-MM-DD#H24:MM:SS>ation level
  if (auth_type == LEVEL_GUEST) {
    String pwd = get_param(cmd_params, "pwd=");
    auth_type = AuthenticationService::authenticated_level(pwd.c_str(), output);
  }
#endif  // AUTHENTICATION_FEATURE
  // log_esp3d("Authentication = %d", auth_type);
  String parameter;
  switch (cmd) {
    // ESP3D Help
    //[ESP0] or [ESP]
    case 0:
      response = ESP0(cmd_params, auth_type, output);
      break;
#if defined(WIFI_FEATURE)
    // STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
      response = ESP100(cmd_params, auth_type, output);
      break;
    // STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
      response = ESP101(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    // Change STA IP mode (DHCP/STATIC)
    //[ESP102]<mode>pwd=<admin password>
    case 102:
      response = ESP102(cmd_params, auth_type, output);
      break;
    // Change STA IP/Mask/GW
    //[ESP103]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
    case 103:
      response = ESP103(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE ||ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
    // Set fallback mode which can be BT,  WIFI-AP, OFF
    //[ESP104]<state>pwd=<admin password>
    case 104:
      response = ESP104(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE)
#if defined(WIFI_FEATURE)
    // AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
      response = ESP105(cmd_params, auth_type, output);
      break;
    // AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
      response = ESP106(cmd_params, auth_type, output);
      break;
    // Change AP IP
    //[ESP107]<IP> pwd=<admin password>
    case 107:
      response = ESP107(cmd_params, auth_type, output);
      break;
    // Change AP channel
    //[ESP108]<channel>pwd=<admin password>
    case 108:
      response = ESP108(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE

#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
    // Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA, OFF
    //[ESP110]<state>pwd=<admin password>
    case 110:
      response = ESP110(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    // Get current IP
    //[ESP111]
    case 111:
      response = ESP111(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
    // Get/Set hostname
    //[ESP112]<Hostname> pwd=<admin password>
    case 112:
      response = ESP112(cmd_params, auth_type, output);
      break;
    // Get/Set boot Network (WiFi/BT/Ethernet) state which can be ON, OFF
    //[ESP114]<state>pwd=<admin password>
    case 114:
      response = ESP114(cmd_params, auth_type, output);
      break;
    // Get/Set immediate Network (WiFi/BT/Ethernet) state which can be ON, OFF
    //[ESP115]<state>pwd=<admin password>
    case 115:
      response = ESP115(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE

#ifdef HTTP_FEATURE
    // Set HTTP state which can be ON, OFF
    //[ESP120]<state>pwd=<admin password>
    case 120:
      response = ESP120(cmd_params, auth_type, output);
      break;
    // Set HTTP port
    //[ESP121]<port>pwd=<admin password>
    case 121:
      response = ESP121(cmd_params, auth_type, output);
      break;
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
    // Set TELNET state which can be ON, OFF
    //[ESP130]<state>pwd=<admin password>
    case 130:
      response = ESP130(cmd_params, auth_type, output);
      break;
    // Set TELNET port
    //[ESP131]<port>pwd=<admin password>
    case 131:
      response = ESP131(cmd_params, auth_type, output);
      break;
#endif  // TELNET_FEATURE
#ifdef TIMESTAMP_FEATURE
    // Sync / Set / Get current time
    //[ESP140]<SYNC>  <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <zone=xxx>
    //<dst=YES/NO> <time=YYYY-MM-DD#H24:MM:SS> pwd=<admin password>
    case 140:
      response = ESP140(cmd_params, auth_type, output);
      break;
#endif  // TIMESTAMP_FEATURE
    // Get/Set display/set boot delay in ms / Verbose boot
    //[ESP150]<delay=time in milliseconds><verbose=YES/NO>[pwd=<admin password>]
    case 150:
      response = ESP150(cmd_params, auth_type, output);
      break;
#ifdef WS_DATA_FEATURE
    // Set WebSocket state which can be ON, OFF
    //[ESP160]<state>pwd=<admin password>
    case 160:
      response = ESP160(cmd_params, auth_type, output);
      break;
    // Set WebSocket port
    //[ESP161]<port>pwd=<admin password>
    case 161:
      response = ESP161(cmd_params, auth_type, output);
      break;
#endif  // WS_DATA_FEATURE
#ifdef CAMERA_DEVICE
    // Get/Set Camera command value / list all values in JSON/plain
    //[ESP170]label=<value> pwd=<admin/user password>
    // label can be:
    // light/framesize/quality/contrast/brightness/saturation/gainceiling/colorbar/awb/agc/aec/hmirror/vflip/awb_gain/agc_gain/aec_value/aec2/cw/bpc/wpc/raw_gma/lenc/special_effect/wb_mode/ae_level
    case 170:
      response = ESP170(cmd_params, auth_type, output);
      break;
    // Save frame to target path and filename (default target = today date,
    // default name=timestamp.jpg) [ESP171]path=<target path> filename=<target
    // filename> pwd=<admin/user password>
    case 171:
      response = ESP171(cmd_params, auth_type, output);
      break;
#endif  // CAMERA_DEVICE
#ifdef FTP_FEATURE
    // Set Ftp state which can be ON, OFF
    //[ESP180]<state>pwd=<admin password>
    case 180:
      response = ESP180(cmd_params, auth_type, output);
      break;
    // Set/get ftp ports
    //[ESP181]ctrl=<port> active=<port> passive=<port> pwd=<admin password>
    case 181:
      response = ESP181(cmd_params, auth_type, output);
      break;
#endif  // FTP_FEATURE
#ifdef WEBDAV_FEATURE
    // Set webdav state which can be ON, OFF
    //[ESP190]<state>pwd=<admin password>
    case 190:
      response = ESP190(cmd_params, auth_type, output);
      break;
    // Set/get webdav port
    //[ESP191]ctrl=<port> active=<port> passive=<port> pwd=<admin password>
    case 191:
      response = ESP191(cmd_params, auth_type, output);
      break;
#endif  // WEBDAV_FEATURE
#if defined(SD_DEVICE)
    // Get/Set SD Card Status
    //[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password>
    case 200:
      response = ESP200(cmd_params, auth_type, output);
      break;
#if SD_DEVICE != ESP_SDIO
    // Get/Set SD card Speed factor 1 2 4 6 8 16 32
    //[ESP202]SPEED=<value>pwd=<user/admin password>
    case 202:
      response = ESP202(cmd_params, auth_type, output);
      break;
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
    // Get/Set SD Check at boot state which can be ON, OFF
    //[ESP402]<state>pwd=<admin password>
    case 402:
      response = ESP402(cmd_params, auth_type, output);
      break;
#endif  // #ifdef SD_UPDATE_FEATURE
#endif  // SD_DEVICE
#ifdef DIRECT_PIN_FEATURE
    // Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]pwd=<admin password>
    case 201:
      response = ESP201(cmd_params, auth_type, output);
      break;
#endif  // DIRECT_PIN_FEATURE
#ifdef SENSOR_DEVICE
    // Get SENSOR Value / type/Set SENSOR type
    //[ESP210] <TYPE> <type=NONE/xxx> <interval=XXX in millisec>
    case 210:
      response = ESP210(cmd_params, auth_type, output);
      break;
#endif  // #ifdef SENSOR_DEVICE
#if defined(DISPLAY_DEVICE)
    // Output to esp screen status
    //[ESP214]<Text>pwd=<user password>
    case 214:
      response = ESP214(cmd_params, auth_type, output);
      break;
#if defined(DISPLAY_TOUCH_DRIVER)
    // Touch Calibration
    //[ESP215]<CALIBRATE>[pwd=<user password>]
    case 215:
      response = ESP215(cmd_params, auth_type, output);
      break;
#endif  // DISPLAY_TOUCH_DRIVER
#ifdef BUZZER_DEVICE
    // Play sound
    //[ESP250]F=<frequency> D=<duration> [pwd=<user password>]
    case 250:
      response = ESP250(cmd_params, auth_type, output);
      break;
#endif  // BUZZER_DEVICE
#endif  // DISPLAY_DEVICE
    // Show pins
    //[ESP220][pwd=<user password>]
    case 220:
      response = ESP220(cmd_params, auth_type, output);
      break;
    // Delay command
    //[ESP290]<delay in ms>[pwd=<user password>]
    case 290:
      response = ESP290(cmd_params, auth_type, output);
      break;
    // Get full ESP3D settings
    //[ESP400]<pwd=admin>
    case 400:
      response = ESP400(cmd_params, auth_type, output);
      break;
    // Set EEPROM setting
    //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
    case 401:
      response = ESP401(cmd_params, auth_type, output);
      break;
#if defined(WIFI_FEATURE)
    // Get available AP list (limited to 30)
    // output is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410:
      response = ESP410(cmd_params, auth_type, output);
      break;
#endif  // WIFI_FEATURE
    // Get ESP current status
    // output is JSON or plain text according parameter
    //[ESP420]<plain>
    case 420:
      response = ESP420(cmd_params, auth_type, output);
      break;
    // Set ESP State
    // cmd are RESTART / RESET
    //[ESP444]<cmd><pwd=admin>
    case 444:
      response = ESP444(cmd_params, auth_type, output);
      break;
#ifdef MDNS_FEATURE
    // Get ESP3D list
    //[ESP450] pwd=<admin/user password>
    case 450:
      response = ESP450(cmd_params, auth_type, output);
      break;
#endif  // MDNS_FEATURE
#ifdef AUTHENTICATION_FEATURE
    // Change admin password
    //[ESP550]<password>pwd=<admin password>
    case 550:
      response = ESP550(cmd_params, auth_type, output);
      break;
    // Change user password
    //[ESP555]<password>pwd=<admin/user password>
    case 555:
      response = ESP555(cmd_params, auth_type, output);
      break;
#endif  // AUTHENTICATION_FEATURE
#if defined(NOTIFICATION_FEATURE)
    // Send Notification
    //[ESP600]<msg>[pwd=<admin password>]
    case 600:
      response = ESP600(cmd_params, auth_type, output);
      break;
    // Set/Get Notification settings
    //[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE> T1=<token1> T2=<token2>
    // TS=<Settings> [pwd=<admin password>] Get will give type and settings only
    // not the protected T1/T2
    case 610:
      response = ESP610(cmd_params, auth_type, output);
      break;
    // Send Notification using URL
    //[ESP620]URL=<encoded url> [pwd=<admin password>]
    case 620:
      response = ESP620(cmd_params, auth_type, output);
      break;
#endif  // NOTIFICATION_FEATURE
#if defined(FILESYSTEM_FEATURE)
    // Format ESP Filesystem
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
      response = ESP710(cmd_params, auth_type, output);
      break;
    // List ESP Filesystem
    //[ESP720]<Root> pwd=<admin password>
    case 720:
      response = ESP720(cmd_params, auth_type, output);
      break;
    // Action on ESP Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP730]<Action>=<path> pwd=<admin password>
    case 730:
      response = ESP730(cmd_params, auth_type, output);
      break;
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
    // Format ESP Filesystem
    //[ESP715]FORMATSD pwd=<admin password>
    case 715:
      response = ESP715(cmd_params, auth_type, output);
      break;
#endif  // SD_DEVICE
#if defined(GCODE_HOST_FEATURE)
    // Open local file
    //[ESP700]<filename>
    case 700:
      response = ESP700(cmd_params, auth_type, output);
      break;
    // Get Status and Control ESP700 stream
    //[ESP701]action=<PAUSE/RESUME/ABORT>
    case 701:
      response = ESP701(cmd_params, auth_type, output);
      break;
#endif  // GCODE_HOST_FEATURE
#if defined(SD_DEVICE)
    // List SD Filesystem
    //[ESP740]<Root> pwd=<admin password>
    case 740:
      response = ESP740(cmd_params, auth_type, output);
      break;
    // Action on SD Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP750]<Action>=<path> pwd=<admin password>
    case 750:
      response = ESP750(cmd_params, auth_type, output);
      break;
#endif  // SD_DEVICE
#if defined(GLOBAL_FILESYSTEM_FEATURE)
    // List Global Filesystem
    //[ESP780]<Root> pwd=<admin password>
    case 780:
      response = ESP780(cmd_params, auth_type, output);
      break;
    // Action on Global Filesystem
    // rmdir / remove / mkdir / exists
    //[ESP790]<Action>=<path> pwd=<admin password>
    case 790:
      response = ESP790(cmd_params, auth_type, output);
      break;
#endif  // GLOBAL_FILESYSTEM_FEATURE
    // Get fw version firmare target and fw version
    // eventually set time with pc time
    // output is JSON or plain text according parameter
    //[ESP800]<plain><time=YYYY-MM-DD-HH-MM-SS>
    case 800:
      response = ESP800(cmd_params, auth_type, output);
      break;

#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
    // Get state / Set Enable / Disable Serial Communication
    //[ESP900]<ENABLE/DISABLE>
    case 900:
      response = ESP900(cmd_params, auth_type, output);
      break;
    // Get / Set Serial Baud Rate
    //[ESP901]<BAUD RATE> json=<no> pwd=<admin/user password>
    case 901:
      response = ESP901(cmd_params, auth_type, output);
      break;
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#ifdef BUZZER_DEVICE
    // Get state / Set Enable / Disable buzzer
    //[ESP910]<ENABLE/DISABLE>
    case 910:
      response = ESP910(cmd_params, auth_type, output);
      break;
#endif  // BUZZER_DEVICE
    case 920:
      // Get state / Set state of output message clients
      //[ESP910]<SERIAL / SCREEN / REMOTE_SCREEN/ WEBSOCKET / TELNET /BT /
      // ALL>=<ON/OFF>[pwd=<admin password>]
      response = ESP920(cmd_params, auth_type, output);
      break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    // Get state / Set Enable / Disable Serial Bridge Communication
    //[ESP930]<ENABLE/DISABLE>
    case 930:
      response = ESP930(cmd_params, auth_type, output);
      break;
    // Get / Set Serial Bridge Baud Rate
    //[ESP931]<BAUD RATE> json=<no> pwd=<admin/user password>
    case 931:
      response = ESP931(cmd_params, auth_type, output);
      break;
#endif  // defined(ESP_SERIAL_BRIDGE_OUTPUT)
#if defined(ARDUINO_ARCH_ESP32) &&                             \
    (CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32S2 || \
     CONFIG_IDF_TARGET_ESP32C3)
    case 999:
      // Set quiet boot if strapping pin is High
      //[ESP999]<QUIETBOOT> [pwd=<admin/user password>]
      response = ESP999(cmd_params, auth_type, output);
      break;
#endif  // ARDUINO_ARCH_ESP32
    default:
      output->printERROR("Invalid Command");
      response = false;
  }
  return response;
}

bool Commands::_dispatchSetting(
    bool json, const char *filter, ESP3DSettingIndex index, const char *help,
    const char **optionValues, const char **optionLabels, uint32_t maxsize,
    uint32_t minsize, uint32_t minsize2, uint8_t precision, const char *unit,
    bool needRestart, ESP3DOutput *output, bool isFirst) {
  String tmpstr;
  String value;
  char out_str[255];
  tmpstr.reserve(
      350);  // to save time and avoid several memories allocation delay
  const ESP3DSettingDescription *elementSetting =
      Settings_ESP3D::getSettingPtr(index);
  if (!elementSetting) {
    return false;
  }
  switch (elementSetting->type) {
    case ESP3DSettingType::byte_t:
      value = String(Settings_ESP3D::read_byte(index));
      break;
    case ESP3DSettingType::integer_t:
      value = String(Settings_ESP3D::read_uint32(index));
      break;
    case ESP3DSettingType::ip_t:
      value = Settings_ESP3D::read_IP_String(index);
      break;
    case ESP3DSettingType::float_t:
      // TODO Add float support ?
      value = "Not supported";
      log_esp3d_e("Float not supported");
      return false;
      break;
    case ESP3DSettingType::mask:
      // TODO Add Mask support ?
      value = "Not supported";
      log_esp3d_e("Mask not supported");
      return false;
      break;
    case ESP3DSettingType::bitsfield:
      // TODO Add bitfield support ?
      value = "Not supported";
      log_esp3d_e("Bitsfield not supported");
      return false;
      break;
    case ESP3DSettingType::string_t:
      // String
      if (index == ESP_AP_PASSWORD || index == ESP_STA_PASSWORD ||
#if ESP3D_NOTIFICATIONS_FEATURE
          index == ESP_NOTIFICATION_TOKEN1 ||
          index == ESP_NOTIFICATION_TOKEN2 ||
#endif  // ESP3D_NOTIFICATIONS_FEATURE

          index == ESP_USER_PWD || index == ESP_ADMIN_PWD) {  // hide passwords
                                                              // using  ********
        value = HIDDEN_PASSWORD;
      } else {
        value = Settings_ESP3D::read_string(index);
      }
      break;
    default:
      value = "Not supported";
      log_esp3d_e("Type not supported");
      return false;
      break;
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
    // Limited support already done for above so even not supported we can
    // define it once will be enabled
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
    tmpstr += value;
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
  output->print(tmpstr.c_str());
  return true;
}