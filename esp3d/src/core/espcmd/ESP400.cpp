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

#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined(BT_FEATURE)
    //Hostname network/network
    output->print ("{\"F\":\"network/network\",\"P\":\"");
    output->print (ESP_HOSTNAME);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_HOSTNAME));
    output->print ("\",\"H\":\"hostname\" ,\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_HOSTNAME));
    output->print ("\", \"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_HOSTNAME));
    output->print ("\"}");
#endif //WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
    //radio mode network/network
    output->print (",{\"F\":\"network/network\",\"P\":\"");
    output->print (ESP_RADIO_MODE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_RADIO_MODE));
    output->print ("\",\"H\":\"radio mode\",\"O\":[{\"none\":\"0\"}");
#ifdef WIFI_FEATURE
    output->print (",{\"sta\":\"1\"},{\"ap\":\"2\"}");
#endif //WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
    output->print (",{\"bt\":\"3\"}");
#endif  //BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
    output->print (",{\"eth-sta\":\"4\"}");
#endif  //ETH_FEATURE
    output->print ("]}");
#ifdef WIFI_FEATURE
    //STA SSID network/sta
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_SSID);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_STA_SSID));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_STA_SSID));
    output->print ("\",\"H\":\"SSID\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_STA_SSID));
    output->print ("\"}");

    //STA password
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_PASSWORD);
    output->print ("\",\"T\":\"S\",\"N\":\"1\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_STA_PASSWORD));
    output->print ("\",\"H\":\"pwd\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_STA_PASSWORD));
    output->print ("\"}");

#endif  //WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    //STA IP mode
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_IP_MODE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_STA_IP_MODE));
    output->print ("\",\"H\":\"ip mode\",\"O\":[{\"dhcp\":\"1\"},{\"static\":\"0\"}]}");

    //STA static IP
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE));
    output->print ("\",\"H\":\"ip\"}");

    //STA static Gateway
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_GATEWAY_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE));
    output->print ("\",\"H\":\"gw\"}");

    //STA static Mask
    output->print (",{\"F\":\"network/sta\",\"P\":\"");
    output->print (ESP_STA_MASK_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE));
    output->print ("\",\"H\":\"msk\"}");
