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
#include "../../modules/serial/serial_service.h"
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
#if defined (TIMESTAMP_FEATURE)
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#if defined (DHT_DEVICE)
#include "../../modules/dht/dht.h"
#endif //DHT_DEVICE
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

//Get ESP current status
//output is JSON or plain text according parameter
//[ESP420]<plain>
bool Commands::ESP420(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    bool plain = hastag(cmd_params,"plain");
    //TODO add plain / JSON support
    if (!plain) {
        output->print ("{\"Status\":[");
    }
    //Chip ID
    if (!plain) {
        output->print ("{\"id\":\"");
    }
    output->print ("chip id");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->printf("%d",Hal::getChipID());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    //CPU freq
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("CPU Freq");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->printf("%d Mhz",ESP.getCpuFreqMHz());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    //CPU temp
    if (Hal::has_temperature_sensor()) {
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("CPU Temp");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf("%s C",String (Hal::temperature(), 1).c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
    //Free Memory
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("free mem");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print(ESP_FileSystem::formatBytes (ESP.getFreeHeap()).c_str());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    //SDK version
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("SDK");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->printf("%s", ESP.getSdkVersion());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    //Flash size
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("flash size");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print(ESP_FileSystem::formatBytes (ESP.getFlashChipSize()).c_str());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }

#if (defined (WIFI_FEATURE) || defined (ETH_FEATURE)) && (defined(OTA_FEATURE) || defined(WEB_UPDATE_FEATURE))
    //update space
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("size for update");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print(ESP_FileSystem::formatBytes (ESP_FileSystem::max_update_size()).c_str());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(FILESYSTEM_FEATURE)
    //FileSystem type
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("FS type");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (ESP_FileSystem::FilesystemName());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    //FileSystem capacity
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("FS usage");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (ESP_FileSystem::formatBytes (ESP_FileSystem::usedBytes()).c_str());
    output->print ("/");
    output->print (ESP_FileSystem::formatBytes (ESP_FileSystem::totalBytes()).c_str());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //FILESYSTEM_FEATURE
    //baud rate
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("baud");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->printf ("%ld", serial_service.baudRate());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#if defined (WIFI_FEATURE)
    if (WiFi.getMode() != WIFI_OFF) {
        //sleep mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("sleep mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (WiFiConfig::getSleepModeString ());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //WIFI_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    //Wifi enabled
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("wifi");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((WiFi.getMode() == WIFI_OFF)?"OFF":"ON");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#if defined (ETH_FEATURE)
    //Ethernet enabled
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("ethernet");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((EthConfig::started())?"ON":"OFF");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
    //BT enabled
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print("bt");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((bt_service.started())?"ONN":"OFF");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //BLUETOOTH_FEATURE
    //Hostname
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("hostname");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (NetConfig::hostname());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#if defined (HTTP_FEATURE)
    if (HTTP_Server::started()) {
        //http port
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("HTTP port");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%d",HTTP_Server::port());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //HTTP_FEATURE
#if defined (TELNET_FEATURE)
    if (telnet_server.started()) {
        //telnet port
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Telnet port");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%d",telnet_server.port());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
    if (telnet_server.isConnected()) {
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Telnet Client");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%s",telnet_server.clientIPAddress());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //TELNET_FEATURE
#if defined (FTP_FEATURE)
    if (ftp_server.started()) {
        //ftp ports
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Ftp ports");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%d, %d, %d",ftp_server.ctrlport(),ftp_server.dataactiveport(), ftp_server.datapassiveport());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
    if (ftp_server.isConnected()) {
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Ftp Client");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%s",ftp_server.clientIPAddress());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //FTP_FEATURE
#if defined (WS_DATA_FEATURE)
    if (websocket_data_server.started()) {
        //websocket port
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Websocket port");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%d",websocket_data_server.port());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //WS_DATA_FEATURE
#if defined (CAMERA_DEVICE)
    if (esp3d_camera.started()) {
        //camera name
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("camera name");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%s(%d)",esp3d_camera.GetModelString(),esp3d_camera.GetModel());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //camera port
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("camera ports");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf ("%d - %d",esp3d_camera.port(), esp3d_camera.port()+1);
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //CAMERA_DEVICE

#if defined (BLUETOOTH_FEATURE)
    if (bt_service.started()) {
        //BT mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("bt");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (BTService::macAddress());

        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //BT status
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("BT Status");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ((bt_service.isConnected())?"connected":"disconnected");
        if (bt_service.isConnected()) {
            output->print (" (client: ");
            output->print (BTService::clientmacAddress());
            output->print (")");
        }
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //BLUETOOTH_FEATURE
#if defined (ETH_FEATURE)
    if (EthConfig::started()) {
        //Ethernet mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("ethernet");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (ETH.macAddress().c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //Ethernet cable
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("cable");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ((ETH.linkUp())?"connected":"disconnected");
        if(ETH.linkUp()) {
            output->print (" (");
            output->print (ETH.linkSpeed());
            output->printLN("Mbps)");
        }
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //IP mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("ip mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ((NetConfig::isIPModeDHCP(ESP_ETH_STA))?"dhcp":"static");
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //IP value
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("ip");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (ETH.localIP().toString().c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //GW value
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("gw");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (ETH.gatewayIP().toString().c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //Mask value
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("msk");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (ETH.subnetMask().toString().c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //DNS value
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("DNS");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print (ETH.dnsIP().toString().c_str());
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
    }
#endif //ETH_FEATURE
#if defined (WIFI_FEATURE)
    if (WiFi.getMode() != WIFI_OFF) {
        //WiFi Mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        if (WiFi.getMode() == WIFI_STA) {
            output->print ("sta");
        } else if (WiFi.getMode() == WIFI_AP) {
            output->print ("ap");
        } else if (WiFi.getMode() == WIFI_AP_STA) { //we should not be in this state but just in case ....
            output->print ("mixed");
        } else {
            output->print ("unknown");
        }

        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ("ON");
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }

        //WiFi mac
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("mac");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        if (WiFi.getMode() == WIFI_STA) {
            output->print ( WiFi.macAddress().c_str());
        } else if (WiFi.getMode() == WIFI_AP) {
            output->print (WiFi.softAPmacAddress().c_str());
        } else if (WiFi.getMode() == WIFI_AP_STA) { //we should not be in this state but just in case ....
            output->print (WiFi.macAddress().c_str());
            output->print ("/");
            output->print (WiFi.softAPmacAddress().c_str());
        } else {
            output->print ("unknown");
        }
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }

        //WiFi Station
        if (WiFi.getMode() == WIFI_STA) {
            //Connected to SSID
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("SSID");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            if (WiFi.isConnected()) {
                output->print (WiFi.SSID().c_str());
            }
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            if (WiFi.isConnected()) { //in case query come from serial
                //Signal strength
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("signal");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->printf ("%d %%",WiFiConfig::getSignal(WiFi.RSSI()));
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //Phy Mode
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("phy mode");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (WiFiConfig::getPHYModeString (WIFI_STA));
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //Channel
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("channel");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->printf ("%d",WiFi.channel());
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //IP Mode
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("ip mode");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print ((NetConfig::isIPModeDHCP(ESP_WIFI_STA))?"dhcp":"static");
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //IP value
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("ip");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (WiFi.localIP().toString().c_str());
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //Gateway value
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("gw");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (WiFi.gatewayIP().toString().c_str());
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //Mask value
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("msk");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (WiFi.subnetMask().toString().c_str());
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //DNS value
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("DNS");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (WiFi.dnsIP().toString().c_str());
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
            }
            //Disabled Mode
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("ap");

            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ("OFF");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //Disabled Mode
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("mac");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFi.softAPmacAddress().c_str());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
        } else if (WiFi.getMode() == WIFI_AP) {
            //AP SSID
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("SSID");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFiConfig::AP_SSID());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //AP Visibility
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("visible");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ((WiFiConfig::is_AP_visible()) ? "yes" : "no");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //AP Authentication
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("authentication");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFiConfig::AP_Auth_String());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //DHCP Server
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("DHCP Server");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ((NetConfig::isDHCPServer (ESP_WIFI_AP))?"ON":"OFF");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //IP Value
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("ip");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFi.softAPIP().toString());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //Gateway Value
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("gw");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFiConfig::AP_Gateway_String());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //Mask Value
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("msk");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFiConfig::AP_Mask_String());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //Connected clients
            const char * entry = NULL;
            uint8_t nb = 0;
            entry = WiFiConfig::getConnectedSTA(&nb, true);
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("clients");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (nb);
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            for (uint8_t i = 0; i < nb; i++) {
                //Client
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->printf ("# %d",i);
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print (entry);
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //get next
                entry = WiFiConfig::getConnectedSTA();
            }
            //Disabled Mode
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("sta");

            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ("OFF");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //Disabled Mode
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("mac");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print (WiFi.macAddress().c_str());
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }

        }
    }
