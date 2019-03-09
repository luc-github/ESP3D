/*
 ESP420.cpp - ESP3D command class

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
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
#if defined (TIMESTAMP_FEATURE)
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#if defined (DHT_DEVICE)
#include "../../modules/dht/dht.h"
#endif //DHT_DEVICE
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
    output->print ("Chip ID");
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
    output->print ("CPU Frequency");
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
        output->print ("CPU Temperature");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->printf("%s %s",String (Hal::temperature(), 1).c_str(), (output->client() == ESP_HTTP_CLIENT)?"&deg;C":"C");
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
    output->print ("Free memory");
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
    output->print ("Flash Size");
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
    output->print ("Available Size for update");
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
    output->print ("Filesystem type");
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
    output->print ("Filesystem usage");
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
    output->print ("Baud rate");
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
        output->print ("Sleep mode");
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
    output->print ("WiFi");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((WiFi.getMode() == WIFI_OFF)?"Disabled":"Enabled");
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
    output->print ("Ethernet");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((EthConfig::started())?"Enabled":"Disabled");
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
    output->print("Bluetooth");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print ((bt_service.started())?"Enabled":"Disabled");
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
    output->print ("Hostname");
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
#if defined (BLUETOOTH_FEATURE)
    if (bt_service.started()) {
        //BT mode
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Current BT mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ("Serial (");
        output->print (BTService::macAddress());
        output->print(")");
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
        output->print ((bt_service.isConnected())?"Connected":"Disconnected");
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
        output->print ("Current Ethernet mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ("Client(");
        output->print (ETH.macAddress().c_str());
        output->printLN(")");
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //Ethernet cable
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("Ethernet cable");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ((ETH.linkUp())?"Connected":"Disconnected");
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
        output->print ("IP Mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        output->print ((NetConfig::isIPModeDHCP(ESP_ETH_STA))?"DHCP":"Static");
        if (!plain) {
            output->print ("\"}");
        } else {
            output->printLN("");
        }
        //IP value
        if (!plain) {
            output->print (",{\"id\":\"");
        }
        output->print ("IP");
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
        output->print ("Gateway");
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
        output->print ("Mask");
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
        output->print ("Current WiFi Mode");
        if (!plain) {
            output->print ("\",\"value\":\"");
        } else {
            output->print (": ");
        }
        if (WiFi.getMode() == WIFI_STA) {
            output->print ("STA (");
            output->print ( WiFi.macAddress().c_str());
            output->print (")");
        } else if (WiFi.getMode() == WIFI_AP) {
            output->print ("AP (");
            output->print (WiFi.softAPmacAddress().c_str());
            output->print (")");
        } else if (WiFi.getMode() == WIFI_AP_STA) { //we should not be in this state but just in case ....
            output->print ("Mixed");
            output->printLN("");
            output->print ("STA (");
            output->print (WiFi.macAddress().c_str());
            output->print (")");
            output->printLN("");
            output->print ("AP (");
            output->print (WiFi.softAPmacAddress().c_str());
            output->print (")");
        } else {
            output->print ("???");
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
            output->print ("Connected to");
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
                output->print ("Signal");
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
                output->print ("Phy Mode");
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
                output->print ("Channel");
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
                output->print ("IP Mode");
                if (!plain) {
                    output->print ("\",\"value\":\"");
                } else {
                    output->print (": ");
                }
                output->print ((NetConfig::isIPModeDHCP(ESP_WIFI_STA))?"DHCP":"Static");
                if (!plain) {
                    output->print ("\"}");
                } else {
                    output->printLN("");
                }
                //IP value
                if (!plain) {
                    output->print (",{\"id\":\"");
                }
                output->print ("IP");
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
                output->print ("Gateway");
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
                output->print ("Mask");
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
            output->print ("Disabled Mode");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ("AP (");
            output->print (WiFi.softAPmacAddress().c_str());
            output->print (")");
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
            output->print ("Visible");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ((WiFiConfig::is_AP_visible()) ? "Yes" : "No");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //AP Authentication
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("Authentication");
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
            output->print ((NetConfig::isDHCPServer (ESP_WIFI_AP))?"Started":"Stopped");
            if (!plain) {
                output->print ("\"}");
            } else {
                output->printLN("");
            }
            //IP Value
            if (!plain) {
                output->print (",{\"id\":\"");
            }
            output->print ("IP");
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
            output->print ("Gateway");
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
            output->print ("Mask");
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
            output->print ("Connected clients");
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
                output->printf ("Client %d",i);
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
            output->print ("Disabled Mode");
            if (!plain) {
                output->print ("\",\"value\":\"");
            } else {
                output->print (": ");
            }
            output->print ("STA (");
            output->print (WiFi.macAddress().c_str());
            output->print (")");
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
    output->print ("Time client");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (timeserver.started()?"Started":"Disabled");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //TIMESTAMP_FEATURE
#if defined (DHT_DEVICE)
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("DHT sensor");
    if (!plain) {
        output->print ("\",\"value\":\"");
    } else {
        output->print (": ");
    }
    output->print (esp3d_DHT.started()?"Enabled":"Disabled");
    output->print ("(");
    output->print (esp3d_DHT.GetModelString());
    output->print (")");
    if (!plain) {
        output->print ("\"}");
    } else {
        output->printLN("");
    }
#endif //DHT_DEVICE
    //FW version
    if (!plain) {
        output->print (",{\"id\":\"");
    }
    output->print ("FW version");
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
    output->print ("FW architecture");
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
