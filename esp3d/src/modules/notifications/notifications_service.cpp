/*
  notifications_service.cpp -  notifications service functions class

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
//* Telegram
// - https://medium.com/@xabaras/sending-a-message-to-a-telegram-channel-the-easy-way-eb0a0b32968

#include "../../include/esp3d_config.h"
#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../network/netconfig.h"
#include <WiFiClientSecure.h>

#if defined( ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <libb64/cdecode.h>
#endif //ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
extern "C" {
#include "libb64/cdecode.h"
}
#endif //ARDUINO_ARCH_ESP32
#if defined (HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../websocket/websocket_server.h"
#endif //HTTP_FEATURE || WS_DATA_FEATURE
#if defined (DISPLAY_DEVICE)
#include "../display/display.h"
#endif //DISPLAY_DEVICE
#include <base64.h>

#define PUSHOVERTIMEOUT 5000
#define PUSHOVERSERVER "api.pushover.net"
#define PUSHOVERPORT    443

#define LINETIMEOUT 5000
#define LINESERVER "notify-api.line.me"
#define LINEPORT    443

#define TELEGRAMTIMEOUT 5000
#define TELEGRAMSERVER "api.telegram.org"
#define TELEGRAMPORT    443

#define IFTTTTIMEOUT 5000
#define IFTTTSERVER "maker.ifttt.com"
#define IFTTTPORT    443

#define EMAILTIMEOUT 5000

NotificationsService notificationsservice;

#if defined(ARDUINO_ARCH_ESP8266)
void NotificationsService::BearSSLSetup(WiFiClientSecure & Notificationclient)
{
    if (Notificationclient.probeMaxFragmentLength(_serveraddress.c_str(), _port, BEARSSL_MFLN_SIZE)) {
        log_esp3d("Handshake success");
        Notificationclient.setBufferSizes(BEARSSL_MFLN_SIZE, 512);
    } else {
        log_esp3d("Handshake failed");
        Notificationclient.setBufferSizes(BEARSSL_MFLN_SIZE_FALLBACK, 512);
    }
}
#endif //ARDUINO_ARCH_ESP8266

//TODO: put error in variable to allow better error handling
bool NotificationsService::Wait4Answer(WiFiClientSecure & client, const char * linetrigger, const char * expected_answer,  uint32_t timeout)
{
    if(client.connected()) {
        String answer;
        uint32_t starttimeout = millis();
        while (client.connected() && ((millis() -starttimeout) < timeout)) {
            answer = client.readStringUntil('\n');
            log_esp3d("Answer: %s", answer.c_str());
            if ((answer.indexOf(linetrigger) != -1) || (strlen(linetrigger) == 0)) {
                break;
            }
            Hal::wait(10);
        }
        if (strlen(expected_answer) == 0) {
            log_esp3d("Answer ignored as requested");
            return true;
        }
        if(answer.indexOf(expected_answer) == -1) {
            log_esp3d("Did not got answer!");
            return false;
        } else {
            log_esp3d("Got expected answer");
            return true;
        }
    }
    log_esp3d("Failed to send message");
    return false;
}

bool NotificationsService::sendAutoNotification(const char * msg)
{
    if (!(NetConfig::started()) || (NetConfig::getMode() != ESP_WIFI_STA)|| (!_started) || (!_autonotification)) {
        log_esp3d("Auto notification rejected");
        return false;
    }
    String msgtpl = msg;
    //check if has variable to change
    if (msgtpl.indexOf("%") != -1) {
        msgtpl.replace("%ESP_IP%", WiFi.localIP().toString().c_str());
        msgtpl.replace("%ESP_NAME%", NetConfig::hostname());
    }
    if (!sendMSG(ESP_NOTIFICATION_TITLE, msgtpl.c_str())) {
        log_esp3d("Auto notification failed");
        return false;
    } else {
        log_esp3d("Auto notification sent");
        return true;
    }
}

NotificationsService::NotificationsService()
{
    _started = false;
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
        return "pushover";
    case ESP_EMAIL_NOTIFICATION:
        return "email";
    case ESP_LINE_NOTIFICATION:
        return "line";
    case ESP_TELEGRAM_NOTIFICATION:
        return "telegram";
    case ESP_IFTTT_NOTIFICATION:
        return "IFTTT";
    default:
        break;
    }
    return "none";
}

bool NotificationsService::sendMSG(const char * title, const char * message)
{
    if(!_started) {
        log_esp3d("Error notification not started");
        return false;
    }
    if (!((strlen(title) == 0) && (strlen(message) == 0))) {
        //push to webui by default
#if defined (HTTP_FEATURE) || defined(WS_DATA_FEATURE)
        String msg = "NOTIFICATION:";

        msg += message;
        websocket_terminal_server.pushMSG(msg.c_str());
#endif //HTTP_FEATURE || WS_DATA_FEATURE
#ifdef DISPLAY_DEVICE
        esp3d_display.setStatus(message);
#endif //DISPLAY_DEVICE
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
        case ESP_TELEGRAM_NOTIFICATION :
            return sendTelegramMSG(title,message);
            break;
        case ESP_IFTTT_NOTIFICATION :
            return sendIFTTTMSG(title,message);
            break;
        default:
            break;
        }
    }
    return true;
}
//Messages are currently limited to 1024 4-byte UTF-8 characters
//but we do not do any check
//TODO: put error in variable to allow better error handling
bool NotificationsService::sendPushoverMSG(const char * title, const char * message)
{
    String data;
    String postcmd;
    bool res;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
    Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
    BearSSLSetup(Notificationclient);
#endif //ARDUINO_ARCH_ESP8266
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    //build data for post
    data = "user=";
    data += _token1;
    data += "&token=";
    data += _token2;
    data +="&title=";
    data += title;
    data += "&message=";
    data += message;
    data += "&device=";
    data += NetConfig::hostname();
    //build post query
    postcmd  = "POST /1/messages.json HTTP/1.1\r\nHost: api.pushover.net\r\nConnection: close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Length: ";
    postcmd  += data.length();
    postcmd  +="\r\n\r\n";
    postcmd  +=data;
    log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "{", "\"status\":1",  PUSHOVERTIMEOUT);
    Notificationclient.stop();
    return res;
}

//Telegram
//TODO: put error in variable to allow better error handling
bool NotificationsService::sendTelegramMSG(const char * title, const char * message)
{
    String data;
    String postcmd;
    bool res;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
    Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
    BearSSLSetup(Notificationclient);
#endif //ARDUINO_ARCH_ESP8266
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    (void)title;
    //build url for get
    data = "chat_id=";
    data += _token2;
    data += "&text=";
    data += message;

    //build post query
    postcmd  = "POST /bot";
    postcmd +=_token1;
    postcmd += "/sendMessage HTTP/1.1\r\nHost: api.telegram.org\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Length: ";
    postcmd  += data.length();
    postcmd  +="\r\n\r\n";
    postcmd  +=data;
    log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "{", "\"ok\":true",  TELEGRAMTIMEOUT);
    Notificationclient.stop();
    return res;
}

//TODO: put error in variable to allow better error handling
bool NotificationsService::sendEmailMSG(const char * title, const char * message)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
    Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
    BearSSLSetup(Notificationclient);
#endif //ARDUINO_ARCH_ESP8266
    log_esp3d("Connect to server");
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }
    //Check answer of connection
    if(!Wait4Answer(Notificationclient, "220", "220", EMAILTIMEOUT)) {
        log_esp3d("Connection failed!");
        return false;
    }
    //Do HELO
    log_esp3d("HELO");
    Notificationclient.print("HELO friend\r\n");
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        log_esp3d("HELO failed!");
        return false;
    }
    log_esp3d("AUTH LOGIN");
    //Request AUthentication
    Notificationclient.print("AUTH LOGIN\r\n");
    if(!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
        log_esp3d("AUTH LOGIN failed!");
        return false;
    }
    log_esp3d("Send LOGIN");
    //sent Login
    Notificationclient.printf("%s\r\n",_token1.c_str());
    if(!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
        log_esp3d("Sent login failed!");
        return false;
    }
    log_esp3d("Send PASSWORD");
    //Send password
    Notificationclient.printf("%s\r\n",_token2.c_str());
    if(!Wait4Answer(Notificationclient, "235", "235", EMAILTIMEOUT)) {
        log_esp3d("Sent password failed!");
        return false;
    }
    log_esp3d("MAIL FROM");
    //Send From
    Notificationclient.printf("MAIL FROM: <%s>\r\n",_settings.c_str());
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        log_esp3d("MAIL FROM failed!");
        return false;
    }
    log_esp3d("RCPT TO");
    //Send To
    Notificationclient.printf("RCPT TO: <%s>\r\n",_settings.c_str());
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        log_esp3d("RCPT TO failed!");
        return false;
    }
    log_esp3d("DATA");
    //Send Data
    Notificationclient.print("DATA\r\n");
    if(!Wait4Answer(Notificationclient, "354", "354", EMAILTIMEOUT)) {
        log_esp3d("Preparing DATA failed!");
        return false;
    }
    log_esp3d("Send message");
    //Send message
    Notificationclient.printf("From:ESP3D<%s>\r\n",_settings.c_str());
    Notificationclient.printf("To: <%s>\r\n",_settings.c_str());
    Notificationclient.printf("Subject: %s\r\n\r\n",title);
    Notificationclient.println(message);

    log_esp3d("Send final dot");
    //Send Final dot
    Notificationclient.print(".\r\n");
    if(!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
        log_esp3d("Sending final dot failed!");
        return false;
    }
    log_esp3d("QUIT");
    //Quit
    Notificationclient.print("QUIT\r\n");
    if(!Wait4Answer(Notificationclient, "221", "221", EMAILTIMEOUT)) {
        log_esp3d("QUIT failed!");
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
    Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
    BearSSLSetup(Notificationclient);
#endif //ARDUINO_ARCH_ESP8266
    (void)title;
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
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
    log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "{", "\"status\":200",  LINETIMEOUT);
    Notificationclient.stop();
    return res;
}

//IFTTT
bool NotificationsService::sendIFTTTMSG(const char * title, const char * message)
{
    String data;
    String postcmd;
    bool res;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
    Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
    BearSSLSetup(Notificationclient);
#endif //ARDUINO_ARCH_ESP8266
    (void)title;
    if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
        log_esp3d("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
        return false;
    }

    //build data for post

    data ="value1=";
    data += title;
    data += "&value2=";
    data += message;
    data += "&value3=";
    data += NetConfig::hostname();

    //build post query
    postcmd  = "POST /trigger/" + _token1 + "/with/key/" + _token2 + "  HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
    postcmd  += data.length();
    postcmd  +="\r\n\r\n";
    postcmd  +=data;

    //log_esp3d("Query: %s", postcmd.c_str());
    //send query
    Notificationclient.print(postcmd);
    res = Wait4Answer(Notificationclient, "Congratulations", "Congratulations",  IFTTTTIMEOUT);
    Notificationclient.stop();
    return res;
}

//Email#serveraddress:port
bool NotificationsService::getPortFromSettings()
{
    String tmp = Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS);
    int pos = tmp.lastIndexOf(':');
    if (pos == -1) {
        return false;
    }
    _port= tmp.substring(pos+1).toInt();
    log_esp3d("port : %d", _port);
    if (_port > 0) {
        return true;
    } else {
        return false;
    }
}
//Email#serveraddress:port
bool NotificationsService::getServerAddressFromSettings()
{
    String tmp = Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS);
    int pos1 = tmp.indexOf('#');
    int pos2 = tmp.lastIndexOf(':');
    if ((pos1 == -1) || (pos2 == -1)) {
        return false;
    }

    //TODO add a check for valid email ?
    _serveraddress = tmp.substring(pos1+1, pos2);
    log_esp3d("server : %s", _serveraddress.c_str());
    return true;
}
//Email#serveraddress:port
bool NotificationsService::getEmailFromSettings()
{
    String tmp = Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS);
    int pos = tmp.indexOf('#');
    if (pos == -1) {
        return false;
    }
    _settings = tmp.substring(0, pos);
    log_esp3d("email : %s", _settings.c_str());
    //TODO add a check for valid email ?
    return true;
}

bool NotificationsService::decode64(const char* encodedURL,  char *decodedURL)
{
    size_t out_len = 0;
    out_len =  base64_decode_chars(encodedURL, strlen(encodedURL), decodedURL);
    log_esp3d("URLE: %s", encodedURL);
    log_esp3d("URLD: %s", decodedURL);
    return (out_len>0);
}

bool NotificationsService::GET(const char * URL64)
{
    //TODO do we need https client ?
    WiFiClient client;
    HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)
    char * decodedurl[255];
    bool res = false;
    if (decode64(URL64, (char*)decodedurl)) {
        http.begin(client, (const char*)decodedurl);
        int httpCode = http.GET();
        log_esp3d("HTTP code: %d", httpCode);
        if (httpCode > 0) {
            if(httpCode == HTTP_CODE_OK) {
                res = true;
            }
        }
        http.end();
    }
    return res;
}

bool NotificationsService::begin()
{
    bool res = true;
    end();
    _notificationType = Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE);
    switch(_notificationType) {
    case 0: //no notification = no error but no start
        _started=true;
        return true;
    case ESP_PUSHOVER_NOTIFICATION:
        _token1 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1);
        _token2 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN2);
        _port = PUSHOVERPORT;
        _serveraddress = PUSHOVERSERVER;
        break;
    case ESP_TELEGRAM_NOTIFICATION:
        _token1 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1);
        _token2 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN2);
        _port = TELEGRAMPORT;
        _serveraddress = TELEGRAMSERVER;
        break;
    case ESP_LINE_NOTIFICATION:
        _token1 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1);
        _port = LINEPORT;
        _serveraddress = LINESERVER;
        break;
    case ESP_IFTTT_NOTIFICATION:
        _token1 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1);
        _token2 = Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN2);
        _port = IFTTTPORT;
        _serveraddress = IFTTTSERVER;
        break;
    case ESP_EMAIL_NOTIFICATION:
        _token1 = base64::encode(Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN1));
        _token2 = base64::encode(Settings_ESP3D::read_string(ESP_NOTIFICATION_TOKEN2));
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
    _autonotification = (Settings_ESP3D::read_byte(ESP_AUTO_NOTIFICATION) == 0) ? false: true;
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
