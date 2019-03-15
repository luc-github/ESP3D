/*
  notifications_service.h -  notifications service functions class

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



#ifndef _NOTIFICATIONS_SERVICE_H
#define _NOTIFICATIONS_SERVICE_H


class NotificationsService
{
public:
    NotificationsService();
    ~NotificationsService();
    bool begin();
    void end();
    void handle();
    bool sendMSG(const char * title, const char * message);
    const char * getTypeString();
    bool started();
private:
    bool _started;
    uint8_t _notificationType;
    String _token1;
    String _token2;
    String _settings;
    String _serveraddress;
    uint16_t _port;
    bool sendPushoverMSG(const char * title, const char * message);
    bool sendEmailMSG(const char * title, const char * message);
    bool sendLineMSG(const char * title, const char * message);
    bool getPortFromSettings();
    bool getServerAddressFromSettings();
    bool getEmailFromSettings();
};

extern NotificationsService notificationsservice;

#endif //_NOTIFICATIONS_SERVICE_H

