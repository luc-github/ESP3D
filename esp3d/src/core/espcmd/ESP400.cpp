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
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/serial/serial_service.h"
#endif // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/authentication/authentication_service.h"
#if defined (SENSOR_DEVICE)
#include "../../modules/sensor/sensor.h"
#endif //SENSOR_DEVICE
#define COMMANDID   400
//Get full ESP3D settings
//[ESP400]<pwd=admin>
bool Commands::ESP400(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    uint8_t count = 0;
    const long *bl = NULL;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        response = format_response(COMMANDID, json, false, "Wrong authentication level");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (noError) {
        parameter = clean_param(get_param (cmd_params, ""));
        if (parameter.length() == 0) {
            //Start JSON
            output->print("{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[");
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined(BT_FEATURE)
            //Hostname network/network
            output->print ("{\"F\":\"network/network\",\"P\":\"");
            output->print (ESP_HOSTNAME);
            output->print ("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_HOSTNAME)));
            output->print ("\",\"H\":\"hostname\" ,\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_HOSTNAME));
            output->print ("\", \"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_HOSTNAME));
            output->print ("\"}");
#endif //WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
            //radio mode network/network
            output->print (",{\"F\":\"network/network\",\"P\":\"");
            output->print (ESP_RADIO_MODE);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_RADIO_MODE));
            output->print ("\",\"H\":\"radio mode\",\"O\":[{\"none\":\"0\"}");
#ifdef WIFI_FEATURE
            output->print (",{\"sta\":\"1\"},{\"ap\":\"2\"},{\"setup\":\"5\"}");
#endif //WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
            output->print (",{\"bt\":\"3\"}");
#endif  //BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
            output->print (",{\"eth-sta\":\"4\"}");
#endif  //ETH_FEATURE
            output->print ("]}");
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined(BT_FEATURE)
            //Radio State at Boot
            output->print (",{\"F\":\"network/network\",\"P\":\"");
            output->print (ESP_BOOT_RADIO_STATE);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_BOOT_RADIO_STATE));
            output->print ("\",\"H\":\"radio_boot\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif // 
#ifdef WIFI_FEATURE
            //STA SSID network/sta
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_SSID);
            output->print ("\",\"T\":\"S\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_STA_SSID)));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_STA_SSID));
            output->print ("\",\"H\":\"SSID\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_STA_SSID));
            output->print ("\"}");

            //STA password
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_PASSWORD);
            output->print ("\",\"T\":\"S\",\"N\":\"1\",\"MS\":\"0\",\"R\":\"1\",\"V\":\"");
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
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_STA_IP_MODE));
            output->print ("\",\"H\":\"ip mode\",\"O\":[{\"dhcp\":\"1\"},{\"static\":\"0\"}]}");

            //STA static IP
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_IP_VALUE);
            output->print ("\",\"T\":\"A\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE));
            output->print ("\",\"H\":\"ip\"}");

            //STA static Gateway
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_GATEWAY_VALUE);
            output->print ("\",\"T\":\"A\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE));
            output->print ("\",\"H\":\"gw\"}");

            //STA static Mask
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_MASK_VALUE);
            output->print ("\",\"T\":\"A\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE));
            output->print ("\",\"H\":\"msk\"}");

            //STA static DNS
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_DNS_VALUE);
            output->print ("\",\"T\":\"A\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_IP_String(ESP_STA_DNS_VALUE));
            output->print ("\",\"H\":\"DNS\"}");
#endif  //WIFI_FEATURE || ETH_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined(BT_FEATURE)
            //Sta fallback mode
            output->print (",{\"F\":\"network/sta\",\"P\":\"");
            output->print (ESP_STA_FALLBACK_MODE);
            output->print ("\",\"T\":\"B\",\"R\":\"0\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_STA_FALLBACK_MODE));
            output->print ("\",\"H\":\"sta fallback mode\",\"O\":[{\"none\":\"0\"}");
#ifdef WIFI_FEATURE
            output->print (",{\"setup\":\"5\"}");
#endif //WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
            output->print (",{\"bt\":\"3\"}");
#endif  //BLUETOOTH_FEATURE
            output->print ("]}");
#endif //WIFI_FEATURE || ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
            //AP SSID network/ap
            output->print (",{\"F\":\"network/ap\",\"P\":\"");
            output->print (ESP_AP_SSID);
            output->print ("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_AP_SSID)));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_AP_SSID));
            output->print ("\",\"H\":\"SSID\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_AP_SSID));
            output->print ("\"}");

            //AP password
            output->print (",{\"F\":\"network/ap\",\"P\":\"");
            output->print (ESP_AP_PASSWORD);
            output->print ("\",\"T\":\"S\",\"N\":\"1\",\"MS\":\"0\",\"R\":\"1\",\"V\":\"");
            output->print (HIDDEN_PASSWORD);
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_AP_PASSWORD));
            output->print ("\",\"H\":\"pwd\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_AP_PASSWORD));
            output->print ("\"}");

            //AP static IP
            output->print (",{\"F\":\"network/ap\",\"P\":\"");
            output->print (ESP_AP_IP_VALUE);
            output->print ("\",\"T\":\"A\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_IP_String(ESP_AP_IP_VALUE));
            output->print ("\",\"H\":\"ip\"}");

            //AP Channel
            output->print (",{\"F\":\"network/ap\",\"P\":\"");
            output->print (ESP_AP_CHANNEL);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
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

            //session timeout
            output->print (",{\"F\":\"security/security\",\"P\":\"");
            output->print (ESP_SESSION_TIMEOUT);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SESSION_TIMEOUT));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_byte(ESP_SESSION_TIMEOUT));
            output->print ("\",\"H\":\"session timeout\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_byte(ESP_SESSION_TIMEOUT));
            output->print ("\"}");
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            //Secure Serial
            output->print (",{\"F\":\"security/security\",\"P\":\"");
            output->print (ESP_SECURE_SERIAL);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SECURE_SERIAL));
            output->print ("\",\"H\":\"serial\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //COMMUNICATION_PROTOCOL
#endif //AUTHENTICATION_FEATURE

#ifdef HTTP_FEATURE
            //HTTP On service/http
            output->print (",{\"F\":\"service/http\",\"P\":\"");
            output->print (ESP_HTTP_ON);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_HTTP_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //HTTP Port
            output->print (",{\"F\":\"service/http\",\"P\":\"");
            output->print (ESP_HTTP_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
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
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_TELNET_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //TELNET Port
            output->print (",{\"F\":\"service/telnetp\",\"P\":\"");
            output->print (ESP_TELNET_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_TELNET_PORT));
            output->print ("\",\"H\":\"port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_TELNET_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_TELNET_PORT));
            output->print ("\"}");
#endif //TELNET
#ifdef WS_DATA_FEATURE
            //Websocket On service
            output->print (",{\"F\":\"service/websocketp\",\"P\":\"");
            output->print (ESP_WEBSOCKET_ON);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_WEBSOCKET_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //Websocket Port
            output->print (",{\"F\":\"service/websocketp\",\"P\":\"");
            output->print (ESP_WEBSOCKET_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_WEBSOCKET_PORT));
            output->print ("\",\"H\":\"port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_WEBSOCKET_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_WEBSOCKET_PORT));
            output->print ("\"}");
#endif //WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
            //WebDav On service
            output->print (",{\"F\":\"service/webdavp\",\"P\":\"");
            output->print (ESP_WEBDAV_ON);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_WEBDAV_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //WebDav Port
            output->print (",{\"F\":\"service/webdavp\",\"P\":\"");
            output->print (ESP_WEBDAV_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_WEBDAV_PORT));
            output->print ("\",\"H\":\"port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_WEBDAV_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_WEBDAV_PORT));
            output->print ("\"}");
#endif //WEBDAV_FEATURE
#ifdef FTP_FEATURE
            //FTP On service/ftp
            output->print (",{\"F\":\"service/ftp\",\"P\":\"");
            output->print (ESP_FTP_ON);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_FTP_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //FTP Ports
            output->print (",{\"F\":\"service/ftp\",\"P\":\"");
            output->print (ESP_FTP_CTRL_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_FTP_CTRL_PORT));
            output->print ("\",\"H\":\"control port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_CTRL_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_CTRL_PORT));
            output->print ("\"}");

            output->print (",{\"F\":\"service/ftp\",\"P\":\"");
            output->print (ESP_FTP_DATA_ACTIVE_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_ACTIVE_PORT));
            output->print ("\",\"H\":\"active port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_ACTIVE_PORT));
            output->print ("\"}");

            output->print (",{\"F\":\"service/ftp\",\"P\":\"");
            output->print (ESP_FTP_DATA_PASSIVE_PORT);
            output->print ("\",\"T\":\"I\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_FTP_DATA_PASSIVE_PORT));
            output->print ("\",\"H\":\"passive port\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_FTP_DATA_PASSIVE_PORT));
            output->print ("\"}");
#endif //FTP_FEATURE

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
            //Serial bridge On service
            output->print (",{\"F\":\"service/serial_bridge\",\"P\":\"");
            output->print (ESP_SERIAL_BRIDGE_ON);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SERIAL_BRIDGE_ON));
            output->print ("\",\"H\":\"enable\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
            //Baud Rate
            output->print (",{\"F\":\"service/serial_bridge\",\"P\":\"");
            output->print (ESP_SERIAL_BRIDGE_BAUD);
            output->print ("\",\"T\":\"I\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_SERIAL_BRIDGE_BAUD));
            output->print ("\",\"H\":\"baud\",\"O\":[");

            bl =  serial_service.get_baudratelist(&count);
            for (uint8_t i = 0 ; i < count ; i++) {
                if (i > 0) {
                    output->print (",");
                }
                output->printf("{\"%ld\":\"%ld\"}", bl[i], bl[i]);
            }
            output->print ("]}");
#endif //defined(ESP_SERIAL_BRIDGE_OUTPUT)

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
            output->print("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
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
            output->print("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_TIME_IS_DST));
            output->print("\",\"H\":\"dst\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");

            //Time Server1
            output->print (",{\"F\":\"service/time\",\"P\":\"");
            output->print (ESP_TIME_SERVER1);
            output->print("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_TIME_SERVER1)));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER1));
            output->print ("\",\"H\":\"t-server\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER1));
            output->print ("\"}");

            //27- Time Server2
            output->print (",{\"F\":\"service/time\",\"P\":\"");
            output->print (ESP_TIME_SERVER2);
            output->print("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_TIME_SERVER2)));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER2));
            output->print ("\",\"H\":\"t-server\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_TIME_SERVER2));
            output->print ("\"}");

            //28- Time Server3
            output->print (",{\"F\":\"service/time\",\"P\":\"");
            output->print (ESP_TIME_SERVER3);
            output->print("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_TIME_SERVER3)));
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
            output->print("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_AUTO_NOTIFICATION));
            output->print("\",\"H\":\"auto notif\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
            //Notification type
            output->print (",{\"F\":\"service/notification\",\"P\":\"");
            output->print (ESP_NOTIFICATION_TYPE);
            output->print ("\",\"T\":\"B\",\"R\":\"1\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE));
            output->print ("\",\"H\":\"notification\",\"O\":[{\"none\":\"0\"},{\"pushover\":\"");
            output->print (ESP_PUSHOVER_NOTIFICATION);
            output->print ("\"},{\"email\":\"");
            output->print (ESP_EMAIL_NOTIFICATION);
            output->print ("\"},{\"line\":\"");
            output->print (ESP_LINE_NOTIFICATION);
            output->print ("\"},{\"telegram\":\"");
            output->print (ESP_TELEGRAM_NOTIFICATION);
            output->print ("\"},{\"IFTTT\":\"");
            output->print (ESP_IFTTT_NOTIFICATION);
            output->print ("\"}]}");
            //Token 1
            output->print (",{\"F\":\"service/notification\",\"P\":\"");
            output->print (ESP_NOTIFICATION_TOKEN1);
            output->print ("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (HIDDEN_PASSWORD);
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN1));
            output->print ("\",\"H\":\"t1\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN1));
            output->print ("\"}");
            //Token 2
            output->print (",{\"F\":\"service/notification\",\"P\":\"");
            output->print (ESP_NOTIFICATION_TOKEN2);
            output->print ("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (HIDDEN_PASSWORD);
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_TOKEN2));
            output->print ("\",\"H\":\"t2\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_TOKEN2));
            output->print ("\"}");
            //Notifications Settings
            output->print (",{\"F\":\"service/notification\",\"P\":\"");
            output->print (ESP_NOTIFICATION_SETTINGS);
            output->print ("\",\"T\":\"S\",\"R\":\"1\",\"V\":\"");
            output->print (ESP3DOutput::encodeString(Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS)));
            output->print ("\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_string_size(ESP_NOTIFICATION_SETTINGS));
            output->print ("\",\"H\":\"ts\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_string_size(ESP_NOTIFICATION_SETTINGS));
            output->print ("\"}");
#endif //NOTIFICATION_FEATURE
#ifdef BUZZER_DEVICE
            //Buzzer state
            output->print (",{\"F\":\"device/device\",\"P\":\"");
            output->print (ESP_BUZZER);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_BUZZER));
            output->print ("\",\"H\":\"buzzer\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //BUZZER_DEVICE

#ifdef SENSOR_DEVICE
            //Sensor type
            output->print (",{\"F\":\"device/sensor\",\"P\":\"");
            output->print (ESP_SENSOR_TYPE);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SENSOR_TYPE));
            output->print ("\",\"H\":\"type\",\"O\":[{\"none\":\"0\"}");
            for (uint8_t p = 0; p < esp3d_sensor.nbType(); p++) {
                output->print (",{\"");
                output->print (esp3d_sensor.GetModelString(p));
                output->print ("\":\"");
                output->print (esp3d_sensor.GetModel(p));
                output->print ("\"}");
            }
            output->print ("]}");

            //Sensor interval
            output->print (",{\"F\":\"device/sensor\",\"P\":\"");
            output->print (ESP_SENSOR_INTERVAL);
            output->print ("\",\"T\":\"I\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_SENSOR_INTERVAL));
            output->print ("\",\"H\":\"intervalms\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_SENSOR_INTERVAL));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_SENSOR_INTERVAL));
            output->print ("\"}");
#endif //SENSOR_DEVICE
#if defined(SD_DEVICE)
#if SD_DEVICE != ESP_SDIO
            //SPI SD Divider
            output->print(",{\"F\":\"device/sd\",\"P\":\"");
            output->print(ESP_SD_SPEED_DIV);
            output->print("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV));
            output->print("\",\"H\":\"speedx\",\"O\":[{\"1\":\"1\"},{\"2\":\"2\"},{\"3\":\"3\"},{\"4\":\"4\"},{\"6\":\"6\"},{\"8\":\"8\"},{\"16\":\"16\"},{\"32\":\"32\"}]}");
#endif //SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
            //SD CHECK UPDATE AT BOOT feature
            output->print(",{\"F\":\"device/sd\",\"P\":\"");
            output->print(ESP_SD_CHECK_UPDATE_AT_BOOT);
            output->print("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SD_CHECK_UPDATE_AT_BOOT));
            output->print("\",\"H\":\"SD updater\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //SD_UPDATE_FEATURE
#endif //SD_DEVICE 
#if !defined( FIXED_FW_TARGET )
            //Target FW
            output->print (",{\"F\":\"system/system\",\"P\":\"");
            output->print (ESP_TARGET_FW);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_TARGET_FW));
            output->print ("\",\"H\":\"targetfw\",\"O\":[{\"repetier\":\"");
            output->print (REPETIER);
            output->print ("\"},{\"marlin\":\"");
            output->print (MARLIN);
            output->print ("\"},{\"smoothieware\":\"");
            output->print (SMOOTHIEWARE);
            output->print ("\"},{\"grbl\":\"");
            output->print (GRBL);
            output->print ("\"},{\"unknown\":\"");
            output->print (UNKNOWN_FW);
            output->print ("\"}]}");
#endif //FIXED_FW_TARGET
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            //Baud Rate
            output->print (",{\"F\":\"system/system\",\"P\":\"");
            output->print (ESP_BAUD_RATE);
            output->print ("\",\"T\":\"I\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_BAUD_RATE));
            output->print ("\",\"H\":\"baud\",\"O\":[");

            bl =  serial_service.get_baudratelist(&count);
            for (uint8_t i = 0 ; i < count ; i++) {
                if (i > 0) {
                    output->print (",");
                }
                output->printf("{\"%ld\":\"%ld\"}", bl[i], bl[i]);
            }
            output->print ("]}");
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            //Start delay
            output->print (",{\"F\":\"system/boot\",\"P\":\"");
            output->print (ESP_BOOT_DELAY);
            output->print ("\",\"T\":\"I\",\"V\":\"");
            output->print (Settings_ESP3D::read_uint32(ESP_BOOT_DELAY));
            output->print ("\",\"H\":\"bootdelay\",\"S\":\"");
            output->print (Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY));
            output->print ("\",\"M\":\"");
            output->print (Settings_ESP3D::get_min_int32_value(ESP_BOOT_DELAY));
            output->print ("\"}");
            //Verbose boot
            output->print(",{\"F\":\"system/boot\",\"P\":\"");
            output->print(ESP_VERBOSE_BOOT);
            output->print("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_VERBOSE_BOOT));
            output->print("\",\"H\":\"verbose\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
            //Output flag
            //Serial
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_SERIAL_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SERIAL_FLAG));
            output->print ("\",\"H\":\"serial\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_SERIAL_BRIDGE_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SERIAL_BRIDGE_FLAG));
            output->print ("\",\"H\":\"serial_bridge\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //ESP_SERIAL_BRIDGE_OUTPUT
#if (defined(ESP3DLIB_ENV) && defined(HAS_DISPLAY)) || defined(HAS_SERIAL_DISPLAY)
            //Printer SCREEN
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_REMOTE_SCREEN_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_REMOTE_SCREEN_FLAG));
            output->print ("\",\"H\":\"M117\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //ESP3DLIB_ENV
#ifdef DISPLAY_DEVICE
            //ESP SCREEN
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_SCREEN_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_SCREEN_FLAG));
            output->print ("\",\"H\":\"M117\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //DISPLAY_DEVICE
#ifdef WS_DATA_FEATURE
            //Websocket
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_WEBSOCKET_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_WEBSOCKET_FLAG));
            output->print ("\",\"H\":\"ws\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
            //BT
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_BT_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_BT_FLAG));
            output->print ("\",\"H\":\"BT\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
            //Telnet
            output->print (",{\"F\":\"system/outputmsg\",\"P\":\"");
            output->print (ESP_TELNET_FLAG);
            output->print ("\",\"T\":\"B\",\"V\":\"");
            output->print (Settings_ESP3D::read_byte(ESP_TELNET_FLAG));
            output->print ("\",\"H\":\"telnet\",\"O\":[{\"no\":\"0\"},{\"yes\":\"1\"}]}");
#endif //TELNET_FEATURE
            output->print ("]}");
            if (!json) {
                output->printLN("");
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
