/*
  mDNS.h - ESP3D mDNS encapsulation class

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
#ifndef _MDNS_H
#define _MDNS_H
#include <Arduino.h>
class mDNS_Service
{
public:
    mDNS_Service();
    ~mDNS_Service();
    bool started()
    {
        return _started;
    }
    bool begin(const char * hostname);
    void end();
    void handle();
    void addESP3DServices(uint16_t port);
    uint16_t servicesCount();
    const char* answerHostname(uint16_t index);
    const char* answerIP(uint16_t index);
    uint16_t answerPort(uint16_t index);
    uint16_t answerTxtCount(uint16_t index);
    const char* answerTxtKey(uint16_t index, uint16_t txtIndex);
    const char* answerTxt(uint16_t index, uint16_t txtIndex);
private:
    bool _started;
    uint16_t _port;
    uint16_t _currentQueryCount;
    uint16_t _currentQueryTxtCount;
    String _hostname;
#if defined(ARDUINO_ARCH_ESP8266)
    const void* _hMDNSServiceQuery;
#endif //ARDUINO_ARCH_ESP8266
};
extern mDNS_Service esp3d_mDNS;
#endif //_MDNS_H
