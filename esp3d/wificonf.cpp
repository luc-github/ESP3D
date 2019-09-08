/*
  wificonf.cpp - ESP3D configuration class

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
#include "config.h"
#include "wificonf.h"
#include "espcom.h"
#include "webinterface.h"

#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
#else
#include <WiFi.h>
#include "esp_wifi.h"
#ifdef MDNS_FEATURE
#include <ESPmDNS.h>
#endif
#endif
#include "IPAddress.h"
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
DNSServer dnsServer;
const byte DNS_PORT = 53;
#endif
#ifdef SSDP_FEATURE
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266SSDP.h>
#else
#include <ESP32SSDP.h>
#endif
#endif
#ifdef NETBIOS_FEATURE
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266NetBIOS.h>
#else
#include <ESP32NetBIOS.h>
#endif
#endif
#ifdef ARDUINO_ARCH_ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#ifdef TIMESTAMP_FEATURE
#include <time.h>
#endif

#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#endif

#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#endif

#if defined (ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif

#if defined(TIMESTAMP_FEATURE) && defined(ARDUINO_ARCH_ESP8266)
void dateTime (uint16_t* date, uint16_t* dtime)
{
    struct tm  tmstruct;
    time_t now;
    time (&now);
    localtime_r (&now, &tmstruct);
    *date = FAT_DATE ( (tmstruct.tm_year) + 1900, ( tmstruct.tm_mon) + 1, tmstruct.tm_mday);
    *dtime = FAT_TIME (tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
}
#endif

WIFI_CONFIG::WIFI_CONFIG()
{
    iweb_port = DEFAULT_WEB_PORT;
    idata_port = DEFAULT_DATA_PORT;
    baud_rate = DEFAULT_BAUD_RATE;
    sleep_mode = DEFAULT_SLEEP_MODE;
    _hostname[0] = 0;
    WiFi_on = true;
}

int32_t WIFI_CONFIG::getSignal (int32_t RSSI)
{
    if (RSSI <= -100) {
        return 0;
    }
    if (RSSI >= -50) {
        return 100;
    }
    return (2 * (RSSI + 100) );
}

const char * WIFI_CONFIG::get_hostname()
{
    String hname;
#ifdef ARDUINO_ARCH_ESP8266
    hname = WiFi.hostname();
#else
    hname = WiFi.getHostname();
#endif
    if (hname.length() == 0) {
        if (!CONFIG::read_string (EP_HOSTNAME, _hostname, MAX_HOSTNAME_LENGTH) ) {
            strcpy (_hostname, get_default_hostname() );
        }
    } else {
        strcpy (_hostname, hname.c_str() );
    }
    return _hostname;
}

const char * WIFI_CONFIG::get_default_hostname()
{
    static char hostname[33];
    uint8_t mac [WL_MAC_ADDR_LENGTH];
    WiFi.macAddress (mac);
#if defined (ESP_HOST_NAME)
    strcpy (hostname, ESP_DEFAULT_NAME);
#else
    if (0 > sprintf (hostname, "ESP_%02X%02X%02X", mac[3], mac[4], mac[5]) ) {
        strcpy (hostname, ESP_DEFAULT_NAME);
    }
#endif
    return hostname;
}

//safe setup if no connection
void  WIFI_CONFIG::Safe_Setup()
{
#ifdef CAPTIVE_PORTAL_FEATURE
    dnsServer.stop();
    CONFIG::wait(100);
#endif

    WiFi.disconnect();
    //setup Soft AP
    WiFi.mode (WIFI_AP);
    IPAddress local_ip (DEFAULT_IP_VALUE[0], DEFAULT_IP_VALUE[1], DEFAULT_IP_VALUE[2], DEFAULT_IP_VALUE[3]);
    IPAddress gateway (DEFAULT_GATEWAY_VALUE[0], DEFAULT_GATEWAY_VALUE[1], DEFAULT_GATEWAY_VALUE[2], DEFAULT_GATEWAY_VALUE[3]);
    IPAddress subnet (DEFAULT_MASK_VALUE[0], DEFAULT_MASK_VALUE[1], DEFAULT_MASK_VALUE[2], DEFAULT_MASK_VALUE[3]);
    String ssid = FPSTR (DEFAULT_AP_SSID);
    String pwd = FPSTR (DEFAULT_AP_PASSWORD);
    WiFi.softAP (ssid.c_str(), pwd.c_str() );
    CONFIG::wait (500);
    WiFi.softAPConfig ( local_ip,  gateway,  subnet);
    CONFIG::wait (1000);
#ifdef ESP_OLED_FEATURE
    OLED_DISPLAY::display_signal(100);
    OLED_DISPLAY::setCursor(0, 0);
    ESPCOM::print(ssid.c_str(), OLED_PIPE);
    OLED_DISPLAY::setCursor(0, 16);
    ESPCOM::print(local_ip.toString().c_str(), OLED_PIPE);
#endif
    ESPCOM::println (F ("Safe mode started"), PRINTER_PIPE);
}


//wifi event
void onWiFiEvent(WiFiEvent_t event)
{

    switch (event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
#ifndef MKS_TFT_FEATURE
        ESPCOM::println (F ("Connected"), PRINTER_PIPE);
#endif
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::display_signal(wifi_config.getSignal (WiFi.RSSI ()));
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print("", OLED_PIPE);
#endif
        break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
        ESPCOM::println (F ("Disconnected"), PRINTER_PIPE);
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::display_signal(-1);
        OLED_DISPLAY::setCursor(0, 16);
        ESPCOM::print("", OLED_PIPE);
        OLED_DISPLAY::setCursor(0, 48);
#endif
        break;
    case WIFI_EVENT_STAMODE_GOT_IP:
#ifndef MKS_TFT_FEATURE
        ESPCOM::println (WiFi.localIP().toString().c_str(), PRINTER_PIPE);
#endif
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::setCursor(0, 16);
        ESPCOM::print(WiFi.localIP().toString().c_str(), OLED_PIPE);
        OLED_DISPLAY::setCursor(0, 48);
        ESPCOM::print("", OLED_PIPE);
#endif
        break;
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
        ESPCOM::println (F ("New client"), PRINTER_PIPE);
        break;
#ifdef ARDUINO_ARCH_ESP32
    case SYSTEM_EVENT_STA_LOST_IP:
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        break;
#endif
    default:
        break;
    }

}

//Read configuration settings and apply them
bool WIFI_CONFIG::Setup (bool force_ap)
{
    char pwd[MAX_PASSWORD_LENGTH + 1];
    char sbuf[MAX_SSID_LENGTH + 1];
    char hostname [MAX_HOSTNAME_LENGTH + 1];
    //int wstatus;
    IPAddress currentIP;
    byte bflag = 0;
    byte bmode = 0;
#ifdef ARDUINO_ARCH_ESP8266
    WiFi.onEvent(onWiFiEvent, WIFI_EVENT_ANY);
#else
    WiFi.onEvent(onWiFiEvent);
#endif
    //system_update_cpu_freq(SYS_CPU_160MHZ);
    //set the sleep mode
    if (!CONFIG::read_byte (EP_SLEEP_MODE, &bflag ) ) {
        LOG ("Error read Sleep mode\r\n")
        return false;
    }
#ifdef ESP_OLED_FEATURE
    OLED_DISPLAY::clear_lcd();
#endif

    sleep_mode = bflag;
    if (force_ap) {
        bmode = AP_MODE;
    } else {
        //AP or client ?
        if (!CONFIG::read_byte (EP_WIFI_MODE, &bmode ) ) {
            LOG ("Error read wifi mode\r\n")
            return false;
        }
    }
    if (!CONFIG::read_string (EP_HOSTNAME, hostname, MAX_HOSTNAME_LENGTH) ) {
        strcpy (hostname, get_default_hostname() );
    }
    //this is AP mode
    if (bmode == AP_MODE) {
        LOG ("Set AP mode\r\n")
        if (!CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH) ) {
            return false;
        }
        if (!CONFIG::read_string (EP_AP_PASSWORD, pwd, MAX_PASSWORD_LENGTH) ) {
            return false;
        }
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::display_signal(100);
        OLED_DISPLAY::setCursor(0, 0);

        ESPCOM::print(sbuf, OLED_PIPE);
#else
#ifndef MKS_TFT_FEATURE
        ESPCOM::println (sbuf, PRINTER_PIPE);
#endif
#endif
        LOG ("SSID ")
        LOG (sbuf)
        LOG ("\r\n")
        //DHCP or Static IP ?
        if (!CONFIG::read_byte (EP_AP_IP_MODE, &bflag ) ) {
            LOG ("Error IP mode\r\n")
            return false;
        }
        LOG ("IP Mode: ")
        LOG (CONFIG::intTostr (bflag) )
        LOG ("\r\n")
        if (bflag == STATIC_IP_MODE) {
            byte ip_buf[4];
            LOG ("Static mode\r\n")
            //get the IP
            LOG ("IP value:")
            if (!CONFIG::read_buffer (EP_AP_IP_VALUE, ip_buf, IP_LENGTH) ) {
                LOG ("Error\r\n")
                return false;
            }
            IPAddress local_ip (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            LOG (local_ip.toString() )
            LOG ("\r\nGW value:")
            //get the gateway
            if (!CONFIG::read_buffer (EP_AP_GATEWAY_VALUE, ip_buf, IP_LENGTH) ) {
                LOG ("Error\r\n")
                return false;
            }
            IPAddress gateway (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            LOG (gateway.toString() )
            LOG ("\r\nMask value:")
            //get the mask
            if (!CONFIG::read_buffer (EP_AP_MASK_VALUE, ip_buf, IP_LENGTH) ) {
                LOG ("Error Mask value\r\n")
                return false;
            }
            IPAddress subnet (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            LOG (subnet.toString() )
            LOG ("\r\n")
            //apply according active wifi mode
            LOG ("Set IP\r\n")
            WiFi.softAPConfig ( local_ip,  gateway,  subnet);
            delay (100);
        }
        LOG ("Disable STA\r\n")
        WiFi.enableSTA (false);
        delay (100);
        LOG ("Set phy mode\r\n")
        //setup PHY_MODE
        if (!CONFIG::read_byte (EP_AP_PHY_MODE, &bflag ) ) {
            return false;
        }
        LOG ("Set AP\r\n")
        //setup Soft AP
        WiFi.mode (WIFI_AP);
#ifdef ARDUINO_ARCH_ESP32
        esp_wifi_set_protocol (ESP_IF_WIFI_AP, bflag);
#endif
        wifi_config.WiFi_on = true;
        delay (50);
        WiFi.softAP (sbuf, pwd);
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::display_signal(100);
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print(sbuf, OLED_PIPE);
#endif
        delay (100);
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode ( (WiFiPhyMode_t) bflag);
#endif
        delay (100);
        LOG ("Get current config\r\n")
        //get current config
#ifdef ARDUINO_ARCH_ESP32
        wifi_config_t conf;
        esp_wifi_get_config (ESP_IF_WIFI_AP, &conf);
#else
        struct softap_config apconfig;
        wifi_softap_get_config (&apconfig);
#endif
        //set the chanel
        if (!CONFIG::read_byte (EP_CHANNEL, &bflag ) ) {
            return false;
        }
#ifdef ARDUINO_ARCH_ESP32
        conf.ap.channel = bflag;
#else
        apconfig.channel = bflag;
#endif
        //set Authentification type
        if (!CONFIG::read_byte (EP_AUTH_TYPE, &bflag ) ) {
            return false;
        }
#ifdef ARDUINO_ARCH_ESP32
        conf.ap.authmode = (wifi_auth_mode_t) bflag;
#else
        apconfig.authmode = (AUTH_MODE) bflag;
#endif
        //set the visibility of SSID
        if (!CONFIG::read_byte (EP_SSID_VISIBLE, &bflag ) ) {
            return false;
        }
#ifdef ARDUINO_ARCH_ESP32
        conf.ap.ssid_hidden = !bflag;
#else
        apconfig.ssid_hidden = !bflag;
#endif

        //no need to add these settings to configuration just use default ones
#ifdef ARDUINO_ARCH_ESP32
        conf.ap.max_connection = DEFAULT_MAX_CONNECTIONS;
        conf.ap.beacon_interval = DEFAULT_BEACON_INTERVAL;
        if (esp_wifi_set_config (ESP_IF_WIFI_AP, &conf) != ESP_OK) {
            ESPCOM::println (F ("Error Wifi AP!"), PRINTER_PIPE);
            delay (1000);
        }
#else
        apconfig.max_connection = DEFAULT_MAX_CONNECTIONS;
        apconfig.beacon_interval = DEFAULT_BEACON_INTERVAL;
        //apply settings to current and to default
        if (!wifi_softap_set_config (&apconfig) || !wifi_softap_set_config_current (&apconfig) ) {
            ESPCOM::println (F ("Error Wifi AP!"), PRINTER_PIPE);
            delay (1000);
        }
#endif
    } else {
        LOG ("Set STA mode\r\n")
        if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
            return false;
        }
        if (!CONFIG::read_string (EP_STA_PASSWORD, pwd, MAX_PASSWORD_LENGTH) ) {
            return false;
        }

#ifndef MKS_TFT_FEATURE
        ESPCOM::println (sbuf, PRINTER_PIPE);
#endif
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::display_signal(-1);
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print(sbuf, OLED_PIPE);
#endif
        LOG ("SSID ")
        LOG (sbuf)
        LOG ("\r\n")
        if (!CONFIG::read_byte (EP_STA_IP_MODE, &bflag ) ) {
            return false;
        }
        if (bflag == STATIC_IP_MODE) {
            byte ip_buf[4];
            //get the IP
            if (!CONFIG::read_buffer (EP_STA_IP_VALUE, ip_buf, IP_LENGTH) ) {
                return false;
            }
            IPAddress local_ip (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            //get the gateway
            if (!CONFIG::read_buffer (EP_STA_GATEWAY_VALUE, ip_buf, IP_LENGTH) ) {
                return false;
            }
            IPAddress gateway (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            //get the mask
            if (!CONFIG::read_buffer (EP_STA_MASK_VALUE, ip_buf, IP_LENGTH) ) {
                return false;
            }
            IPAddress subnet (ip_buf[0], ip_buf[1], ip_buf[2], ip_buf[3]);
            //apply according active wifi mode
            WiFi.config ( local_ip,  gateway,  subnet);
        }
        WiFi.enableAP (false);
        delay (100);
        if (!CONFIG::read_byte (EP_STA_PHY_MODE, &bflag ) ) {
            return false;
        }

        //setup station mode
        WiFi.mode (WIFI_STA);
        wifi_config.WiFi_on = true;
        delay (100);
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode ( (WiFiPhyMode_t) bflag);
#else
        esp_wifi_set_protocol (ESP_IF_WIFI_STA, bflag);
#endif
        if (strlen(pwd) > 0) {
            WiFi.begin (sbuf, pwd);
        } else {
            WiFi.begin (sbuf);
        }
        delay (100);
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setSleepMode ( (WiFiSleepType_t) sleep_mode);
#else
        //for backward compatibility
        if ((wifi_ps_type_t) sleep_mode == WIFI_PS_MAX_MODEM) {
            sleep_mode=WIFI_PS_MIN_MODEM;
        }
        esp_wifi_set_ps ( (wifi_ps_type_t) sleep_mode);
#endif
        delay (100);
        byte i = 0;
        //try to connect
        byte dot = 0;
        String msg;
        int last = -1;
        while (WiFi.status() != WL_CONNECTED && i < 40) {
            switch (WiFi.status() ) {
            case 1:
#ifdef ESP_OLED_FEATURE
                OLED_DISPLAY::display_signal(-1);
#endif
                if ((dot == 0) || last!=WiFi.status()) {
                    msg = F ("No SSID");
                    last=WiFi.status();
                }
                break;

            case 4:
                if ((dot == 0) || last!=WiFi.status()) {
                    msg = F ("No Connection");
                    last=WiFi.status();
                }
                break;

            default:
#ifdef ESP_OLED_FEATURE
                OLED_DISPLAY::display_signal(wifi_config.getSignal (WiFi.RSSI ()));
#endif
                if ((dot == 0) || last!=WiFi.status()) {
                    msg = F ("Connecting");
                    last=WiFi.status();
                }
                break;
            }
            dot++;
            msg.trim();
            msg += F (".");
            //for smoothieware to keep position
            for (byte i = 0; i < 4 - dot; i++) {
                msg += F (" ");
            }
            if (dot == 4) {
                dot = 0;
            }
#ifndef MKS_TFT_FEATURE
#ifndef DISABLE_CONNECTING_MSG
            ESPCOM::println (msg, PRINTER_PIPE);
#endif
#endif

            delay (500);
            i++;
        }
        if (WiFi.status() != WL_CONNECTED) {
#ifndef MKS_TFT_FEATURE
            ESPCOM::println (F ("Not Connected!"), PRINTER_PIPE);
#endif
            return false;
        }
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.hostname (hostname);
#else
        WiFi.setHostname (hostname);
#endif
    }

    //Get IP
    if (WiFi.getMode()== WIFI_AP) {
        currentIP = WiFi.softAPIP();
        ESPCOM::println (currentIP.toString().c_str(), PRINTER_PIPE);
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::setCursor(0, 16);
        ESPCOM::print(currentIP.toString().c_str(), OLED_PIPE);
#endif
    }
#ifdef ESP_OLED_FEATURE
    OLED_DISPLAY::setCursor(0, 48);
    if (force_ap) {
        ESPCOM::print("Safe mode 1", OLED_PIPE);
    } else if ((WiFi.getMode() == WIFI_STA) && (WiFi.status() == WL_CONNECTED)) {
        ESPCOM::print("Connected", OLED_PIPE);
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print(sbuf, OLED_PIPE);
    } else if (WiFi.getMode() != WIFI_STA) {
        ESPCOM::print("AP Ready", OLED_PIPE);
    }
#endif
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
    return true;
}

bool WIFI_CONFIG::Enable_servers()
{
    //start web interface
    web_interface = new WEBINTERFACE_CLASS (wifi_config.iweb_port);
#ifdef AUTHENTICATION_FEATURE
    //here the list of headers to be recorded
    const char * headerkeys[] = {"Cookie"} ;
    size_t headerkeyssize = sizeof (headerkeys) / sizeof (char*);
    //ask server to track these headers
    web_interface->web_server.collectHeaders (headerkeys, headerkeyssize );
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
    if (WiFi.getMode() != WIFI_STA ) {
        // if DNSServer is started with "*" for domain name, it will reply with
        // provided IP to all DNS request
        dnsServer.setErrorReplyCode (DNSReplyCode::NoError);
        dnsServer.start (DNS_PORT, "*", WiFi.softAPIP() );
    }
#endif
    web_interface->web_server.begin();
#ifdef TCP_IP_DATA_FEATURE
    //start TCP/IP interface
    data_server = new WiFiServer (wifi_config.idata_port);
    data_server->begin();
    data_server->setNoDelay (true);
#endif
#if !defined (ASYNCWEBSERVER)
    socket_server = new WebSocketsServer (wifi_config.iweb_port+1);
    socket_server->begin();
    socket_server->onEvent(webSocketEvent);
#endif

#ifdef MDNS_FEATURE
    // Set up mDNS responder:
    //useless in AP mode and service consuming
    if (WiFi.getMode() != WIFI_AP ) {
        char hostname [MAX_HOSTNAME_LENGTH + 1];
        if (!CONFIG::read_string (EP_HOSTNAME, hostname, MAX_HOSTNAME_LENGTH) ) {
            strcpy (hostname, get_default_hostname() );
        }
        if (!mdns.begin (hostname) ) {
            ESPCOM::println (F ("Error with mDNS!"), PRINTER_PIPE);
            delay (1000);
        } else {
            // Check for any mDNS queries and send responses
            delay (100);
            wifi_config.mdns.addService ("http", "tcp", wifi_config.iweb_port);
        }
    }
#endif
#if defined(SSDP_FEATURE) || defined(NETBIOS_FEATURE)
    String shost;
    if (!CONFIG::read_string (EP_HOSTNAME, shost, MAX_HOSTNAME_LENGTH) ) {
        shost = wifi_config.get_default_hostname();
    }
#endif
#ifdef SSDP_FEATURE
    String stmp;
    SSDP.setSchemaURL ("description.xml");
    SSDP.setHTTPPort ( wifi_config.iweb_port);
    SSDP.setName (shost.c_str() );
#if defined(ARDUINO_ARCH_ESP8266)
    stmp = String (ESP.getChipId() );
    SSDP.setModelName (ESP8266_MODEL_NAME);
    SSDP.setModelURL (ESP8266_MODEL_URL);
#else
    stmp =  String ( (uint16_t) (ESP.getEfuseMac() >> 32) );
    SSDP.setModelName (ESP32_MODEL_NAME);
    SSDP.setModelURL (ESP32_MODEL_URL);
#endif
    SSDP.setSerialNumber (stmp.c_str() );
    SSDP.setURL ("/");
    SSDP.setModelNumber (ESP_MODEL_NUMBER);
    SSDP.setManufacturer (ESP_MANUFACTURER_NAME);
    SSDP.setManufacturerURL (ESP_MANUFACTURER_URL);
    SSDP.setDeviceType ("upnp:rootdevice");
    if (WiFi.getMode() != WIFI_AP ) {
        SSDP.begin();
    }
#endif
#ifdef NETBIOS_FEATURE
    //useless in AP mode and service consuming
    if (WiFi.getMode() != WIFI_AP ) {
        NBNS.begin (shost.c_str() );
    }
#endif

#if defined(TIMESTAMP_FEATURE)
    CONFIG::init_time_client();
    if (CONFIG::is_direct_sd) {
#if defined(ARDUINO_ARCH_ESP8266)
        //set callback to get time on files on SD
        SdFile::dateTimeCallback (dateTime);
#endif
    }
#endif
#if defined(NOTIFICATION_FEATURE)
    notificationsservice.begin();
    notificationsservice.sendAutoNotification(NOTIFICATION_ESP_ONLINE);
#endif

    return true;
}

bool WIFI_CONFIG::Disable_servers()
{
#ifdef TCP_IP_DATA_FEATURE
    data_server->stop();
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
    if (WiFi.getMode() != WIFI_STA ) {
        dnsServer.stop();
    }
#endif
#ifdef NETBIOS_FEATURE
    //useless in AP mode and service consuming
    if (WiFi.getMode() != WIFI_AP ) {
        NBNS.end();
    }
#endif
#if defined(NOTIFICATION_FEATURE)
    notificationsservice.end();
#endif
    return true;
}

WIFI_CONFIG wifi_config;
