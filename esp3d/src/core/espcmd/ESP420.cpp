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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/serial/serial_service.h"
#endif // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#ifdef FILESYSTEM_FEATURE
#include "../../modules/filesystem/esp_filesystem.h"
#endif //FILESYSTEM_FEATURE
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE) ||defined(BLUETOOTH_FEATURE)
#include "../../modules/network/netconfig.h"
#if defined (WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
#include "../../modules/ethernet/ethconfig.h"
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../../modules/bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#endif //WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef HTTP_FEATURE
#include "../../modules/http/http_server.h"
#endif //HTTP_FEATURE
#ifdef TELNET_FEATURE
#include "../../modules/telnet/telnet_server.h"
#endif //TELNET_FEATURE
#ifdef FTP_FEATURE
#include "../../modules/ftp/FtpServer.h"
#endif //FTP_FEATURE
#ifdef WS_DATA_FEATURE
#include "../../modules/websocket/websocket_server.h"
#endif //WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
#include "../../modules/webdav/webdav_server.h"
#endif //WEBDAV_FEATURE
#if defined (TIMESTAMP_FEATURE)
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#if defined (SENSOR_DEVICE)
#include "../../modules/sensor/sensor.h"
#endif //SENSOR_DEVICE
#ifdef NOTIFICATION_FEATURE
#include "../../modules/notifications/notifications_service.h"
#endif //NOTIFICATION_FEATURE
#ifdef BUZZER_DEVICE
#include "../../modules/buzzer/buzzer.h"
#endif //BUZZER_DEVICE
#ifdef CAMERA_DEVICE
#include "../../modules/camera/camera.h"
#endif //CAMERA_DEVICE
#ifdef SD_DEVICE
#include "../../modules/filesystem/esp_sd.h"
#endif //SD_DEVICE
#if defined (DISPLAY_DEVICE)
#include "../../modules/display/display.h"
#endif //DISPLAY_DEVICE
#define COMMANDID   420