#endif //WIFI_FEATURE
#endif //WIFI_FEATURE || ETH FEATURE
#if defined (TIMESTAMP_FEATURE)
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("i-time");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (timeserver.started()?"ON":"OFF");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //TIMESTAMP_FEATURE
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("serial");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (serial_service.started()?"ON":"OFF");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#if defined (NOTIFICATION_FEATURE)
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("notification");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (notificationsservice.started()?"ON":"OFF");
    if (notificationsservice.started()) {
        output->print ("(");
        output->print (notificationsservice.getTypeString());
        output->print (")");
    }
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //NOTIFICATION_FEATURE
#ifdef SD_DEVICE
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("sd");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((Settings_ESP3D::GetSDDevice() == ESP_DIRECT_SD)?"direct":(Settings_ESP3D::GetSDDevice() == ESP_SHARED_SD)?"shared":"none");
    output->print ("(");
    output->print (ESP_SD::FilesystemName());
    output->print (")");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //SD_DEVICE
#if defined (DHT_DEVICE)
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("dht");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (esp3d_DHT.started()?"ON":"OFF");
    output->print ("(");
    output->print (esp3d_DHT.GetModelString());
    output->print (")");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //DHT_DEVICE
#if defined (BUZZER_DEVICE)
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("buzzer");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (esp3d_buzzer.started()?"ON":"OFF");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //BUZZER_DEVICE
#if defined (ESP_DEBUG_FEATURE)
    //debug
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("debug");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL0
    output->print ("Serial");
#endif //DEBUG_OUTPUT_SERIAL0
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL1
    output->print ("Serial1");
#endif //DEBUG_OUTPUT_SERIAL1
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2
    output->print ("Serial2");
#endif //DEBUG_OUTPUT_SERIAL2   
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_TELNET
    output->printf ("Telnet(%d)", DEBUG_ESP3D_OUTPUT_PORT);
#endif //DEBUG_OUTPUT_TELNET    
#if ESP_DEBUG_FEATURE == DEBUG_OUTPUT_WEBSOCKET
    output->printf ("Websocket(%d)", DEBUG_ESP3D_OUTPUT_PORT);
#endif //DEBUG_OUTPUT_WEBSOCKET         
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //ESP_DEBUG_FEATURE
    //FW version
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("FW ver");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (FW_VERSION);
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    //FW architecture
    output->print ("FW arch");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (Settings_ESP3D::TargetBoard());
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }

    if (!plain) {
        output->print ("]}");
        output->printLN ("");
    }
    return response;
}
