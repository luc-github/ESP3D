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
#define COMMANDID   170
//Set Camera command value / list all values in JSON/plain
//[ESP170]<plain><label=value> pwd=<admin password>
//label can be: light/framesize/quality/contrast/brightness/saturation/gainceiling/colorbar
//             /awb/agc/aec/hmirror/vflip/awb_gain/agc_gain/aec_value/aec2/cw/bpc/wpc
//             /raw_gma/lenc/special_effect/wb_mode/ae_level
bool Commands::ESP170(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead

#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        response = format_response(COMMANDID, json, false, "Guest user can't use this command");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if(noError) {
        if (!esp3d_camera.started()) {
            response = format_response(COMMANDID, json, false, "No camera initialized");
            noError = false;
        } else {
            parameter = clean_param(get_param (cmd_params, ""));
            //get
            if (parameter.length() == 0) {
                sensor_t * s = esp_camera_sensor_get();
                if (s == nullptr) {
                    response = format_response(COMMANDID, json, false, "No camera initialized");
                    noError = false;
                } else {
                    String line = "";
                    if (json) {
                        output->print ("{\"cmd\":\"170\",\"status\":\"ok\",\"data\":\"[");
                    }
                    //framesize
                    if (json) {
                        line +="{\"id\":\"framesize\",\"value\":\"";
                    } else {
                        line +="framesize:";
                    }
                    line +=s->status.framesize;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //quality
                    if (json) {
                        line +="{\"id\":\"quality\",\"value\":\"";
                    } else {
                        line +="quality:";
                    }
                    line +=s->status.quality;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //brightness
                    if (json) {
                        line +="{\"id\":\"brightness\",\"value\":\"";
                    } else {
                        line +="brightness:";
                    }
                    line +=s->status.brightness;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //contrast
                    if (json) {
                        line +="{\"id\":\"contrast\",\"value\":\"";
                    } else {
                        line +="contrast:";
                    }
                    line +=s->status.contrast;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //saturation
                    if (json) {
                        line +="{\"id\":\"saturation\",\"value\":\"";
                    } else {
                        line +="saturation:";
                    }
                    line +=s->status.saturation;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //sharpness
                    if (json) {
                        line +="{\"id\":\"sharpness\",\"value\":\"";
                    } else {
                        line +="sharpness:";
                    }
                    line +=s->status.sharpness;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //special_effect
                    if (json) {
                        line +="{\"id\":\"special_effect\",\"value\":\"";
                    } else {
                        line +="special_effect:";
                    }
                    line +=s->status.special_effect;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //wb_mode
                    if (json) {
                        line +="{\"id\":\"wb_mode\",\"value\":\"";
                    } else {
                        line +="wb_mode:";
                    }
                    line +=s->status.wb_mode;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //awb
                    if (json) {
                        line +="{\"id\":\"awb\",\"value\":\"";
                    } else {
                        line +="awb:";
                    }
                    line +=s->status.awb;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //awb_gain
                    if (json) {
                        line +="{\"id\":\"awb_gain\",\"value\":\"";
                    } else {
                        line +="awb_gain:";
                    }
                    line +=s->status.awb_gain;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //aec
                    if (json) {
                        line +="{\"id\":\"aec\",\"value\":\"";
                    } else {
                        line +="aec:";
                    }
                    line +=s->status.aec;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //aec2
                    if (json) {
                        line +="{\"id\":\"aec2\",\"value\":\"";
                    } else {
                        line +="aec2:";
                    }
                    line +=s->status.aec2;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //ae_level
                    if (json) {
                        line +="{\"id\":\"ae_level\",\"value\":\"";
                    } else {
                        line +="ae_level:";
                    }
                    line +=s->status.ae_level;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //aec_value
                    if (json) {
                        line +="{\"id\":\"aec_value\",\"value\":\"";
                    } else {
                        line +="aec_value:";
                    }
                    line +=s->status.aec_value;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //agc
                    if (json) {
                        line +="{\"id\":\"agc\",\"value\":\"";
                    } else {
                        line +="agc:";
                    }
                    line +=s->status.agc;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //agc_gain
                    if (json) {
                        line +="{\"id\":\"agc_gain\",\"value\":\"";
                    } else {
                        line +="agc_gain:";
                    }
                    line +=s->status.agc_gain;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //gainceiling
                    if (json) {
                        line +="{\"id\":\"gainceiling\",\"value\":\"";
                    } else {
                        line +="gainceiling:";
                    }
                    line +=s->status.gainceiling;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //bpc
                    if (json) {
                        line +="{\"id\":\"bpc\",\"value\":\"";
                    } else {
                        line +="bpc:";
                    }
                    line +=s->status.bpc;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //wpc
                    if (json) {
                        line +="{\"id\":\"wpc\",\"value\":\"";
                    } else {
                        line +="wpc:";
                    }
                    line +=s->status.wpc;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //raw_gma
                    if (json) {
                        line +="{\"id\":\"raw_gma\",\"value\":\"";
                    } else {
                        line +="raw_gma:";
                    }
                    line +=s->status.raw_gma;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //lenc
                    if (json) {
                        line +="{\"id\":\"lenc\",\"value\":\"";
                    } else {
                        line +="lenc:";
                    }
                    line +=s->status.lenc;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //vflip
                    if (json) {
                        line +="{\"id\":\"vflip\",\"value\":\"";
                    } else {
                        line +="vflip:";
                    }
                    line +=s->status.vflip;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //hmirror
                    if (json) {
                        line +="{\"id\":\"hmirror\",\"value\":\"";
                    } else {
                        line +="hmirror:";
                    }
                    line +=s->status.hmirror;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //dcw
                    if (json) {
                        line +="{\"id\":\"dcw\",\"value\":\"";
                    } else {
                        line +="dcw:";
                    }
                    line +=s->status.dcw;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //colorbar
                    if (json) {
                        line +="{\"id\":\"colorbar\",\"value\":\"";
                    } else {
                        line +="colorbar:";
                    }
                    line +=s->status.colorbar;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
#if CAM_LED_PIN != -1
                    //light
                    if (json) {
                        line +="{\"id\":\"light\",\"value\":\"";
                    } else {
                        line +="light:";
                    }
                    line +=digitalRead(CAM_LED_PIN)==HIGH?1:0;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
#endif //CAM_LED_PIN 
                    if (json) {
                        output->printLN ("]}");
                    }
                    return true;
                }
            } else { //set
#ifdef AUTHENTICATION_FEATURE
                if (auth_type != LEVEL_ADMIN) {
                    response = format_response(COMMANDID, json, false, "Wrong authentication level");
                    noError = false;
                    errorCode = 401;
                }
#endif //AUTHENTICATION_FEATURE
                if (noError) {
                    String label = get_label (parameter.c_str(), "=");
                    if (label.length()==0) {
                        response = format_response(COMMANDID, json, false, "Missing command");
                        noError = false;
                    } else {
                        String labels = label+"=";
                        String value = get_param (cmd_params,labels.c_str());
                        if (value.length()==0) {
                            response = format_response(COMMANDID, json, false, "Invalid value");
                            noError = false;
                        }
                        if (noError) {
                            int r = esp3d_camera.command(label.c_str(), value.c_str());
                            if (r == -1) {
                                response = format_response(COMMANDID, json, false, "Unknow command");
                                noError = false;
                            } else if (r == 1) {
                                response = format_response(COMMANDID, json, false, "Invalid value");
                                noError = false;
                            } else {
                                response = format_response(COMMANDID, json, true, "ok");
                            }
                        }
                    }
                }
            }
        }
    }
    if (noError) {
        if (json) {
            output->printLN (response.c_str() );
        } else {
            output->printMSG (response.c_str() );
        }
    } else {
        output->printERROR(response.c_str(), errorCode);
    }
    return noError;
}

#endif //CAMERA_DEVICE
