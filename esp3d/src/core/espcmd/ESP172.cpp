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
#if defined (CAMERA_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "esp_camera.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/camera/camera.h"
//Set Camera command value / list all values in JSON/plain
//[ESP172]<plain><label=value> pwd=<admin password>
bool Commands::ESP172(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (!esp3d_camera.started()) {
        output->printERROR("No camera initialized!", 401);
        return false;
    }
    parameter = get_param (cmd_params, "");
    //get
    bool plain = hastag (cmd_params, "plain");
    if ((parameter.length() == 0) || plain) {
        sensor_t * s = esp_camera_sensor_get();
        if (s == nullptr) {
            if (!plain) {
                output->print ("{\"status\":\"error\"}");
            } else {
                output->printERROR("No camera initialized!", 401);
            }
            return false;
        }
        if (!plain) {
            output->print ("{\"status\":\"ok\",");
        }
        //framesize
        if (!plain) {
            output->print ("\"");
        }
        output->print ("framesize");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.framesize);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //quality
        if (!plain) {
            output->print ("\"");
        }
        output->print ("quality");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.quality);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //brightness
        if (!plain) {
            output->print ("\"");
        }
        output->print ("brightness");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.brightness);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //contrast
        if (!plain) {
            output->print ("\"");
        }
        output->print ("contrast");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.contrast);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //saturation
        if (!plain) {
            output->print ("\"");
        }
        output->print ("saturation");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.saturation);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //sharpness
        if (!plain) {
            output->print ("\"");
        }
        output->print ("sharpness");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.sharpness);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //special_effect
        if (!plain) {
            output->print ("\"");
        }
        output->print ("special_effect");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.special_effect);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //wb_mode
        if (!plain) {
            output->print ("\"");
        }
        output->print ("wb_mode");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.wb_mode);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //awb
        if (!plain) {
            output->print ("\"");
        }
        output->print ("awb");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.awb);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //awb_gain
        if (!plain) {
            output->print ("\"");
        }
        output->print ("awb_gain");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.awb_gain);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //aec
        if (!plain) {
            output->print ("\"");
        }
        output->print ("aec");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.aec);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //aec2
        if (!plain) {
            output->print ("\"");
        }
        output->print ("aec2");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.aec2);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //ae_level
        if (!plain) {
            output->print ("\"");
        }
        output->print ("ae_level");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.ae_level);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //aec_value
        if (!plain) {
            output->print ("\"");
        }
        output->print ("aec_value");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.aec_value);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //agc
        if (!plain) {
            output->print ("\"");
        }
        output->print ("agc");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.agc);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //agc_gain
        if (!plain) {
            output->print ("\"");
        }
        output->print ("agc_gain");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.agc_gain);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //gainceiling
        if (!plain) {
            output->print ("\"");
        }
        output->print ("gainceiling");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.gainceiling);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //bpc
        if (!plain) {
            output->print ("\"");
        }
        output->print ("bpc");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.bpc);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //wpc
        if (!plain) {
            output->print ("\"");
        }
        output->print ("wpc");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.wpc);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //raw_gma
        if (!plain) {
            output->print ("\"");
        }
        output->print ("raw_gma");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.raw_gma);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //lenc
        if (!plain) {
            output->print ("\"");
        }
        output->print ("lenc");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.lenc);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //vflip
        if (!plain) {
            output->print ("\"");
        }
        output->print ("vflip");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.vflip);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //hmirror
        if (!plain) {
            output->print ("\"");
        }
        output->print ("hmirror");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.hmirror);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //dcw
        if (!plain) {
            output->print ("\"");
        }
        output->print ("dcw");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.dcw);
        if (!plain) {
            output->print ("\",");
        } else {
            output->printLN("");
        }
        //colorbar
        if (!plain) {
            output->print ("\"");
        }
        output->print ("colorbar");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (s->status.colorbar);
        if (!plain) {
            output->print ("\"");
        } else {
            output->printLN("");
        }
#if CAM_LED_PIN != -1
        //light
        if (!plain) {
            output->print (",\"");
        }
        output->print ("light");
        if (!plain) {
            output->print ("\":\"");
        } else {
            output->print (" : ");
        }
        output->print (digitalRead(CAM_LED_PIN)==HIGH?1:0);
        if (!plain) {
            output->print ("\"");
        } else {
            output->printLN("");
        }
#endif //CAM_LED_PIN 
        if (!plain) {
            output->print ("}");
        }
    } else { //set
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        String label = get_label (cmd_params, "=");
        if (label.length()==0) {
            output->printERROR("Missing command!");
            return false;
        }
        String labels = label+"=";
        String value = get_param (cmd_params,labels.c_str());
        if (value.length()==0) {
            output->printERROR("Invalid value!");
            return false;
        }
        int r = esp3d_camera.command(label.c_str(), value.c_str());
        if (r == -1) {
            output->printERROR("Unknow command!");
            response = false;
        } else if (r == 1) {
            output->printERROR("Invalid value!");
            response = false;
        } else {
            output->printMSG ("ok");
        }
    }
    return response;
}

#endif //CAMERA_DEVICE
