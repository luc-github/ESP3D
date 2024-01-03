/*
 ESP410.cpp - ESP3D command class

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
#if defined(MDNS_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/mDNS/mDNS.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

// Get available ESP3D list
// output is JSON or plain text according parameter
//[ESP4\50]json=<no>
#define COMMAND_ID 450
void ESP3DCommands::ESP450(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;

#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE

  if (!(WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP)) {
    tmpstr = "Network not enabled";
    dispatchAnswer(msg, COMMAND_ID, json, true, tmpstr.c_str());
    return;
  }

  if (json) {
    tmpstr = "{\"cmd\":\"450\",\"status\":\"ok\",\"data\":[";

  } else {
    tmpstr = "Start Scan\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }

  uint16_t count = esp3d_mDNS.servicesCount();
  uint16_t nbListed = 0;
  esp3d_log("Total : %d", count);
  for (uint16_t i = 0; i < count; i++) {
    if (strlen(esp3d_mDNS.answerHostname(i)) == 0) {
      continue;
    }
    // Currently esp3d support only IPV4 and only one address per device

    tmpstr = "";

    if (json) {
      if (nbListed > 0) {
        tmpstr += ",";
      }

      tmpstr += "{\"Hostname\":\"";
    }
    tmpstr += esp3d_mDNS.answerHostname(i);
    if (json) {
      tmpstr += "\",\"IP\":\"";
    } else {
      tmpstr += " (";
    }
    tmpstr += esp3d_mDNS.answerIP(i);
    if (json) {
      tmpstr += "\",\"port\":\"";
      tmpstr += String(esp3d_mDNS.answerPort(i));
      tmpstr += "\",\"TxT\":[";

    } else {
      tmpstr += ":";
      tmpstr += String(esp3d_mDNS.answerPort(i));
      tmpstr += ") ";
    }
    uint16_t nbtxt = esp3d_mDNS.answerTxtCount(i);
    for (uint8_t t = 0; t < nbtxt; t++) {
      if (t != 0) {
        tmpstr += ",";
      }
      if (json) {
        tmpstr += "{\"key\":\"";
      }
      tmpstr += esp3d_mDNS.answerTxtKey(i, t);
      if (json) {
        tmpstr += "\",\"value\":\"";
      } else {
        tmpstr += "=";
      }
      tmpstr += esp3d_mDNS.answerTxt(i, t);
      if (json) {
        tmpstr += "\"}";
      }
    }

    if (json) {
      tmpstr += "]}";
    } else {
      tmpstr += "\n";
    }
    if (!dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::core)) {
      esp3d_log_e("Error sending answer to clients");
    }

    nbListed++;
  }

  // end of list
  if (json) {
    tmpstr = "]}";
  } else {
    tmpstr = "End Scan\n";
  }
  if (!dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::tail)) {
    esp3d_log_e("Error sending answer to clients");
  }
}

#endif  // MDNS_FEATURE
