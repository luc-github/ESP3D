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
// Inspired by following sources
//* Line :
//  - https://github.com/TridentTD/TridentTD_LineNotify
//  - https://notify-bot.line.me/doc/en/
//* Pushover:
//  - https://github.com/ArduinoHannover/Pushover
//  - https://pushover.net/api
//* Email:
//  - https://github.com/CosmicBoris/ESP8266SMTP
//  - https://www.electronicshub.org/send-an-email-using-esp8266/
//* Telegram
//  -
//  https://medium.com/@xabaras/sending-a-message-to-a-telegram-channel-the-easy-way-eb0a0b32968
//* Home Assistant
//  - https://developers.home-assistant.io/docs/api/rest/

#include "../../include/esp3d_config.h"
#ifdef NOTIFICATION_FEATURE
#include <WiFiClientSecure.h>

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../network/netconfig.h"
#include "notifications_service.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <libb64/cdecode.h>

#endif  // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
#include <HTTPClient.h>
#include <WiFi.h>

extern "C" {
#include "libb64/cdecode.h"
}
#endif  // ARDUINO_ARCH_ESP32
#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../websocket/websocket_server.h"
#endif  // HTTP_FEATURE || WS_DATA_FEATURE
#if defined(DISPLAY_DEVICE)
#include "../display/display.h"
#endif  // DISPLAY_DEVICE
#include <base64.h>

#define PUSHOVERTIMEOUT 5000
#define PUSHOVERSERVER "api.pushover.net"
#define PUSHOVERPORT 443

#define LINETIMEOUT 5000
#define LINESERVER "notify-api.line.me"
#define LINEPORT 443

#define TELEGRAMTIMEOUT 5000
#define TELEGRAMSERVER "api.telegram.org"
#define TELEGRAMPORT 443

#define IFTTTTIMEOUT 5000
#define IFTTTSERVER "maker.ifttt.com"
#define IFTTTPORT 443

#define HOMEASSISTANTTIMEOUT 5000

#define EMAILTIMEOUT 5000

NotificationsService notificationsservice;

#if defined(ARDUINO_ARCH_ESP8266)
void NotificationsService::BearSSLSetup(WiFiClientSecure& Notificationclient) {
  if (Notificationclient.probeMaxFragmentLength(_serveraddress.c_str(), _port,
                                                BEARSSL_MFLN_SIZE)) {
    esp3d_log("Handshake success");
    Notificationclient.setBufferSizes(BEARSSL_MFLN_SIZE, 512);
  } else {
    esp3d_log_e("Handshake failed");
    Notificationclient.setBufferSizes(BEARSSL_MFLN_SIZE_FALLBACK, 512);
  }
}
#endif  // ARDUINO_ARCH_ESP8266

// TODO: put error in variable to allow better error handling
template<typename T>
bool NotificationsService::Wait4Answer(T& client,
                                       const char* linetrigger,
                                       const char* expected_answer,
                                       uint32_t timeout) {
  if (client.connected()) {
    String answer;
    uint32_t starttimeout = millis();
    while (client.connected() && ((millis() - starttimeout) < timeout)) {
      answer = client.readStringUntil('\n');
      esp3d_log("Answer: %s", answer.c_str());
      if ((answer.indexOf(linetrigger) != -1) || (strlen(linetrigger) == 0)) {
        break;
      }
      ESP3DHal::wait(10);
    }
    if (strlen(expected_answer) == 0) {
      esp3d_log("Answer ignored as requested");
      return true;
    }
    if (answer.indexOf(expected_answer) == -1) {
      esp3d_log("Did not got answer!");
      return false;
    } else {
      esp3d_log("Got expected answer");
      return true;
    }
  }
  esp3d_log_e("Failed to send message");
  return false;
}

