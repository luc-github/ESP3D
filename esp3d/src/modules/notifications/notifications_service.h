/*
  notifications_service.h -  notifications service functions class

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



#ifndef _NOTIFICATIONS_SERVICE_H
#define _NOTIFICATIONS_SERVICE_H

#include <WiFiClientSecure.h>


class NotificationsService
{
public:
    NotificationsService();
    ~NotificationsService();
    bool begin();
    void end();
    void handle();
    bool sendMSG(const char * title, const char * message);
    bool GET(const char * URL64);
    const char * getTypeString();
    bool started();
    bool isAutonotification()
    {
        return _autonotification;
    };
    void setAutonotification(bool value)
    {
        _autonotification = value;
    };
    bool sendAutoNotification(const char * msg);
private:
    bool _started;
    bool _autonotification;
    uint8_t _notificationType;
    String _token1;
    String _token2;
    String _settings;
    String _serveraddress;
    uint16_t _port;
#if defined(ARDUINO_ARCH_ESP8266)
    void BearSSLSetup(WiFiClientSecure & Notificationclient);
#endif//ARDUINO_ARCH_ESP8266
    bool decode64(const char* encodedURL, char *decodedURL);
    bool sendPushoverMSG(const char * title, const char * message);
    bool sendEmailMSG(const char * title, const char * message);
    bool sendLineMSG(const char * title, const char * message);
    bool sendTelegramMSG(const char * title, const char * message);
    bool sendIFTTTMSG(const char * title, const char * message);
    bool getPortFromSettings();
    bool getServerAddressFromSettings();
    bool getEmailFromSettings();
    bool Wait4Answer(WiFiClientSecure & client, const char * linetrigger, const char * expected_answer,  uint32_t timeout);
};

extern NotificationsService notificationsservice;

#endif //_NOTIFICATIONS_SERVICE_H