#endif  //WIFI_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE)
    //AP SSID network/ap
    output->print (",{\"F\":\"network/ap\",\"P\":\"");
    output->print (ESP_AP_SSID);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (Settings_ESP3D::read_string(ESP_AP_SSID));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_SSID));
    output->print ("\",\"H\":\"SSID\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_AP_SSID));
    output->print ("\"}");

    //AP password
    output->print (",{\"F\":\"network/ap\",\"P\":\"");
    output->print (ESP_AP_PASSWORD);
    output->print ("\",\"T\":\"S\",\"N\":\"1\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
    output->print ("\",\"H\":\"pwd\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_AP_PASSWORD));
    output->print ("\"}");

    //AP static IP
    output->print (",{\"F\":\"network/ap\",\"P\":\"");
    output->print (ESP_AP_IP_VALUE);
    output->print ("\",\"T\":\"A\",\"V\":\"");
    output->print (Settings_ESP3D::read_IP_String(ESP_AP_IP_VALUE));
    output->print ("\",\"H\":\"ip\"}");

    //AP Channel
    output->print (",{\"F\":\"network/ap\",\"P\":\"");
    output->print (ESP_AP_CHANNEL);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_AP_CHANNEL));
    output->print ("\",\"H\":\"channel\",\"O\":[");
    for (uint8_t i = Settings_ESP3D::get_min_byte(ESP_AP_CHANNEL); i <= Settings_ESP3D::get_max_byte(ESP_AP_CHANNEL) ; i++) {
        if (i > 1) {
            output->print (",");
        }
        output->printf("{\"%d\":\"%d\"}", i, i);
    }
    output->print ("]}");
#endif  //WIFI_FEATURE

#ifdef AUTHENTICATION_FEATURE
    //Admin password
    output->print (",{\"F\":\"security/security\",\"P\":\"");
    output->print (ESP_ADMIN_PWD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_ADMIN_PWD));
    output->print ("\",\"H\":\"adm pwd\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_ADMIN_PWD));
    output->print ("\"}");

    //User password
    output->print (",{\"F\":\"security/security\",\"P\":\"");
    output->print (ESP_USER_PWD);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_USER_PWD));
    output->print ("\",\"H\":\"user pwd\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_USER_PWD));
    output->print ("\"}");
#endif //AUTHENTICATION_FEATURE

#ifdef HTTP_FEATURE
    //HTTP On service/http
    output->print (",{\"F\":\"service/http\",\"P\":\"");
    output->print (ESP_HTTP_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_HTTP_ON));
    output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

    //HTTP Port
    output->print (",{\"F\":\"service/http\",\"P\":\"");
    output->print (ESP_HTTP_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_HTTP_PORT));
    output->print ("\",\"H\":\"port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_HTTP_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_HTTP_PORT));
    output->print ("\"}");
#endif //HTTP_FEATURE

#ifdef TELNET_FEATURE
    //TELNET On service/telnet
    output->print (",{\"F\":\"service/telnetp\",\"P\":\"");
    output->print (ESP_TELNET_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TELNET_ON));
    output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

    //TELNET Port
    output->print (",{\"F\":\"service/telnetp\",\"P\":\"");
    output->print (ESP_TELNET_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_TELNET_PORT));
    output->print ("\",\"H\":\"port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_TELNET_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_TELNET_PORT));
    output->print ("\"}");
#endif //TELNET

#ifdef FTP_FEATURE
    //FTP On service/ftp
    output->print (",{\"F\":\"service/ftp\",\"P\":\"");
    output->print (ESP_FTP_ON);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_FTP_ON));
    output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

    //FTP Ports
    output->print (",{\"F\":\"service/ftp\",\"P\":\"");
    output->print (ESP_FTP_CTRL_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_CTRL_PORT));
    output->print ("\",\"H\":\"control port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_CTRL_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_CTRL_PORT));
    output->print ("\"}");

    output->print (",{\"F\":\"service/ftp\",\"P\":\"");
    output->print (ESP_FTP_DATA_ACTIVE_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\",\"H\":\"active port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
    output->print ("\"}");

    output->print (",{\"F\":\"service/ftp\",\"P\":\"");
    output->print (ESP_FTP_DATA_PASSIVE_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\",\"H\":\"passive port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
    output->print ("\"}");
#endif //FTP_FEATURE

#ifdef TIMESTAMP_FEATURE

    //Internet Time
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_INTERNET_TIME);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print ((int8_t)Settings_ESP3D::read_byte(ESP_INTERNET_TIME));
    output->print("\",\"H\":\"i-time\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

    //Time zone
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_TIMEZONE);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print ((int8_t)Settings_ESP3D::read_byte(ESP_TIMEZONE));
    output->print("\",\"H\":\"tzone\",\"O\":[");
    for (int8_t i = Settings_ESP3D::get_min_byte(ESP_TIMEZONE); i <= Settings_ESP3D::get_max_byte(ESP_TIMEZONE) ; i++) {
        if (i > Settings_ESP3D::get_min_byte(ESP_TIMEZONE)) {
            output->print (",");
        }
        output->printf("{\"%d\":\"%d\"}", i, i);
    }
    output->print("]}");

    //DST
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_TIME_IS_DST);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TIME_IS_DST));
    output->print("\",\"H\":\"dst\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

    //Time Server1
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_TIME_SERVER1);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (strlen(Settings_ESP3D::read_string(ESP_TIME_SERVER1))==0?" ":Settings_ESP3D::read_string(ESP_TIME_SERVER1));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER1));
    output->print ("\",\"H\":\"t-server\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER1));
    output->print ("\"}");

    //27- Time Server2
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_TIME_SERVER2);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (strlen(Settings_ESP3D::read_string(ESP_TIME_SERVER2))==0?" ":Settings_ESP3D::read_string(ESP_TIME_SERVER2));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER2));
    output->print ("\",\"H\":\"t-server\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER2));
    output->print ("\"}");

    //28- Time Server3
    output->print (",{\"F\":\"service/time\",\"P\":\"");
    output->print (ESP_TIME_SERVER3);
    output->print("\",\"T\":\"S\",\"V\":\"");
    output->print (strlen(Settings_ESP3D::read_string(ESP_TIME_SERVER3))==0?" ":Settings_ESP3D::read_string(ESP_TIME_SERVER3));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER3));
    output->print ("\",\"H\":\"t-server\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER3));
    output->print ("\"}");
#endif //TIMESTAMP_FEATURE

#ifdef NOTIFICATION_FEATURE
    //Auto notification
    output->print (",{\"F\":\"service/notification\",\"P\":\"");
    output->print (ESP_AUTO_NOTIFICATION);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_AUTO_NOTIFICATION));
    output->print("\",\"H\":\"auto notif\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
    //Notification type
    output->print (",{\"F\":\"service/notification\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TYPE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE));
    output->print ("\",\"H\":\"notification\",\"O\":[{\"none\":\"0\"},{\"pushover\":\"");
    output->print (ESP_PUSHOVER_NOTIFICATION);
    output->print ("\"},{\"email\":\"");
    output->print (ESP_EMAIL_NOTIFICATION);
    output->print ("\"},{\"line\":\"");
    output->print (ESP_LINE_NOTIFICATION);
    output->print ("\"},{\"telegram\":\"");
    output->print (ESP_TELEGRAM_NOTIFICATION);
    output->print ("\"}]}");
    //Token 1
    output->print (",{\"F\":\"service/notification\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TOKEN1);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN1));
    output->print ("\",\"H\":\"t1\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN1));
    output->print ("\"}");
    //Token 2
    output->print (",{\"F\":\"service/notification\",\"P\":\"");
    output->print (ESP_NOTIFICATION_TOKEN2);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print (HIDDEN_PASSWORD);
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN2));
    output->print ("\",\"H\":\"t2\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN2));
    output->print ("\"}");
    //Notifications Settings
    output->print (",{\"F\":\"service/notification\",\"P\":\"");
    output->print (ESP_NOTIFICATION_SETTINGS);
    output->print ("\",\"T\":\"S\",\"V\":\"");
    output->print ((strlen(Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS))==0)?" ":Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS));
    output->print ("\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_SETTINGS));
    output->print ("\",\"H\":\"ts\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_SETTINGS));
    output->print ("\"}");
#endif //NOTIFICATION_FEATURE
#ifdef CAMERA_DEVICE
    //Camera Port
    output->print (",{\"F\":\"device/camera\",\"P\":\"");
    output->print (ESP_CAMERA_PORT);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_CAMERA_PORT));
    output->print ("\",\"H\":\"port\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_CAMERA_PORT));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_CAMERA_PORT));
    output->print ("\"}");
#endif //CAMERA_DEVICE
#ifdef BUZZER_DEVICE
    //Buzzer state
    output->print (",{\"F\":\"device/device\",\"P\":\"");
    output->print (ESP_BUZZER);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_BUZZER));
    output->print ("\",\"H\":\"buzzer\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //BUZZER_DEVICE

#ifdef DHT_DEVICE
    //DHT type
    output->print (",{\"F\":\"device/dht\",\"P\":\"");
    output->print (ESP_DHT_TYPE);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_DHT_TYPE));
    output->print ("\",\"H\":\"type\",\"O\":[{\"none\":\"0\"},{\"11\":\"");
    output->print (DHT11_DEVICE);
    output->print ("\"},{\"22\":\"");
    output->print (DHT22_DEVICE);
    output->print ("\"}]}");

    //DHT interval
    output->print (",{\"F\":\"device/dht\",\"P\":\"");
    output->print (ESP_DHT_INTERVAL);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_DHT_INTERVAL));
    output->print ("\",\"H\":\"intervalms\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_DHT_INTERVAL));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_DHT_INTERVAL));
    output->print ("\"}");
#endif //DHT_DEVICE
#ifdef SD_DEVICE
    //Direct SD
    output->print(",{\"F\":\"device/sd\",\"P\":\"");
    output->print(ESP_SD_DEVICE_TYPE);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_SD_DEVICE_TYPE));
    //hard coded for readibility but should use ESP_NO_SD / ESP_DIRECT_SD / ESP_SHARED_SD
    output->print("\",\"H\":\"type\",\"O\":[{\"none\":\"0\"},{\"direct\":\"1\"},{\"dhared\":\"2\"}]}");
    //SPI SD Divider
    output->print(",{\"F\":\"device/sd\",\"P\":\"");
    output->print(ESP_SD_SPEED_DIV);
    output->print("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV));
    output->print("\",\"H\":\"speedx\",\"O\":[{\"1\":\"1\"},{\"2\":\"2\"},{\"3\":\"3\"},{\"4\":\"4\"},{\"6\":\"6\"},{\"8\":\"8\"},{\"16\":\"16\"},{\"32\":\"32\"}]}");
#endif //SD_DEVICE 
    //Target FW
    output->print (",{\"F\":\"system/system\",\"P\":\"");
    output->print (ESP_TARGET_FW);
    output->print ("\",\"T\":\"B\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_TARGET_FW));
    output->print ("\",\"H\":\"targetfw\",\"O\":[{\"repetier\":\"");
    output->print (REPETIER);
    output->print ("\"},{\"repetier4davinci\":\"");
    output->print (REPETIER4DV);
    output->print ("\"},{\"marlin\":\"");
    output->print (MARLIN);
    output->print ("\"},{\"marlinkimbra\":\"");
    output->print (MARLINKIMBRA);
    output->print ("\"},{\"smoothieware\":\"");
    output->print (SMOOTHIEWARE);
    output->print ("\"},{\"grbl\":\"");
    output->print (GRBL);
    output->print ("\"},{\"unknown\":\"");
    output->print (UNKNOWN_FW);
    output->print ("\"}]}");
    //Baud Rate
    output->print (",{\"F\":\"system/system\",\"P\":\"");
    output->print (ESP_BAUD_RATE);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_BAUD_RATE));
    output->print ("\",\"H\":\"baud\",\"O\":[");
    uint8_t count = 0;
    const long *bl =  serial_service.get_baudratelist(&count);
    for (uint8_t i = 0 ; i < count ; i++) {
        if (i > 0) {
            output->print (",");
        }
        output->printf("{\"%ld\":\"%ld\"}", bl[i], bl[i]);
    }
    output->print ("]}");
    //Start delay
    output->print (",{\"F\":\"system/system\",\"P\":\"");
    output->print (ESP_BOOT_DELAY);
    output->print ("\",\"T\":\"I\",\"V\":\"");
    output->print (Settings_ESP3D::read_uint32(ESP_BOOT_DELAY));
    output->print ("\",\"H\":\"bootdelay\",\"S\":\"");
    output->print (Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY));
    output->print ("\",\"M\":\"");
    output->print (Settings_ESP3D::get_min_int32_value(ESP_BOOT_DELAY));
    output->print ("\"}");
    //Output flag
    output->print (",{\"F\":\"system/system\",\"P\":\"");
    output->print (ESP_OUTPUT_FLAG);
    output->print ("\",\"T\":\"F\",\"V\":\"");
    output->print (Settings_ESP3D::read_byte(ESP_OUTPUT_FLAG));
    output->printf ("\",\"H\":\"outputmsg\",\"O\":[{\"M117\":\"%d\"}", ESP_PRINTER_LCD_CLIENT);
#ifdef DISPLAY_DEVICE
    output->printf (",{\"screen\":\"%d\"}", ESP_SCREEN_CLIENT);
#endif //DISPLAY_DEVICE
    output->printf (",{\"serial\":\"%d\"}", ESP_SERIAL_CLIENT);
#ifdef WS_DATA_FEATURE
    output->printf (",{\"ws\":\"%d\"}", ESP_WEBSOCKET_CLIENT);
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
    output->printf (",{\"bt\":\"%d\"}", ESP_BT_CLIENT);
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
    output->printf (",{\"telnet\":\"%d\"}", ESP_TELNET_CLIENT);
#endif //TELNET_FEATURE
    output->print ("]}");

    output->print ("]}");
    return true;
}