bool NotificationsService::sendAutoNotification(const char* msg) {
  if (!(NetConfig::started()) || (NetConfig::getMode() != ESP_WIFI_STA) ||
      (!_started) || (!_autonotification)) {
    esp3d_log("Auto notification rejected");
    return false;
  }
  String msgtpl = msg;
  // check if has variable to change
  if (msgtpl.indexOf("%") != -1) {
    msgtpl.replace("%ESP_IP%", WiFi.localIP().toString().c_str());
    msgtpl.replace("%ESP_NAME%", NetConfig::hostname());
  }
  if (!sendMSG(ESP_NOTIFICATION_TITLE, msgtpl.c_str())) {
    esp3d_log_e("Auto notification failed");
    return false;
  } else {
    esp3d_log("Auto notification sent");
    return true;
  }
}

NotificationsService::NotificationsService() {
  _started = false;
  _notificationType = 0;
  _token1 = "";
  _token1 = "";
  _settings = "";
}
NotificationsService::~NotificationsService() { end(); }

bool NotificationsService::started() { return _started; }

const char* NotificationsService::getTypeString() {
  switch (_notificationType) {
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
    case ESP_HOMEASSISTANT_NOTIFICATION:
      return "HomeAssistant";
    default:
      break;
  }
  return "none";
}

bool NotificationsService::sendMSG(const char* title, const char* message) {
  if (!_started) {
    esp3d_log_e("Error notification not started");
    return false;
  }
  if (!((strlen(title) == 0) && (strlen(message) == 0))) {
    if (_notificationType != ESP_HOMEASSISTANT_NOTIFICATION) {
      // push to webui by default
      #if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
          String msg = "NOTIFICATION:";
          msg += message;
          websocket_terminal_server.pushMSG(msg.c_str());
      #endif  // HTTP_FEATURE || WS_DATA_FEATURE
      #ifdef DISPLAY_DEVICE
          esp3d_display.setStatus(message);
      #endif  // DISPLAY_DEVICE
    }
    switch (_notificationType) {
      case ESP_PUSHOVER_NOTIFICATION:
        return sendPushoverMSG(title, message);
        break;
      case ESP_EMAIL_NOTIFICATION:
        return sendEmailMSG(title, message);
        break;
      case ESP_LINE_NOTIFICATION:
        return sendLineMSG(title, message);
        break;
      case ESP_TELEGRAM_NOTIFICATION:
        return sendTelegramMSG(title, message);
        break;
      case ESP_IFTTT_NOTIFICATION:
        return sendIFTTTMSG(title, message);
        break;
      case ESP_HOMEASSISTANT_NOTIFICATION:
        return sendHomeAssistantMSG(title, message);
        break;
      default:
        break;
    }
  }
  return true;
}
// Messages are currently limited to 1024 4-byte UTF-8 characters
// but we do not do any check
// TODO: put error in variable to allow better error handling
bool NotificationsService::sendPushoverMSG(const char* title,
                                           const char* message) {
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
#endif  // ARDUINO_ARCH_ESP8266
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log_e("Error connecting  server %s:%d", _serveraddress.c_str(),
                _port);
    return false;
  }
  // build data for post
  data = "user=";
  data += _token1;
  data += "&token=";
  data += _token2;
  data += "&title=";
  data += title;
  data += "&message=";
  data += message;
  data += "&device=";
  data += NetConfig::hostname();
  // build post query
  postcmd =
      "POST /1/messages.json HTTP/1.1\r\nHost: api.pushover.net\r\nConnection: "
      "close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
      "*;q=0.8\r\nContent-Length: ";
  postcmd += data.length();
  postcmd += "\r\n\r\n";
  postcmd += data;
  esp3d_log("Query: %s", postcmd.c_str());
  // send query
  Notificationclient.print(postcmd);
  res = Wait4Answer(Notificationclient, "{", "\"status\":1", PUSHOVERTIMEOUT);
  Notificationclient.stop();
  return res;
}