//Get ESP current status
//output is JSON or plain text according parameter
//[ESP420]json=<no>
bool Commands::ESP420(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
            String line = "";
            if(json) {
                line = "{\"cmd\":\"420\",\"status\":\"ok\",\"data\":[";
            }
            //Chip ID
            if (json) {
                line += "{\"id\":\"";
            }
            line +="chip id";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=Hal::getChipID();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //CPU freq
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="CPU Freq";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=ESP.getCpuFreqMHz();
            line +="Mhz";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //CPU temp
            if (Hal::has_temperature_sensor()) {
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="CPU Temp";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=String(Hal::temperature(), 1);
                line +="C";
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
            //Free Memory
            if (json) {
                line +=",{\"id\":\"";
            }
            line+="free mem";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
#ifdef FILESYSTEM_FEATURE
            line+=ESP_FileSystem::formatBytes (ESP.getFreeHeap()).c_str();
#else
            line+=ESP.getFreeHeap();
#endif//FILESYSTEM_FEATURE

#ifdef ARDUINO_ARCH_ESP32
#ifdef BOARD_HAS_PSRAM
            line+=" - PSRAM:";
            line+=ESP_FileSystem::formatBytes (ESP.getFreePsram());

#endif //BOARD_HAS_PSRAM
#endif //ARDUINO_ARCH_ESP32
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //SDK version
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="SDK";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line+= ESP.getSdkVersion();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //Flash size
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="flash size";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
#ifdef FILESYSTEM_FEATURE
            line+=ESP_FileSystem::formatBytes (ESP.getFlashChipSize());
#else
            line+=ESP.getFlashChipSize();
#endif//FILESYSTEM_FEATURE
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";

#if (defined (WIFI_FEATURE) || defined (ETH_FEATURE)) && (defined(OTA_FEATURE) || defined(WEB_UPDATE_FEATURE))
            //update space
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="size for update";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line+=ESP_FileSystem::formatBytes (ESP_FileSystem::max_update_size());
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(FILESYSTEM_FEATURE)
            //FileSystem type
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="FS type";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=ESP_FileSystem::FilesystemName();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //FileSystem capacity
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="FS usage";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=ESP_FileSystem::formatBytes (ESP_FileSystem::usedBytes());
            line +="/";
            line +=ESP_FileSystem::formatBytes (ESP_FileSystem::totalBytes());
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //FILESYSTEM_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            //baud rate
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="baud";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line+=serial_service.baudRate();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined (WIFI_FEATURE)
            if (WiFi.getMode() != WIFI_OFF) {
                //sleep mode
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="sleep mode";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=WiFiConfig::getSleepModeString ();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
#endif //WIFI_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
            //Wifi enabled
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="wifi";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=(WiFi.getMode() == WIFI_OFF)?"OFF":"ON";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#if defined (ETH_FEATURE)
            //Ethernet enabled
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="ethernet";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=(EthConfig::started())?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
            //BT enabled
            if (json) {
                line +=",{\"id\":\"";
            }
            line+="bt";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=(bt_service.started())?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //BLUETOOTH_FEATURE
            //Hostname
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="hostname";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            if (json) {
                line +=ESP3DOutput::encodeString(NetConfig::hostname());
            } else {
                line +=NetConfig::hostname();
            }
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#if defined (HTTP_FEATURE)
            if (HTTP_Server::started()) {
                //http port
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="HTTP port";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+=HTTP_Server::port();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
#endif //HTTP_FEATURE
#if defined (TELNET_FEATURE)
            if (telnet_server.started()) {
                //telnet port
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="Telnet port";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+=telnet_server.port();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                if (telnet_server.isConnected()) {
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="Telnet Client";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line+=telnet_server.clientIPAddress();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                }
            }
#endif //TELNET_FEATURE
#if defined (WEBDAV_FEATURE)
            if (webdav_server.started()) {
                //WebDav port
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="WebDav port";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+=webdav_server.port();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                if (webdav_server.isConnected()) {
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="WebDav Client";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line+=webdav_server.clientIPAddress();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                }
            }
#endif //WEBDAV_FEATURE
#if defined (FTP_FEATURE)
            if (ftp_server.started()) {
                //ftp ports
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="Ftp ports";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+=String(ftp_server.ctrlport())+","+String(ftp_server.dataactiveport())+","+String(ftp_server.datapassiveport());
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";

                if (ftp_server.isConnected()) {
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="Ftp Client";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line+=ftp_server.clientIPAddress();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                }
            }
#endif //FTP_FEATURE
#if defined (WS_DATA_FEATURE)
            if (websocket_data_server.started()) {
                //websocket port
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="Websocket port";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+=websocket_data_server.port();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
#endif //WS_DATA_FEATURE
#if defined (CAMERA_DEVICE)
            if (esp3d_camera.started()) {
                //camera name
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="camera name";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line+= esp3d_camera.GetModelString();
                line+= "(" ;
                line+= esp3d_camera.GetModel();
                line+=")";
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }



#endif //CAMERA_DEVICE
#if defined (DISPLAY_DEVICE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="display";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line+=esp3d_display.getModelString();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //DISPLAY_DEVICE
#if defined (BLUETOOTH_FEATURE)
            if (bt_service.started()) {
                //BT mode
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="bt";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=BTService::macAddress();

                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //BT status
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="BT Status";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=(bt_service.isConnected())?"connected":"disconnected";
                if (bt_service.isConnected()) {
                    line +=" (client: ";
                    line +=BTService::clientmacAddress();
                    line +=")";
                }
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
#endif //BLUETOOTH_FEATURE
#if defined (ETH_FEATURE)
            if (EthConfig::started()) {
                //Ethernet mode
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="ethernet";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=ETH.macAddress();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //Ethernet cable
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="cable";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=(ETH.linkUp())?"connected":"disconnected";
                if(ETH.linkUp()) {
                    line +=" (";
                    line +=ETH.linkSpeed();
                    line+="Mbps)";
                }
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //IP mode
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="ip mode";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=(NetConfig::isIPModeDHCP(ESP_ETH_STA))?"dhcp":"static";
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //IP value
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="ip";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=ETH.localIP().toString();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //GW value
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="gw";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=ETH.gatewayIP().toString();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //Mask value
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="msk";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=ETH.subnetMask().toString();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
                //DNS value
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="DNS";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +=ETH.dnsIP().toString();
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
#endif //ETH_FEATURE
#if defined (WIFI_FEATURE)
            if (WiFi.getMode() != WIFI_OFF) {
                //WiFi Mode
                if (json) {
                    line +=",{\"id\":\"";
                }
                if (WiFi.getMode() == WIFI_STA) {
                    line +="sta";
                } else if (WiFi.getMode() == WIFI_AP) {
                    line +="ap";
                } else if (WiFi.getMode() == WIFI_AP_STA) { //we should not be in this state but just in case ....
                    line +="mixed";
                } else {
                    line +="unknown";
                }

                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +="ON";
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";

                //WiFi mac
                if (json) {
                    line +=",{\"id\":\"";
                }
                line +="mac";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                if (WiFi.getMode() == WIFI_STA) {
                    line += WiFi.macAddress();
                } else if (WiFi.getMode() == WIFI_AP) {
                    line +=WiFi.softAPmacAddress();
                } else if (WiFi.getMode() == WIFI_AP_STA) { //we should not be in this state but just in case ....
                    line +=WiFi.macAddress();
                    line +="/";
                    line +=WiFi.softAPmacAddress();
                } else {
                    line +="unknown";
                }
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";

                //WiFi Station
                if (WiFi.getMode() == WIFI_STA) {
                    //Connected to SSID
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="SSID";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    if (WiFi.isConnected()) {
                        if (json) {
                            line +=ESP3DOutput::encodeString(WiFi.SSID().c_str());
                        } else {
                            line +=WiFi.SSID();
                        }
                    }
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    if (WiFi.isConnected()) { //in case query come from serial
                        //Signal strength
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="signal";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line+=WiFiConfig::getSignal(WiFi.RSSI(), false);
                        line+="%";
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //Phy Mode
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="phy mode";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=WiFiConfig::getPHYModeString (WIFI_STA);
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //Channel
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="channel";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line+=WiFi.channel();
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //IP Mode
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="ip mode";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=(NetConfig::isIPModeDHCP(ESP_WIFI_STA))?"dhcp":"static";
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //IP value
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="ip";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=WiFi.localIP().toString();
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //Gateway value
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="gw";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=WiFi.gatewayIP().toString();
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //Mask value
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="msk";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=WiFi.subnetMask().toString();
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //DNS value
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line +="DNS";
                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=WiFi.dnsIP().toString();
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                    }
                    //Disabled Mode
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="ap";

                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +="OFF";
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //Disabled Mode
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="mac";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFi.softAPmacAddress();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                } else if (WiFi.getMode() == WIFI_AP) {
                    //AP SSID
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="SSID";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    if (json) {
                        line +=ESP3DOutput::encodeString(WiFiConfig::AP_SSID());
                    } else {
                        line +=WiFiConfig::AP_SSID();
                    }
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //AP Visibility
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="visible";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=(WiFiConfig::is_AP_visible()) ? "yes" : "no";
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //AP Authentication
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="authentication";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFiConfig::AP_Auth_String();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //DHCP Server
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="DHCP Server";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=(NetConfig::isDHCPServer (ESP_WIFI_AP))?"ON":"OFF";
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //IP Value
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="ip";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFi.softAPIP().toString();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //Gateway Value
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="gw";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFiConfig::AP_Gateway_String();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //Mask Value
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="msk";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFiConfig::AP_Mask_String();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //Connected clients
                    const char * entry = NULL;
                    uint8_t nb = 0;
                    entry = WiFiConfig::getConnectedSTA(&nb, true);
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="clients";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=nb;
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    for (uint8_t i = 0; i < nb; i++) {
                        //Client
                        if (json) {
                            line +=",{\"id\":\"";
                        }
                        line+="# "+String(i);

                        if (json) {
                            line +="\",\"value\":\"";
                        } else {
                            line +=": ";
                        }
                        line +=entry;
                        if (json) {
                            line +="\"}";
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                        line="";
                        //get next
                        entry = WiFiConfig::getConnectedSTA();
                    }
                    //Disabled Mode
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="sta";

                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +="OFF";
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                    //Disabled Mode
                    if (json) {
                        line +=",{\"id\":\"";
                    }
                    line +="mac";
                    if (json) {
                        line +="\",\"value\":\"";
                    } else {
                        line +=": ";
                    }
                    line +=WiFi.macAddress();
                    if (json) {
                        line +="\"}";
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine(line.c_str());
                    }
                    line="";
                }
            }
#endif //WIFI_FEATURE
#endif //WIFI_FEATURE || ETH FEATURE
#if defined (TIMESTAMP_FEATURE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="i-time";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=timeserver.started()?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //TIMESTAMP_FEATURE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="serial";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=serial_service.started()?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //COMMUNICATION_PROTOCOL
#if defined (AUTHENTICATION_FEATURE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="authentication";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +="ON";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //AUTHENTICATION_FEATURE
#if defined (HAS_SERIAL_DISPLAY)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="M117";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +="ON";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //HAS_SERIAL_DISPLAY
#if defined (NOTIFICATION_FEATURE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="notification";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=notificationsservice.started()?"ON":"OFF";
            if (notificationsservice.started()) {
                line +="(";
                line +=notificationsservice.getTypeString();
                line +=")";
            }
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //NOTIFICATION_FEATURE
#ifdef SD_DEVICE
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="sd";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=(Settings_ESP3D::GetSDDevice() == ESP_DIRECT_SD)?"direct":(Settings_ESP3D::GetSDDevice() == ESP_SHARED_SD)?"shared":"none";
            line +="(";
            line +=ESP_SD::FilesystemName();
            line +=")";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#ifdef SD_UPDATE_FEATURE
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="SD updater";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=Settings_ESP3D::read_byte (ESP_SD_CHECK_UPDATE_AT_BOOT)!=0?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //SD_UPDATE_FEATURE

#endif //SD_DEVICE
#if defined (SENSOR_DEVICE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="sensor";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=esp3d_sensor.started()?"ON":"OFF";
            line +="(";
            line +=esp3d_sensor.GetModelString();
            line +=")";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //SENSOR_DEVICE
#if defined (BUZZER_DEVICE)
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="buzzer";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=esp3d_buzzer.started()?"ON":"OFF";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //BUZZER_DEVICE
#if defined (ESP_DEBUG_FEATURE)
            //debug
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="debug";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0
            line +="Serial";
#endif //DEBUG_OUTPUT_SERIAL0
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1
            line +="Serial1";
#endif //DEBUG_OUTPUT_SERIAL1
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2
            line +="Serial2";
#endif //DEBUG_OUTPUT_SERIAL2   
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET
            line+="Telnet("+String( DEBUG_ESP3D_OUTPUT_PORT) +")";
#endif //DEBUG_OUTPUT_TELNET    
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
            line+="Websocket("+ String(DEBUG_ESP3D_OUTPUT_PORT)+")";
#endif //DEBUG_OUTPUT_WEBSOCKET         
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //ESP_DEBUG_FEATURE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
//Target Firmware
            if (json) {
                line +=",{\"id\":\"serial";
            } else {
                line +="Serial";
            }
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +="MKS";
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //COMMUNICATION_PROTOCOL
            //Target Firmware
            if (json) {
                line +=",{\"id\":\"targetfw";
            } else {
                line +="Target Fw";
            }
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=Settings_ESP3D::GetFirmwareTargetShortName();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //FW version
            if (json) {
                line +=",{\"id\":\"";
            }
            line +="FW ver";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
#if defined (SHORT_BUILD_VERSION)
            line+=SHORT_BUILD_VERSION "-";
#endif //SHORT_BUILD_VERSION
            line +=FW_VERSION;
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            //FW architecture
            if (json) {
                line +=",{\"id\":\"";
            }

            line +="FW arch";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=Settings_ESP3D::TargetBoard();
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            if (json) {
                output->printLN ("]}");
            }
            return true;
        } else {
            response = format_response(COMMANDID, json, false, "This command doesn't take parameters");
            noError = false;
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
