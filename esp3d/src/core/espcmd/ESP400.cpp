/*
 ESP400.cpp - ESP3D command class

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
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    (void)cmd_params;
    //Start JSON
    output->print ("{\"Settings\":[");
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
    output->print ("]}");

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
    output->print ("\"}");
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
    output->print ("]}");
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
    output->print ("\"}");

    //5 - STA password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_PASSWORD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_STA_PASSWORD));
    output->print ("\",\"H\":\"Station Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_STA_PASSWORD));
    output->print ("\"}");

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
    //output->print ("\"}]}");
#endif  //WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    // 7 - STA IP mode
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_IP_MODE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_STA_IP_MODE));
    output->print ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"0\"},{\"Static\":\"1\"}]}");

    //8-STA static IP
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE));
    output->print ("\",\"H\":\"Station Static IP\"}");

    //9-STA static Gateway
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_GATEWAY_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE));
    output->print ("\",\"H\":\"Station Static Gateway\"}");

    //10-STA static Mask
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_STA_MASK_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE));
    output->print ("\",\"H\":\"Station Static Mask\"}");
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
    output->print ("\"}");

    //12 - AP password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_PASSWORD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
    output->print ("\",\"H\":\"AP Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
    output->print ("\"}");

    //13 - AP static IP
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AP_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_AP_IP_VALUE));
    output->print ("\",\"H\":\"AP Static IP\"}");

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
    output->print ("]}");
#endif  //WIFI_FEATURE

#ifdef HTTP_FEATURE
    //18-HTTP On
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_HTTP_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_HTTP_ON));
    output->print ("\",\"H\":\"Enable HTTP\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //19-HTTP Port
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_HTTP_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_HTTP_PORT));
    output->print ("\",\"H\":\"HTTP Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_HTTP_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_HTTP_PORT));
    output->print ("\"}");
#endif //HTTP_FEATURE

#ifdef TELNET_FEATURE
    //20-TELNET On
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TELNET_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TELNET_ON));
    output->print ("\",\"H\":\"Enable Telnet\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //21-TELNET Port
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TELNET_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_TELNET_PORT));
    output->print ("\",\"H\":\"Telnet Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_TELNET_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_TELNET_PORT));
    output->print ("\"}");
#endif //TELNET

#ifdef FTP_FEATURE
    //FTP On
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_FTP_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_FTP_ON));
    output->print ("\",\"H\":\"Enable Ftp\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //FTP Ports
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_FTP_CTRL_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_CTRL_PORT));
    output->print ("\",\"H\":\"Ftp Control Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_CTRL_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_CTRL_PORT));
    output->print ("\"}");

    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_FTP_DATA_ACTIVE_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\",\"H\":\"FTP Active Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\"}");

    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_FTP_DATA_PASSIVE_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\",\"H\":\"FTP Passive Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\"}");
#endif //FTP_FEATURE

#ifdef CAMERA_DEVICE
    //Camera Port
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_CAMERA_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_CAMERA_PORT));
    output->print ("\",\"H\":\"Camera Port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_CAMERA_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_CAMERA_PORT));
    output->print ("\"}");
#endif //CAMERA_DEVICE

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
    output->print ("\"}");

    //23-User password
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_USER_PWD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_USER_PWD));
    output->print ("\",\"H\":\"User Password\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_USER_PWD));
    output->print ("\"}");
#endif //AUTHENTICATION_FEATURE

#ifdef TIMESTAMP_FEATURE

    //24a-Time zone
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_INTERNET_TIME);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print ((int8_t)Settings_ESP3D::read_byte(ESP_INTERNET_TIME));
    output->print("\",\"H\":\"Internet Time\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //24b-Time zone
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TIMEZONE);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print ((int8_t)Settings_ESP3D::read_byte(ESP_TIMEZONE));
    output->print("\",\"H\":\"Time Zone\",\"O\":[");
    for (int8_t i = Settings_ESP3D::get_min_byte(ESP_TIMEZONE); i <= Settings_ESP3D::get_max_byte(ESP_TIMEZONE) ; i++) {
        if (i > 1) {
            output->print (",");
        }
        output->printf("{\"%d\":\"%d\"}", i, i);
    }
    output->print("]}");

    //25- DST
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TIME_IS_DST);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TIME_IS_DST));
    output->print("\",\"H\":\"Day Saving Time\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");

    //26- Time Server1
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TIME_SERVER1);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_TIME_SERVER1));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER1));
    output->print ("\",\"H\":\"Time Server 1\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER1));
    output->print ("\"}");

    //27- Time Server2
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TIME_SERVER2);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_TIME_SERVER2));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER2));
    output->print ("\",\"H\":\"Time Server 2\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER2));
    output->print ("\"}");

    //28- Time Server3
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_TIME_SERVER3);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_TIME_SERVER3));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER3));
    output->print ("\",\"H\":\"Time Server 3\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER3));
    output->print ("\"}");
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
    //Auto notification
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_AUTO_NOTIFICATION);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_AUTO_NOTIFICATION));
    output->print("\",\"H\":\"Auto notification\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");
    //Notification type
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TYPE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE));
    output->print ("\",\"H\":\"Notification\",\"O\":[{\"None\":\"0\"},{\"Pushover\":\"");
    output->print (ESP_PUSHOVER_NOTIFICATION);
    output->print ("\"},{\"Email\":\"");
    output->print (ESP_EMAIL_NOTIFICATION);
    output->print ("\"},{\"Line\":\"");
    output->print (ESP_LINE_NOTIFICATION);
    output->print ("\"}]}");
    //Token 1
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TOKEN1);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN1));
    output->print ("\",\"H\":\"Token 1\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN1));
    output->print ("\"}");
    //Token 2
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TOKEN2);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN2));
    output->print ("\",\"H\":\"Token 2\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN2));
    output->print ("\"}");
    //Notifications Settings
    output->print (",{\"F\":\"network\",\"P\":\"");
    output->print (ESP_NOTIFICATION_SETTINGS);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_SETTINGS));
    output->print ("\",\"H\":\"Notifications Settings\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_SETTINGS));
    output->print ("\"}");
#endif //NOTIFICATION_FEATURE
#ifdef BUZZER_DEVICE
    //Buzzer state
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_BUZZER);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_BUZZER));
    output->print ("\",\"H\":\"Buzzer\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}");
#endif //BUZZER_DEVICE
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
    output->print ("\"}]}");
#ifdef DHT_DEVICE
    //DHT type
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_DHT_TYPE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_DHT_TYPE));
    output->print ("\",\"H\":\"DHT Type\",\"O\":[{\"None\":\"0\"},{\"DHT11\":\"");
    output->print (DHT11_DEVICE);
    output->print ("\"},{\"DHT22\":\"");
    output->print (DHT22_DEVICE);
    output->print ("\"}]}");

    //DHT interval
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_DHT_INTERVAL);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_DHT_INTERVAL));
    output->print ("\",\"H\":\"DHT interval (millisec)\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_DHT_INTERVAL));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_DHT_INTERVAL));
    output->print ("\"}");
#endif //DHT_DEVICE
    //Start delay
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_BOOT_DELAY);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_BOOT_DELAY));
    output->print ("\",\"H\":\"Start delay\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_BOOT_DELAY));
    output->print ("\"}");
    //Output flag
    output->print (",{\"F\":\"printer\",\"P\":\"");
    output->print (ESP_OUTPUT_FLAG);
    output->print ("\",\"T\":\"F\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TARGET_FW));
    output->printf ("\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"%d\"}", ESP_PRINTER_LCD_CLIENT);
#ifdef DISPLAY_DEVICE
    output->printf (",{\"Screen\":\"%d\"}", ESP_SCREEN_CLIENT);
#endif //DISPLAY_DEVICE
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
    output->print ("]}");
#ifdef SD_DEVICE
    //Direct SD
    output->print(",{\"F\":\"printer\",\"P\":\"");
    output->print(ESP_SD_DEVICE_TYPE);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_SD_DEVICE_TYPE));
    //hard coded for readibility but should use ESP_NO_SD / ESP_DIRECT_SD / ESP_SHARED_SD
    output->print("\",\"H\":\"SD Device\",\"O\":[{\"None\":\"0\"},{\"Direct\":\"1\"},{\"Shared\":\"2\"}]}");
    //SPI SD Divider
    output->print(",{\"F\":\"printer\",\"P\":\"");
    output->print(ESP_SD_SPEED_DIV);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV));
    output->print("\",\"H\":\"SD speed divider\",\"O\":[{\"1\":\"1\"},{\"2\":\"2\"},{\"3\":\"3\"},{\"4\":\"4\"},{\"6\":\"6\"},{\"8\":\"8\"},{\"16\":\"16\"},{\"32\":\"32\"}]}");
#endif //SD_DEVICE      

    output->print ("]}");
    return true;
}
