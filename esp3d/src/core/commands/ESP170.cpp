/*
 ESP122.cpp - ESP3D command class

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
#if defined(CAMERA_DEVICE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/camera/camera.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "esp_camera.h"

#define COMMAND_ID 170
// Set Camera command value / list all values in JSON/plain
//[ESP170]<plain><label=value> pwd=<admin password>
// label can be:
// light/framesize/quality/contrast/brightness/saturation/gainceiling/colorbar
//              /awb/agc/aec/hmirror/vflip/awb_gain/agc_gain/aec_value/aec2/cw/bpc/wpc
//              /raw_gma/lenc/special_effect/wb_mode/ae_level
void ESP3DCommands::ESP170(int cmd_params_pos, ESP3DMessage* msg) {
  const char* camcmd[] = {
    "framesize",
    "quality",
    "contrast",
    "brightness",
    "saturation",
    "sharpness",
    "special_effect",
    "wb_mode",
    "awb",
    "awb_gain",
    "aec",
    "aec2",
    "ae_level",
    "aec_value",
    "agc",
    "agc_gain",
    "gainceiling",
    "bpc",
    "wpc",
    "raw_gma",
    "lenc",
    "vflip",
    "hmirror",
    "dcw",
    "colorbar",
#if CAM_LED_PIN != -1
    "light",
#endif  // CAM_LED_PIN
  };
  bool hasError = false;
  String ok_msg;
  String error_msg;
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;

#ifdef AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (!esp3d_camera.started()) {
    hasError = true;
    error_msg = "No camera initialized";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    tmpstr = get_clean_param(msg, cmd_params_pos);
    sensor_t* s = esp_camera_sensor_get();
    if (s == nullptr) {
      hasError = true;
      error_msg = "No camera settings available";
      esp3d_log_e("%s", error_msg.c_str());
    } else {
      if (tmpstr.length() == 0) {
        if (json) {
          tmpstr = "{\"cmd\":\"170\",\"status\":\"ok\",\"data\":[";

        } else {
          tmpstr = "Camera:\n";
        }
        msg->type = ESP3DMessageType::head;
        if (!dispatch(msg, tmpstr.c_str())) {
          esp3d_log_e("Error sending response to clients");
          return;
        }
        // now send all settings one by one
        //  framesize
        if (!dispatchIdValue(json, "framesize",
                             String(s->status.framesize).c_str(), target,
                             requestId, true)) {
          return;
        }

        // quality
        if (!dispatchIdValue(json, "quality", String(s->status.quality).c_str(),
                             target, requestId)) {
          return;
        }

        // brightness
        if (!dispatchIdValue(json, "brightness",
                             String(s->status.brightness).c_str(), target,
                             requestId)) {
          return;
        }

        // contrast
        if (!dispatchIdValue(json, "contrast",
                             String(s->status.contrast).c_str(), target,
                             requestId)) {
          return;
        }

        // saturation
        if (!dispatchIdValue(json, "saturation",
                             String(s->status.saturation).c_str(), target,
                             requestId)) {
          return;

          // sharpness
          if (!dispatchIdValue(json, "sharpness",
                               String(s->status.sharpness).c_str(), target,
                               requestId)) {
            return;
          }

          // special_effect
          if (!dispatchIdValue(json, "special_effect",
                               String(s->status.special_effect).c_str(), target,
                               requestId)) {
            return;
          }

          // wb_mode
          if (!dispatchIdValue(json, "wb_mode",
                               String(s->status.wb_mode).c_str(), target,
                               requestId)) {
            return;
          }

          // awb
          if (!dispatchIdValue(json, "awb", String(s->status.awb).c_str(),
                               target, requestId)) {
            return;
          }

          // awb_gain
          if (!dispatchIdValue(json, "awb_gain",
                               String(s->status.awb_gain).c_str(), target,
                               requestId)) {
            return;
          }

          // aec
          if (!dispatchIdValue(json, "aec", String(s->status.aec).c_str(),
                               target, requestId)) {
            return;
          }
          // aec2
          if (!dispatchIdValue(json, "aec2", String(s->status.aec2).c_str(),
                               target, requestId)) {
            return;
          }
          // ae_level
          if (!dispatchIdValue(json, "ae_level",
                               String(s->status.ae_level).c_str(), target,
                               requestId)) {
            return;
          }
          // aec_value
          if (!dispatchIdValue(json, "aec_value",
                               String(s->status.aec_value).c_str(), target,
                               requestId)) {
            return;
          }
          // agc
          if (!dispatchIdValue(json, "agc", String(s->status.agc).c_str(),
                               target, requestId)) {
            return;
          }
          // agc_gain
          if (!dispatchIdValue(json, "agc_gain",
                               String(s->status.agc_gain).c_str(), target,
                               requestId)) {
            return;
          }
          // gainceiling
          if (!dispatchIdValue(json, "gainceiling",
                               String(s->status.gainceiling).c_str(), target,
                               requestId)) {
            return;
          }
          // bpc
          if (!dispatchIdValue(json, "bpc", String(s->status.bpc).c_str(),
                               target, requestId)) {
            return;
          }
          // wpc
          if (!dispatchIdValue(json, "wpc", String(s->status.wpc).c_str(),
                               target, requestId)) {
            return;
          }
          // raw_gma
          if (!dispatchIdValue(json, "raw_gma",
                               String(s->status.raw_gma).c_str(), target,
                               requestId)) {
            return;
          }
          // lenc
          if (!dispatchIdValue(json, "lenc", String(s->status.lenc).c_str(),
                               target, requestId)) {
            return;
          }
          // vflip
          if (!dispatchIdValue(json, "vflip", String(s->status.vflip).c_str(),
                               target, requestId)) {
            return;
          }
          // hmirror
          if (!dispatchIdValue(json, "hmirror",
                               String(s->status.hmirror).c_str(), target,
                               requestId)) {
            return;
          }
          // dcw
          if (!dispatchIdValue(json, "dcw", String(s->status.dcw).c_str(),
                               target, requestId)) {
            return;
          }
          // colorbar
          if (!dispatchIdValue(json, "colorbar",
                               String(s->status.colorbar).c_str(), target,
                               requestId)) {
            return;
          }
#if CAM_LED_PIN != -1
          // light
          if (!dispatchIdValue(
                  json, "light",
                  String(digitalRead(CAM_LED_PIN) == HIGH ? 1 : 0).c_str(),
                  target, requestId)) {
            return;
          }
#endif  // CAM_LED_PIN
        }
        if (json) {
          if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
            esp3d_log_e("Error sending answer to clients");
          }
        } else {
          {
            tmpstr = "ok\n";
            if (!dispatch(tmpstr.c_str(), target, requestId,
                          ESP3DMessageType::tail)) {
              esp3d_log_e("Error sending answer to clients");
            }
          }
        }
        return;
      } else {
        size_t s = sizeof(camcmd) / sizeof(const char*);
        for (size_t i = 0; i < s; i++) {
          String label = camcmd[i];
          label += "=";
          tmpstr = get_param(msg, cmd_params_pos, label.c_str());
          if (tmpstr.length() > 0) {
            int r = esp3d_camera.command(camcmd[i], tmpstr.c_str());
            if (r == -1) {
              hasError = true;
              error_msg = "Unknow command";
              esp3d_log_e("%s", error_msg.c_str());
            } else if (r == 1) {
              hasError = true;
              error_msg = "Invalid value";
              esp3d_log_e("%s", error_msg.c_str());
            } else {
              ok_msg = "ok";
            }
          }
        }
      }
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // CAMERA_DEVICE