// Telegram
// TODO: put error in variable to allow better error handling
bool NotificationsService::sendTelegramMSG(const char* title,
                                           const char* message) {
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
#endif  // ARDUINO_ARCH_ESP8266
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log("Error connecting  server %s:%d", _serveraddress.c_str(), _port);
    return false;
  }
  (void)title;
  // build url for get
  data = "chat_id=";
  data += _token2;
  data += "&text=";
  data += message;

  // build post query
  postcmd = "POST /bot";
  postcmd += _token1;
  postcmd +=
      "/sendMessage HTTP/1.1\r\nHost: api.telegram.org\r\nConnection: "
      "close\r\nContent-Type: "
      "application/x-www-form-urlencoded\r\nCache-Control: "
      "no-cache\r\nUser-Agent: ESP3D\r\nAccept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
      "*;q=0.8\r\nContent-Length: ";
  postcmd += data.length();
  postcmd += "\r\n\r\n";
  postcmd += data;
  esp3d_log("Query: %s", postcmd.c_str());
  // send query
  Notificationclient.print(postcmd);
  res = Wait4Answer(Notificationclient, "{", "\"ok\":true", TELEGRAMTIMEOUT);
  Notificationclient.stop();
  return res;
}

// TODO: put error in variable to allow better error handling
bool NotificationsService::sendEmailMSG(const char* title,
                                        const char* message) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  WiFiClientSecure Notificationclient;
#pragma GCC diagnostic pop
  Notificationclient.setInsecure();
#if defined(ARDUINO_ARCH_ESP8266)
  BearSSLSetup(Notificationclient);
#endif  // ARDUINO_ARCH_ESP8266
  esp3d_log("Connect to server");
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log_e("Error connecting  server %s:%d", _serveraddress.c_str(),
                _port);
    return false;
  }
  // Check answer of connection
  if (!Wait4Answer(Notificationclient, "220", "220", EMAILTIMEOUT)) {
    esp3d_log_e("Connection failed!");
    return false;
  }
  // Do HELO
  esp3d_log("HELO");
  Notificationclient.print("HELO friend\r\n");
  if (!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
    esp3d_log_e("HELO failed!");
    return false;
  }
  esp3d_log("AUTH LOGIN");
  // Request AUthentication
  Notificationclient.print("AUTH LOGIN\r\n");
  if (!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
    esp3d_log("AUTH LOGIN failed!");
    return false;
  }
  esp3d_log("Send LOGIN");
  // sent Login
  Notificationclient.printf("%s\r\n", _token1.c_str());
  if (!Wait4Answer(Notificationclient, "334", "334", EMAILTIMEOUT)) {
    esp3d_log_e("Sent login failed!");
    return false;
  }
  esp3d_log("Send PASSWORD");
  // Send password
  Notificationclient.printf("%s\r\n", _token2.c_str());
  if (!Wait4Answer(Notificationclient, "235", "235", EMAILTIMEOUT)) {
    esp3d_log_e("Sent password failed!");
    return false;
  }
  esp3d_log("MAIL FROM");
  // Send From
  Notificationclient.printf("MAIL FROM: <%s>\r\n", _settings.c_str());
  if (!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
    esp3d_log_e("MAIL FROM failed!");
    return false;
  }
  esp3d_log("RCPT TO");
  // Send To
  Notificationclient.printf("RCPT TO: <%s>\r\n", _settings.c_str());
  if (!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
    esp3d_log_e("RCPT TO failed!");
    return false;
  }
  esp3d_log("DATA");
  // Send Data
  Notificationclient.print("DATA\r\n");
  if (!Wait4Answer(Notificationclient, "354", "354", EMAILTIMEOUT)) {
    esp3d_log_e("Preparing DATA failed!");
    return false;
  }
  esp3d_log("Send message");
  // Send message
  Notificationclient.printf("From:ESP3D<%s>\r\n", _settings.c_str());
  Notificationclient.printf("To: <%s>\r\n", _settings.c_str());
  Notificationclient.printf("Subject: %s\r\n\r\n", title);
  Notificationclient.println(message);

  esp3d_log("Send final dot");
  // Send Final dot
  Notificationclient.print(".\r\n");
  if (!Wait4Answer(Notificationclient, "250", "250", EMAILTIMEOUT)) {
    esp3d_log_e("Sending final dot failed!");
    return false;
  }
  esp3d_log("QUIT");
  // Quit
  Notificationclient.print("QUIT\r\n");
  if (!Wait4Answer(Notificationclient, "221", "221", EMAILTIMEOUT)) {
    esp3d_log_e("QUIT failed!");
    return false;
  }

  Notificationclient.stop();
  return true;
}
bool NotificationsService::sendLineMSG(const char* title, const char* message) {
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
#endif  // ARDUINO_ARCH_ESP8266
  (void)title;
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log_e("Error connecting  server %s:%d", _serveraddress.c_str(),
                _port);
    return false;
  }
  // build data for post
  data = "message=";
  data += message;
  // build post query
  postcmd =
      "POST /api/notify HTTP/1.1\r\nHost: notify-api.line.me\r\nConnection: "
      "close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
      "*;q=0.8\r\nContent-Type: application/x-www-form-urlencoded\r\n";
  postcmd += "Authorization: Bearer ";
  postcmd += _token1 + "\r\n";
  postcmd += "Content-Length: ";
  postcmd += data.length();
  postcmd += "\r\n\r\n";
  postcmd += data;
  esp3d_log("Query: %s", postcmd.c_str());
  // send query
  Notificationclient.print(postcmd);
  res = Wait4Answer(Notificationclient, "{", "\"status\":200", LINETIMEOUT);
  Notificationclient.stop();
  return res;
}

