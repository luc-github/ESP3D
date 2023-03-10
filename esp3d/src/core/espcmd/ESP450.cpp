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
#if defined (MDNS_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/mDNS/mDNS.h"
#include "../../modules/authentication/authentication_service.h"
//Get available ESP3D list
//output is JSON or plain text according parameter
//[ESP4\50]json=<no>
#define COMMANDID   450
bool Commands::ESP450(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    if (noError) {
        parameter = clean_param(get_param (cmd_params, ""));
        if (parameter.length() == 0) {

            uint16_t n = 0;
            if (!json) {
                output->printMSGLine ("Start Scan");
            }

            n = esp3d_mDNS.servicesCount ();
            if (json) {
                output->print ("{\"cmd\":\"450\",\"status\":\"ok\",\"data\":[");
            }
            String line;

            for (uint16_t i = 0; i < n; i++) {
                line = "";
                if (strlen(esp3d_mDNS.answerHostname(i)) == 0) {
                    continue;
                }
                if (i > 0) {
                    if (json) {
                        line+=",";
                    }
                }
                if (json) {
                    line += "{\"Hostname\":\"";
                    line +=ESP3DOutput::encodeString(esp3d_mDNS.answerHostname(i));
                } else {
                    line +=esp3d_mDNS.answerHostname(i);
                }
                if (json) {
                    line +="\",\"IP\":\"";
                } else {
                    line +=" (";
                }
                line += esp3d_mDNS.answerIP(i);
                if (!json) {
                    line +=":";
                }
                if (json) {
                    line +="\",\"port\":\"";
                }
                line += String(esp3d_mDNS.answerPort(i));
                if (json) {
                    line +="\",\"TxT\":[";
                } else {
                    line +=") ";
                }
                uint16_t nbtxt = esp3d_mDNS.answerTxtCount(i);
                for (uint16_t j = 0; j < nbtxt; ++j) {
                    if (j>0) {
                        line += ",";
                    }
                    if (json) {
                        line += "{\"key\":\"";
                    }
                    line+=esp3d_mDNS.answerTxtKey(i, j);
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +="=";
                    }
                    line+=esp3d_mDNS.answerTxt(i, j);
                    if (json) {
                        line +="\"}";
                    }

                }
                if (json) {
                    line +="]}";
                }
                if (json) {
                    output->print (line.c_str());
                } else {
                    output->printMSGLine (line.c_str());
                }
            }



        }

        if (json) {
            output->printLN ("]}");
        } else {
            output->printMSGLine ("End Scan");
        }
        return true;
    } else {
        response = format_response(COMMANDID, json, false, "This command doesn't take parameters");
        noError = false;
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

#endif //MDNS_FEATURE
