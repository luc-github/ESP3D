/*
   mDNS.cpp - ESP3D mDNS encapsulation class

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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"

#ifdef MDNS_FEATURE

#include "mDNS.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#if defined( ARDUINO_ARCH_ESP8266)
#include <ESP8266mDNS.h>
#endif //ARDUINO_ARCH_ESP8266
#if defined( ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#endif //ARDUINO_ARCH_ESP32
mDNS_Service esp3d_mDNS;


#define MDNS_SERVICE_NAME "esp3d"
#define MDNS_SERVICE_TYPE "tcp"


mDNS_Service::mDNS_Service()
{
    _started=false;
    _hostname="";
    _port =0;
    _currentQueryCount =0;
    _currentQueryTxtCount =0;
#if defined(ARDUINO_ARCH_ESP8266)
    _hMDNSServiceQuery = 0;
#endif //ARDUINO_ARCH_ESP8266
}

bool mDNS_Service::begin(const char * hostname)
{
    if(_started) {
        end();
    }
    if(WiFi.getMode() != WIFI_AP) {
        _hostname =hostname;
        _hostname.toLowerCase();
        ESP3DOutput output(ESP_ALL_CLIENTS);
        log_esp3d("Start mdsn for %s", _hostname.c_str());
        if (!MDNS.begin(_hostname.c_str())) {
            output.printERROR("mDNS failed to start");
            _started =false;
        } else {
            String stmp = "mDNS started with '" + _hostname + ".local'";
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
            _started =true;
        }
    }
    return _started;
}
void mDNS_Service::end()
{
    _currentQueryCount =0;
    _currentQueryTxtCount =0;
    if(!_started || WiFi.getMode() == WIFI_AP) {
        return;
    }
    _started =false;
#if defined(ARDUINO_ARCH_ESP8266)

    if(_hMDNSServiceQuery) {
        log_esp3d("Remove mdns service for %s", _hostname.c_str());
        if (!MDNS.removeServiceQuery(_hMDNSServiceQuery)) {
            log_esp3d("failed");
        }
    }
    _hMDNSServiceQuery = 0;
    log_esp3d("Remove mdns for %s", _hostname.c_str());
    if (!MDNS.removeService(_hostname.c_str(),MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE)) {
        log_esp3d("failed");
    }
#endif // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    mdns_service_remove("_" MDNS_SERVICE_NAME, "_" MDNS_SERVICE_TYPE);
#endif // ARDUINO_ARCH_ESP32
    MDNS.end();
    _hostname="";
    _port =0;
}
#if defined(ARDUINO_ARCH_ESP8266)
//call back function for mDNS service query
//currently not used, but necessary to setup the service query
void MDNSServiceQueryCallback(MDNSResponder::MDNSServiceInfo serviceInfo, MDNSResponder::AnswerType answerType, bool p_bSetContent) {}
#endif //ARDUINO_ARCH_ESP8266
void mDNS_Service::addESP3DServices(uint16_t port)
{
    _port = port;
    if(WiFi.getMode() == WIFI_AP) {
        return;
    }
    MDNS.addService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, _port);
    MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "firmware", ESP3D_CODE_BASE);
    MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "version", FW_VERSION);
#if defined(ARDUINO_ARCH_ESP8266)
    _hMDNSServiceQuery = MDNS.installServiceQuery(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE,MDNSServiceQueryCallback);
    if (_hMDNSServiceQuery) {
        log_esp3d("MDNS Service query services installed.");
    } else {
        log_esp3d("MDNS Service query services installation failed.");
    }
#endif //ARDUINO_ARCH_ESP8266   
}

void mDNS_Service::handle()
{
#if defined(ARDUINO_ARCH_ESP8266)
    if(WiFi.getMode() == WIFI_AP) {
        return;
    }
    MDNS.update();
#endif //ARDUINO_ARCH_ESP8266
}

uint16_t mDNS_Service::servicesCount()
{
    _currentQueryCount =0;
    if(WiFi.getMode() == WIFI_AP) {
        return _currentQueryCount;
    }
#if defined(ARDUINO_ARCH_ESP32)
    _currentQueryCount = MDNS.queryService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE);
#endif // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
    if(_hMDNSServiceQuery) {
        _currentQueryCount =MDNS.answerCount(_hMDNSServiceQuery);
    }
#endif //ARDUINO_ARCH_ESP8266
    return _currentQueryCount;
}

const char* mDNS_Service::answerHostname(uint16_t index)
{
    static String tmp;
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount) {
        return "";
    }
#if defined(ARDUINO_ARCH_ESP32)
    tmp = MDNS.hostname(index);

#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    tmp = MDNS.answerHostDomain(_hMDNSServiceQuery,index);
#endif // ARDUINO_ARCH_ESP8266 
    return tmp.c_str();
}

const char * mDNS_Service::answerIP(uint16_t index)
{
    static String tmp;
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount) {
        return "";
    }
#if defined(ARDUINO_ARCH_ESP32)
    tmp = MDNS.IP(index).toString();

#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    tmp = MDNS.answerIP4Address(_hMDNSServiceQuery,index,0).toString();
#endif // ARDUINO_ARCH_ESP8266 
    return tmp.c_str();
}

uint16_t mDNS_Service::answerPort(uint16_t index)
{
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount) {
        return 0;
    }
#if defined(ARDUINO_ARCH_ESP32)
    return MDNS.port(index);
#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    return MDNS.answerPort(_hMDNSServiceQuery,index);
#endif // ARDUINO_ARCH_ESP8266 
}

uint16_t mDNS_Service::answerTxtCount(uint16_t index)
{
    _currentQueryTxtCount =0;
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount) {

        return _currentQueryTxtCount;
    }
#if defined(ARDUINO_ARCH_ESP32)
    _currentQueryTxtCount= MDNS.numTxt(index);
    return _currentQueryTxtCount;
#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    if (!_hMDNSServiceQuery || !MDNS.hasAnswerTxts(_hMDNSServiceQuery, index)) {
        return _currentQueryTxtCount;
    }
    String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
    for (uint i = 0; i < txt.length(); i++) {
        if (txt[i] == ';') {
            _currentQueryTxtCount++;
        }
    }
    //there are n+1 number of ';'
    _currentQueryTxtCount++;
    return _currentQueryTxtCount;
#endif // ARDUINO_ARCH_ESP8266 
}

const char* mDNS_Service::answerTxtKey(uint16_t index, uint16_t txtIndex)
{
    static String tmp;
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount || txtIndex>=_currentQueryTxtCount || _currentQueryTxtCount==0 ) {
        return "";
    }
#if defined(ARDUINO_ARCH_ESP32)
    tmp = MDNS.txtKey(index, txtIndex);
    return tmp.c_str();
#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
    log_esp3d("txt: %s", txt.c_str());
    String keyValue = "";
    bool found = false;
    if (txt.indexOf(";") == -1) {
        keyValue = txt;
    } else {
        uint currentIndex = 0;
        uint pos = 0;
        while (!found) {
            int posend = txt.indexOf(";", pos);
            if (posend==-1) {
                posend=txt.length();
            }
            keyValue = txt.substring(pos, posend);
            if (currentIndex == txtIndex) {
                found = true;
            } else {
                pos=posend+1;
                currentIndex++;
            }
        }
    }
    for(uint p=0; p<keyValue.length(); p++) {
        if(keyValue[p] == '=') {
            tmp = keyValue.substring(0,p);
        }
    }
    return tmp.c_str();
#endif // ARDUINO_ARCH_ESP8266 
}

const char* mDNS_Service::answerTxt(uint16_t index, uint16_t txtIndex)
{
    static String tmp;
    if(WiFi.getMode() == WIFI_AP || _currentQueryCount==0 || index>=_currentQueryCount || txtIndex>=_currentQueryTxtCount || _currentQueryTxtCount==0 ) {
        return "";
    }
#if defined(ARDUINO_ARCH_ESP32)
    tmp = MDNS.txt(index, txtIndex);
    return tmp.c_str();
#endif // ARDUINO_ARCH_ESP32 
#if defined(ARDUINO_ARCH_ESP8266)
    String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
    log_esp3d("txt: %s", txt.c_str());
    String keyValue = "";
    bool found = false;
    if (txt.indexOf(";") == -1) {
        keyValue = txt;
    } else {
        uint currentIndex = 0;
        uint pos = 0;
        while (!found) {
            int posend = txt.indexOf(";",pos);
            if (posend==-1) {
                posend=txt.length();
            }
            keyValue = txt.substring(pos, posend);
            if (currentIndex == txtIndex) {
                found = true;
            } else {
                pos=posend+1;
                currentIndex++;
            }
        }
    }
    for(uint p=0; p<keyValue.length(); p++) {
        if(keyValue[p] == '=') {
            tmp = keyValue.substring(p+1,keyValue.length());
        }
    }
    return tmp.c_str();
#endif // ARDUINO_ARCH_ESP8266 
}

mDNS_Service::~mDNS_Service()
{
    end();
}

#endif //MDNS_FEATURE