// IFTTT
bool NotificationsService::sendIFTTTMSG(const char* title,
                                        const char* message) {
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
#endif  // ARDUINO_ARCH_ESP8266
  (void)title;
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log_e("Error connecting  server %s:%d", _serveraddress.c_str(),
                _port);
    return false;
  }

  // build data for post

  data = "value1=";
  data += title;
  data += "&value2=";
  data += message;
  data += "&value3=";
  data += NetConfig::hostname();

  // build post query
  postcmd = "POST /trigger/" + _token1 + "/with/key/" + _token2 +
            "  HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: "
            "close\r\nCache-Control: no-cache\r\nUser-Agent: ESP3D\r\nAccept: "
            "text/html,application/xhtml+xml,application/xml;q=0.9,*/"
            "*;q=0.8\r\nContent-Type: "
            "application/x-www-form-urlencoded\r\nContent-Length: ";
  postcmd += data.length();
  postcmd += "\r\n\r\n";
  postcmd += data;

  // esp3d_log("Query: %s", postcmd.c_str());
  // send query
  Notificationclient.print(postcmd);
  res = Wait4Answer(Notificationclient, "Congratulations", "Congratulations",
                    IFTTTTIMEOUT);
  Notificationclient.stop();
  return res;
}

// Home Assistant
bool NotificationsService::sendHomeAssistantMSG(const char* title,
                                        const char* message) {
  WiFiClient Notificationclient;
  (void)title;
  if (!Notificationclient.connect(_serveraddress.c_str(), _port)) {
    esp3d_log_e("Error connecting  server %s:%d", _serveraddress.c_str(),
                _port);
    return false;
  }
  String tmp = message;
  int pos = tmp.indexOf('#');
  if (pos == -1) return false;
  String path = tmp.substring(0, pos);
  String json = tmp.substring(pos + 1);
  // build post query
  String postcmd = "POST " + path + "  HTTP/1.1\r\n"
            "Host: " + _serveraddress.c_str() + "\r\n"
            "Connection: close\r\n"
            "Cache-Control: no-cache\r\n"
            "User-Agent: ESP3D\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Authorization: Bearer " + _token1 + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + json.length() + "\r\n"
            "\r\n" + json;

  // esp3d_log("Query: %s", postcmd.c_str());
  // send query
  Notificationclient.print(postcmd);
  bool res = Wait4Answer(Notificationclient, "200 OK", "200 OK", HOMEASSISTANTTIMEOUT);
  Notificationclient.stop();
  return res;
}

