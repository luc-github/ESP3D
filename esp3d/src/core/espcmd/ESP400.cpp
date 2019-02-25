/*
 ESP400.cpp - ESP3D command class

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
#include "../../modules/serial/serial_service.h"
#include "../../modules/authentication/authentication_service.h"
//Get full ESP3D settings
//[ESP400]<pwd=admin>
bool Commands::ESP400(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#endif //AUTHENTICATION_FEATURE
    //Start JSON
    output->printLN ("{\"Settings\":[");
    //1- Baud Rate
    output->print ("{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_BAUD_RATE);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_BAUD_RATE));
    output->print ("\",\"H\":\"Baud Rate\",\"O\":[");
    uint8_t count = 0;
    const long *bl =  serial_service.get_baudratelist(&count);
    for (uint8_t i = 0 ; i < count ; i++) {
        if (i > 0) {
            output->print (",");
        }
        output->printf("{\"%ld\":\"%ld\"}", bl[i], bl[i]);
    }
    output->printLN ("]}");

#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined(BT_FEATURE)
    //2 - Hostname
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_HOSTNAME);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_HOSTNAME));
    output->print ("\",\"H\":\"Hostname\" ,\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_HOSTNAME));
    output->print ("\", \"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_HOSTNAME));
    output->printLN ("\"}");
#endif //WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
    //3 - radio mode
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_RADIO_MODE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_RADIO_MODE));
    output->print ("\",\"H\":\"Radio mode\",\"O\":[{\"None\":\"0\"}");
#ifdef WIFI_FEATURE
    output->print (",{\"STA\":\"1\"},{\"AP\":\"2\"}");
#endif //WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
    output->print (",{\"BT\":\"3\"}");
#endif  //BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
    output->print (",{\"ETH-STA\":\"4\"}");
#endif  //ETH_FEATURE
    output->printLN ("]}");
#ifdef WIFI_FEATURE
    //4 - STA SSID
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_SSID);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_STA_SSID));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_STA_SSID));
    output->print ("\",\"H\":\"Station SSID\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_STA_SSID));
    output->printLN ("\"}");

    //5 - STA password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_PASSWORD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_STA_PASSWORD));
    output->print ("\",\"H\":\"Station Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_STA_PASSWORD));
    output->printLN ("\"}");

    //6-Station Network Mode
    //output->print (",{\"F\":\"network\",\"P\":\"");
    //output->print (ESP_STA_PHY_MODE);
    //output->print ("\",\"T\":\"B\",\"V\":\"");
    //output->print (Settings_ESP3D::read_byte(ESP_STA_PHY_MODE));
    //output->print ("\",\"H\":\"Station Network Mode\",\"O\":[{\"11b\":\"");
    //output->print (WIFI_PHY_MODE_11B);
    //output->print ("\"},{\"11g\":\"");
    //output->print (WIFI_PHY_MODE_11G);
    //output->print ("\"},{\"11n\":\"");
    //output->print (WIFI_PHY_MODE_11N);
    //output->printLN ("\"}]}");
#endif  //WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    // 7 - STA IP mode
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_IP_MODE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_STA_IP_MODE));
    output->printLN ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"0\"},{\"Static\":\"1\"}]}");

    //8-STA static IP
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE));
    output->printLN ("\",\"H\":\"Station Static IP\"}");

    //9-STA static Gateway
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_GATEWAY_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE));
    output->printLN ("\",\"H\":\"Station Static Gateway\"}");

    //10-STA static Mask
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_MASK_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE));
    output->printLN ("\",\"H\":\"Station Static Mask\"}");
#endif  //WIFI_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE)
    //11 - AP SSID
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_SSID);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_AP_SSID));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_SSID));
    output->print ("\",\"H\":\"AP SSID\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_AP_SSID));
    output->printLN ("\"}");

    //12 - AP password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_PASSWORD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
    output->print ("\",\"H\":\"AP Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
    output->printLN ("\"}");

    //13 - AP static IP
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_AP_IP_VALUE));
    output->printLN ("\",\"H\":\"AP Static IP\"}");

    //14 - AP Channel
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_CHANNEL);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_AP_CHANNEL));
    output->print ("\",\"H\":\"AP Channel\",\"O\":[");
    for (uint8_t i = Settings_ESP3D::get_min_byte(ESP_AP_CHANNEL); i <= Settings_ESP3D::get_max_byte(ESP_AP_CHANNEL) ; i++) {
        if (i > 1) {
            output->print (",");
        }
        output->printf("{\"%d\":\"%d\"}", i, i);
    }
    output->printLN ("]}");
#endif  //WIFI_FEATURE
    //15 - AP Network Mode (PHY)
    //output->print (",{\"F\":\"network\",\"P\":\"");
    //output->print (ESP_AP_PHY_MODE);
    //output->print ("\",\"T\":\"B\",\"V\":\"");
    //output->print (Settings_ESP3D::read_byte(ESP_AP_PHY_MODE));
    //output->print ("\",\"H\":\"AP Network Mode\",\"O\":[{\"11b\":\"");
    //output->print (WIFI_PHY_MODE_11B);
    //output->print ("\"},{\"11g\":\"");
    //output->print (WIFI_PHY_MODE_11G);
    //output->printLN ("\"}]}");

    //16-AP Authentication
    //output->print (",{\"F\":\"network\",\"P\":\"");
    //output->print (ESP_AP_AUTH_TYPE);
    //output->print ("\",\"T\":\"B\",\"V\":\"");
    //output->print (Settings_ESP3D::read_byte(ESP_AP_AUTH_TYPE));
    //output->print ("\",\"H\":\"Authentication\",\"O\":[{\"Open\":\"");
    //output->print (AUTH_OPEN);
    //output->print ("\"},{\"WPA\":\"");
    //output->print (AUTH_WPA_PSK);
    //output->print ("\"},{\"WPA2\":\"");
    //output->print (AUTH_WPA2_PSK);
    //output->print ("\"},{\"WPA/WPA2\":\"");
    //output->print (AUTH_WPA_WPA2_PSK);
    //output->printLN ("\"}]}");

    //17-AP SSID visibility
    //output->print (",{\"F\":\"network\",\"P\":\"");
    //output->print (ESP_SSID_VISIBLE);
    //output->print ("\",\"T\":\"B\",\"V\":\"");
    //output->print (Settings_ESP3D::read_byte(ESP_SSID_VISIBLE));
    //output->printLN ("\",\"H\":\"SSID Visible\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");
#ifdef HTTP_FEATURE
    //18-HTTP On
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_HTTP_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_HTTP_ON));
    output->printLN ("\",\"H\":\"Enable HTTP\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //19-HTTP Port
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_HTTP_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_HTTP_PORT));
    output->print ("\",\"H\":\"HTTP Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_HTTP_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_HTTP_PORT));
    output->printLN ("\"}");
#endif //HTTP_FEATURE

#ifdef TELNET_FEATURE
    //20-TELNET On
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TELNET_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TELNET_ON));
    output->printLN ("\",\"H\":\"Enable Telnet\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //21-TELNET Port
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TELNET_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_TELNET_PORT));
    output->print ("\",\"H\":\"HTTP Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_TELNET_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_TELNET_PORT));
    output->printLN ("\"}");
#endif //TELNET

#ifdef AUTHENTICATION_FEATURE
    //22-Admin password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_ADMIN_PWD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_ADMIN_PWD));
    output->print ("\",\"H\":\"Admin Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_ADMIN_PWD));
    output->printLN ("\"}");

    //23-User password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_USER_PWD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_USER_PWD));
    output->print ("\",\"H\":\"User Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_USER_PWD));
    output->printLN ("\"}");
#endif //AUTHENTICATION_FEATURE

    //Target FW
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_TARGET_FW);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TARGET_FW));
    output->print ("\",\"H\":\"Target FW\",\"O\":[{\"Repetier\":\"");
    output->print (REPETIER);
    output->print ("\"},{\"Repetier for Davinci\":\"");
    output->print (REPETIER4DV);
    output->print ("\"},{\"Marlin\":\"");
    output->print (MARLIN);
    output->print ("\"},{\"Marlin Kimbra\":\"");
    output->print (MARLINKIMBRA);
    output->print ("\"},{\"Smoothieware\":\"");
    output->print (SMOOTHIEWARE);
    output->print ("\"},{\"Grbl\":\"");
    output->print (GRBL);
    output->print ("\"},{\"Unknown\":\"");
    output->print (UNKNOWN_FW);
    output->printLN ("\"}]}");

    //Output flag
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_OUTPUT_FLAG);
    output->print ("\",\"T\":\"F\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TARGET_FW));
    output->printf ("\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"%d\"}", ESP_PRINTER_LCD_CLIENT);
#ifdef ESP_OLED_FEATURE
    output->printf (",{\"Oled\":\"%d\"}", ESP_OLED_CLIENT);
#endif //ESP_OLED_FEATURE
    output->printf (",{\"Serial\":\"%d\"}", ESP_SERIAL_CLIENT);
#ifdef WS_DATA_FEATURE
    output->printf (",{\"Web Socket\":\"%d\"}", ESP_WEBSOCKET_CLIENT);
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
    output->printf (",{\"Bluetooth\":\"%d\"}", ESP_BT_CLIENT);
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
    output->printf (",{\"Telnet\":\"%d\"}", ESP_TELNET_CLIENT);
#endif //TELNET_FEATURE
    output->printLN ("]}");
#ifdef SDCARD_FEATURE
    //Direct SD
    output->print(",{\"F\":\"printer\",\"P\":\"");
    output->print(ESP_IS_DIRECT_SD);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_IS_DIRECT_SD));
    output->print("\",\"H\":\"Direct SD access\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");
#endif //SDCARD_FEATURE      

    output->printLN ("]}");

    return true;
}
