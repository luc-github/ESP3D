/*
  notifications_service.cpp -  notifications service functions class

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
//Inspired by following sources
//* Line :
// - https://github.com/TridentTD/TridentTD_LineNotify
// - https://notify-bot.line.me/doc/en/
//* Pushover:
// - https://github.com/ArduinoHannover/Pushover
// - https://pushover.net/api
//* Email:
// - https://github.com/CosmicBoris/ESP8266SMTP
// - https://www.electronicshub.org/send-an-email-using-esp8266/

#include "config.h"
#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#include "wificonf.h"
#if defined( ARDUINO_ARCH_ESP8266)
#define USING_AXTLS
#if defined(USING_AXTLS)
#include "WiFiClientSecureAxTLS.h"
using namespace axTLS;
typedef axTLS::WiFiClientSecure TSecureClient;
#else
#include <WiFiClientSecure.h>
typedef WiFiClientSecure TSecureClient;
#endif //USING_AXTLS
#endif //ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFiClientSecure.h>
typedef WiFiClientSecure TSecureClient;
#endif //ARDUINO_ARCH_ESP32

#include <base64.h>

#define PUSHOVERTIMEOUT 5000
#define PUSHOVERSERVER "api.pushover.net"
#define PUSHOVERPORT	443

#define LINETIMEOUT 5000
#define LINESERVER "notify-api.line.me"
#define LINEPORT	443

#define EMAILTIMEOUT 5000

NotificationsService notificationsservice;

bool Wait4Answer(TSecureClient & client, const char * linetrigger, const char * expected_answer,  uint32_t timeout)
{
    if(client.connected()) {
        String answer;
        uint32_t starttimeout = millis();
        while (client.connected() && ((millis() -starttimeout) < timeout)) {
            answer = client.readStringUntil('\n');
            //log_esp3d("Answer: %s", answer.c_str());
            if ((answer.indexOf(linetrigger) != -1) || (strlen(linetrigger) == 0)) {
                break;
            }
            CONFIG::wait(10);
        }
        if (strlen(expected_answer) == 0) {
            //log_esp3d("Answer ignored as requested");
            return true;
        }
        if(answer.indexOf(expected_answer) == -1) {
            //log_esp3d("Did not got answer!");
            return false;
        } else {
            //log_esp3d("Got expected answer");
            return true;
        }
    }
    //log_esp3d("Failed to send message");
    return false;
}

bool NotificationsService::sendAutoNotification(const char * msg) 
{ 
    if ((WiFi.getMode() == WIFI_AP) || (!_started) || (!_autonotification))return false;
    String msgtpl = msg;
    //check if has variable to change
    if (msgtpl.indexOf("%") != -1) {
    msgtpl.replace("%ESP_IP%", WiFi.localIP().toString().c_str());
    msgtpl.replace("%ESP_NAME%", wifi_config.get_hostname());
    }
    return sendMSG("ESP3D Notification", msgtpl.c_str());
}

NotificationsService::NotificationsService()
{
    _started = false;
    _autonotification = false;
    _notificationType = 0;
    _token1 = "";
    _token1 = "";
    _settings = "";
}
NotificationsService::~NotificationsService()
{
    end();
}

bool NotificationsService::started()
{
    return _started;
}

const char * NotificationsService::getTypeString()
{
    switch(_notificationType) {
    case ESP_PUSHOVER_NOTIFICATION:
        return "Pushover";
    case ESP_EMAIL_NOTIFICATION:
        return "Email";
    case ESP_LINE_NOTIFICATION:
        return "Line";
    default:
        break;
    }
    return "None";
}

bool NotificationsService::sendMSG(const char * title, const char * message)
{
    if(!_started) {
        return false;
    }
    if (!((strlen(title) == 0) && (strlen(message) == 0))) {
        switch(_notificationType) {
        case ESP_PUSHOVER_NOTIFICATION:
            return sendPushoverMSG(title,message);
            break;
        case ESP_EMAIL_NOTIFICATION:
            return sendEmailMSG(title,message);
            break;
        case ESP_LINE_NOTIFICATION :
            return sendLineMSG(title,message);
            break;
        default:
            break;
        }
    }
    return false;
}
//Messages are currently limited to 1024 4-byte UTF-8 characters
//but we do not do any check
bool NotificationsService::sendPushoverMSG(const char * title, const char * message)
{
    String data;
    String postcmd;
    bool res;
    TSecureClient Notificationclient;
#if defined(ARDUINO_ARCH_ESP8266) && !defined(USING_AXTLS)
    Notificationclient.setInsecure();
#endif //ARDUINO_ARCH_ESP8266 && !USING_AXTLS
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        //log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    //build data for post
    data = "user=";
    data += _token1;
    data += "&token=";
    data += _token2;;
    data +="&title=";
    data += title;
    data += "&message=";
    data += message;
    data += "&device=";
    data += wifi_config.get_hostname();
    //build post query
    postcmd  = "POST /1/messages.json HTTP/1.1\r\nHost: api.pushover.net\r\nConnection: close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Length: ";
    postcmd  += data.length();
    postcmd  +="\r\n\r\n";
    postcmd  +=data;
    //log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "{", "\"status\":1",  PUSHOVERTIMEOUT);
    Notificationclient.stop();
    return res;
}
bool NotificationsService::sendEmailMSG(const char * title, const char * message)
{
    TSecureClient Notificationclient;
#if defined(ARDUINO_ARCH_ESP8266) && !defined(USING_AXTLS)
    Notificationclient.setInsecure();
#endif //ARDUINO_ARCH_ESP8266 && !USING_AXTLS
    //log_esp3d("Connect to server");
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        //log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    //Check answer of connection
    if(!Wait4Answer(Notificationclient, "220", "220", EMAILTIMEOUT)) {
        //log_esp3d("Connection failed!");
        return false;
    }
    //Do HELO
    //log_esp3d("HELO");
    Notificationclient.print("HELO friend\r\n");
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        //log_esp3d("HELO failed!");
        return false;
    }
    //log_esp3d("AUTH LOGIN");
    //Request AUthentication
    Notificationclient.print("AUTH LOGIN\r\n");
    if(!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
        //log_esp3d("AUTH LOGIN failed!");
        return false;
    }
    //log_esp3d("Send LOGIN");
    //sent Login
    Notificationclient.printf("%s\r\n",_token1.c_str());
    if(!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
        //log_esp3d("Sent login failed!");
        return false;
    }
    //log_esp3d("Send PASSWORD");
    //Send password
    Notificationclient.printf("%s\r\n",_token2.c_str());
    if(!Wait4Answer(Notificationclient, "235", "235", EMAILTIMEOUT)) {
        //log_esp3d("Sent password failed!");
        return false;
    }
    //log_esp3d("MAIL FROM");
    //Send From
    Notificationclient.printf("MAIL FROM: <%s>\r\n",_settings.c_str());
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        //log_esp3d("MAIL FROM failed!");
        return false;
    }
    //log_esp3d("RCPT TO");
    //Send To
    Notificationclient.printf("RCPT TO: <%s>\r\n",_settings.c_str());
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        //log_esp3d("RCPT TO failed!");
        return false;
    }
    //log_esp3d("DATA");
    //Send Data
    Notificationclient.print("DATA\r\n");
    if(!Wait4Answer(Notificationclient, "354", "354", EMAILTIMEOUT)) {
        //log_esp3d("Preparing DATA failed!");
        return false;
    }
    //log_esp3d("Send message");
    //Send message
    Notificationclient.printf("From:ESP3D<%s>\r\n",_settings.c_str());
    Notificationclient.printf("To: <%s>\r\n",_settings.c_str());
    Notificationclient.printf("Subject: %s\r\n\r\n",title);
    Notificationclient.println(message);

    //log_esp3d("Send final dot");
    //Send Final dot
    Notificationclient.print(".\r\n");
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        //log_esp3d("Sending final dot failed!");
        return false;
    }
    //log_esp3d("QUIT");
    //Quit
    Notificationclient.print("QUIT\r\n");
    if(!Wait4Answer(Notificationclient, "221", "221", EMAILTIMEOUT)) {
        //log_esp3d("QUIT failed!");
        return false;
    }

    Notificationclient.stop();
    return true;
}
bool NotificationsService::sendLineMSG(const char * title, const char * message)
{
    String data;
    String postcmd;
    bool res;
    TSecureClient Notificationclient;
#if defined(ARDUINO_ARCH_ESP8266) && !defined(USING_AXTLS)
    Notificationclient.setInsecure();
#endif //ARDUINO_ARCH_ESP8266 && !USING_AXTLS
    (void)title;
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        //log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    //build data for post
    data = "message=";
    data += message;
    //build post query
    postcmd  = "POST /api/notify HTTP/1.1\r\nHost: notify-api.line.me\r\nConnection: close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Type: application/x-www-form-urlencoded\r\n";
    postcmd  +="Authorization: Bearer ";
    postcmd  += _token1 + "\r\n";
    postcmd  += "Content-Length: ";
    postcmd  += data.length();
    postcmd  +="\r\n\r\n";
    postcmd  +=data;
    //log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "{", "\"status\":200",  LINETIMEOUT);
    Notificationclient.stop();
    return res;
}
//Email#serveraddress:port
bool NotificationsService::getPortFromSettings()
{
    String tmp ;
    char sbuf[MAX_DATA_LENGTH + 1];
    if (CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_SETTINGS_LENGTH) ) {
        tmp = sbuf;
    }
    int pos = tmp.lastIndexOf(':');
    if (pos == -1) {
        return false;
    }
    _port= tmp.substring(pos+1).toInt();
    //log_esp3d("port : %d", _port);
    if (_port > 0) {
        return true;
    } else {
        return false;
    }
}
//Email#serveraddress:port
bool NotificationsService::getServerAddressFromSettings()
{
    String tmp ;
    char sbuf[MAX_DATA_LENGTH + 1];
    if (CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_SETTINGS_LENGTH) ) {
        tmp = sbuf;
    }
    int pos1 = tmp.indexOf('#');
    int pos2 = tmp.lastIndexOf(':');
    if ((pos1 == -1) || (pos2 == -1)) {
        return false;
    }

    //TODO add a check for valid email ?
    _serveraddress = tmp.substring(pos1+1, pos2);
    //log_esp3d("server : %s", _serveraddress.c_str());
    return true;
}
//Email#serveraddress:port
bool NotificationsService::getEmailFromSettings()
{
    String tmp ;
    char sbuf[MAX_DATA_LENGTH + 1];
    if (CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_SETTINGS_LENGTH) ) {
        tmp = sbuf;
    }
    int pos = tmp.indexOf('#');
    if (pos == -1) {
        return false;
    }
    _settings = tmp.substring(0, pos);
    //log_esp3d("email : %s", _settings.c_str());
    //TODO add a check for valid email ?
    return true;
}


bool NotificationsService::begin()
{
    bool res = true;
    end();
    byte bbuf = 0;
    char sbuf[MAX_DATA_LENGTH + 1];
    if (CONFIG::read_byte (ESP_NOTIFICATION_TYPE, &bbuf ) ) {
        _notificationType =bbuf;
    }
    switch(_notificationType) {
    case 0: //no notification = no error but no start
        return true;
    case ESP_PUSHOVER_NOTIFICATION:
        if (CONFIG::read_string (ESP_NOTIFICATION_TOKEN1, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
            _token1 = sbuf;
        }
        if (CONFIG::read_string (ESP_NOTIFICATION_TOKEN2, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
            _token2 = sbuf;
        }
        _port = PUSHOVERPORT;
        _serveraddress = PUSHOVERSERVER;
        break;
    case ESP_LINE_NOTIFICATION:
        if (CONFIG::read_string (ESP_NOTIFICATION_TOKEN1, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
            _token1 = sbuf;
        }
        _port = LINEPORT;
        _serveraddress = LINESERVER;
        break;
    case ESP_EMAIL_NOTIFICATION:
        if (CONFIG::read_string (ESP_NOTIFICATION_TOKEN1, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
            _token1 = base64::encode(sbuf);
        }
        if (CONFIG::read_string (ESP_NOTIFICATION_TOKEN2, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
            _token2 = base64::encode(sbuf);
        }
        //log_esp3d("%s",Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1));
        //log_esp3d("%s",Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN2));
        if(!getEmailFromSettings() || !getPortFromSettings()|| !getServerAddressFromSettings()) {
            return false;
        }
        break;
    default:
        return false;
        break;
    }
    if (CONFIG::read_byte (ESP_AUTO_NOTIFICATION, &bbuf ) ) {
        _autonotification = (bbuf == 0) ? false: true;
    }
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}
void NotificationsService::end()
{
    if(!_started) {
        return;
    }
    _started = false;
    _notificationType = 0;
    _token1 = "";
    _token1 = "";
    _settings = "";
    _serveraddress = "";
    _port = 0;
}

void NotificationsService::handle()
{
    if (_started) {
    }
}

#endif //NOTIFICATION_FEATURE