// Email#serveraddress:port or serveraddress:port
bool NotificationsService::getPortFromSettings() {
  String tmp = ESP3DSettings::readString(ESP_NOTIFICATION_SETTINGS);
  int pos = tmp.lastIndexOf(':');
  if (pos == -1) {
    return false;
  }
  _port = tmp.substring(pos + 1).toInt();
  esp3d_log("port : %d", _port);
  if (_port > 0) {
    return true;
  } else {
    return false;
  }
}
// Email#serveraddress:port or serveraddress:port
bool NotificationsService::getServerAddressFromSettings() {
  String tmp = ESP3DSettings::readString(ESP_NOTIFICATION_SETTINGS);
  int pos1 = tmp.indexOf('#'); // The "#" is optional
  int pos2 = tmp.lastIndexOf(':');
  if (pos2 == -1) return false;
  _serveraddress = tmp.substring(pos1 + 1, pos2);
  esp3d_log("server : %s", _serveraddress.c_str());
  return true;
}
// Email#serveraddress:port
bool NotificationsService::getEmailFromSettings() {
  String tmp = ESP3DSettings::readString(ESP_NOTIFICATION_SETTINGS);
  int pos = tmp.indexOf('#');
  if (pos == -1) return false;
  _settings = tmp.substring(0, pos);
  esp3d_log("email : %s", _settings.c_str());
  // TODO add a check for valid email ?
  return true;
}

bool NotificationsService::decode64(const char* encodedURL, char* decodedURL) {
  size_t out_len = 0;
  out_len = base64_decode_chars(encodedURL, strlen(encodedURL), decodedURL);
  esp3d_log("URLE: %s", encodedURL);
  esp3d_log("URLD: %s", decodedURL);
  return (out_len > 0);
}

bool NotificationsService::GET(const char* URL64) {
  // TODO do we need https client ?
  WiFiClient client;
  HTTPClient http;  // must be declared after WiFiClient for correct destruction
                    // order, because used by http.begin(client,...)
  char* decodedurl[255];
  bool res = false;
  if (decode64(URL64, (char*)decodedurl)) {
    http.begin(client, (const char*)decodedurl);
    int httpCode = http.GET();
    esp3d_log("HTTP code: %d", httpCode);
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        res = true;
      }
    }
    http.end();
  }
  return res;
}

bool NotificationsService::begin() {
  bool res = true;
  end();
  _notificationType = ESP3DSettings::readByte(ESP_NOTIFICATION_TYPE);
  switch (_notificationType) {
    case 0:  // no notification = no error but no start
      _started = true;
      return true;
    case ESP_PUSHOVER_NOTIFICATION:
      _token1 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1);
      _token2 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN2);
      _port = PUSHOVERPORT;
      _serveraddress = PUSHOVERSERVER;
      break;
    case ESP_TELEGRAM_NOTIFICATION:
      _token1 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1);
      _token2 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN2);
      _port = TELEGRAMPORT;
      _serveraddress = TELEGRAMSERVER;
      break;
    case ESP_LINE_NOTIFICATION:
      _token1 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1);
      _port = LINEPORT;
      _serveraddress = LINESERVER;
      break;
    case ESP_IFTTT_NOTIFICATION:
      _token1 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1);
      _token2 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN2);
      _port = IFTTTPORT;
      _serveraddress = IFTTTSERVER;
      break;
    case ESP_HOMEASSISTANT_NOTIFICATION:
      _token1 = ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1);
      if (!getPortFromSettings() || !getServerAddressFromSettings()) {
        return false;
      }
      break;
    case ESP_EMAIL_NOTIFICATION:
      _token1 =
          base64::encode(ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1));
      _token2 =
          base64::encode(ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN2));
      // esp3d_log("%s",ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN1));
      // esp3d_log("%s",ESP3DSettings::readString(ESP_NOTIFICATION_TOKEN2));
      if (!getEmailFromSettings() || !getPortFromSettings() ||
          !getServerAddressFromSettings()) {
        return false;
      }
      break;
    default:
      return false;
      break;
  }
  _autonotification =
      (ESP3DSettings::readByte(ESP_AUTO_NOTIFICATION) == 0) ? false : true;
  if (!res) {
    end();
  }
  _started = res;
  return _started;
}
void NotificationsService::end() {
  if (!_started) {
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

void NotificationsService::handle() {
  if (_started) {
  }
}

#endif  // NOTIFICATION_FEATURE
