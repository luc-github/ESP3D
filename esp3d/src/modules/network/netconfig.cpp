/*
  netconfig.cpp -  network functions class

  Copyright (c) 2018 Luc Lebosse. All rights reserved.

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
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined (BLUETOOTH_FEATURE)
#ifdef ARDUINO_ARCH_ESP32
#define WIFI_EVENT_STAMODE_CONNECTED SYSTEM_EVENT_STA_CONNECTED
#define WIFI_EVENT_STAMODE_DISCONNECTED SYSTEM_EVENT_STA_DISCONNECTED
#define WIFI_EVENT_STAMODE_GOT_IP SYSTEM_EVENT_STA_GOT_IP
#define WIFI_EVENT_SOFTAPMODE_STACONNECTED SYSTEM_EVENT_AP_STACONNECTED
#define RADIO_OFF_MSG "Radio Off"
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#define RADIO_OFF_MSG "WiFi Off"
#endif //ARDUINO_ARCH_ESP8266
#include "netconfig.h"
#if defined (WIFI_FEATURE)
#include "../wifi/wificonfig.h"
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
#include "../ethernet/ethconfig.h"
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#include "netservices.h"
#include "../../core/esp3doutput.h"
#include "../../core/settings_esp3d.h"

String NetConfig::_hostname = "";
bool NetConfig::_needReconnect2AP = false;
bool NetConfig::_events_registered = false;
bool NetConfig::_started = false;
uint8_t NetConfig::_mode = ESP_NO_NETWORK;

//just simple helper to convert mac address to string
char * NetConfig::mac2str (uint8_t mac [8])
{
    static char macstr [18];
    if (0 > sprintf (macstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ) {
        strcpy (macstr, "00:00:00:00:00:00");
    }
    return macstr;
}


/**
 * Helper to convert  IP string to int
 */
uint32_t NetConfig::IP_int_from_string(const char * s)
{
    uint32_t ip_int = 0;
    IPAddress ipaddr;
    if (ipaddr.fromString(s)) {
        ip_int = ipaddr;
    }
    return ip_int;
}

/**
 * Helper to convert int to IP string
 */
String NetConfig::IP_string_from_int(uint32_t ip_int)
{
    IPAddress ipaddr(ip_int);
    return ipaddr.toString();
}

/**
 * Check if Hostname string is valid
 */

bool NetConfig::isHostnameValid (const char * hostname)
{
    //limited size
    char c;
    if (strlen (hostname) > MAX_HOSTNAME_LENGTH || strlen (hostname) < MIN_HOSTNAME_LENGTH) {
        return false;
    }
    //only letter and digit
    for (uint i = 0; i < strlen (hostname); i++) {
        c = hostname[i];
        if (! (isdigit (c) || isalpha (c) || c == '-') ) {
            return false;
        }
        if (c == ' ') {
            return false;
        }
    }
    return true;
}


/**
 * Get IP Integer what ever is enabled
 */
IPAddress  NetConfig::localIPAddress()
{
    IPAddress current_ip = IPAddress(0,0,0,0);
#if defined( WIFI_FEATURE)
    if (WiFi.getMode() == WIFI_STA) {
        current_ip = WiFi.localIP();
    } else if (WiFi.getMode() == WIFI_AP) {
        current_ip = WiFi.softAPIP();
    }
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
    if (EthConfig::started()) {
        current_ip = ETH.localIP();
    }
#endif //ETH_FEATURE

    return current_ip;
}

/**
 * Get IP string what ever is enabled
 */
String NetConfig::localIP()
{
    static String currentIP = "";
#if defined( WIFI_FEATURE)
    if (WiFi.getMode() == WIFI_STA) {
        currentIP = WiFi.localIP().toString();
    } else if (WiFi.getMode() == WIFI_AP) {
        currentIP = WiFi.softAPIP().toString();
    }
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
    if (EthConfig::started()) {
        currentIP = ETH.localIP().toString();
    }
#endif //ETH_FEATURE
    if (currentIP.length() == 0) {
        currentIP = "0.0.0.0";
    }
    return currentIP;
}

/**
 * Check if IP string is valid
 */

bool NetConfig::isValidIP(const char * string)
{
    IPAddress ip;
    return ip.fromString(string);
}


//wifi event
void NetConfig::onWiFiEvent(WiFiEvent_t event)
{
    ESP3DOutput output(ESP_ALL_CLIENTS);
    switch (event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
        _needReconnect2AP = false;
        break;
    case WIFI_EVENT_STAMODE_DISCONNECTED: {
        if(_started) {
            output.printMSG ("Disconnected");
            //_needReconnect2AP = true;
        }
    }
    break;
    case WIFI_EVENT_STAMODE_GOT_IP: {
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
        output.printMSG (WiFi.localIP().toString().c_str());
#endif //#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    }
    break;
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED: {
        output.printMSG ("New client");
    }
    break;
#ifdef ARDUINO_ARCH_ESP32
    case SYSTEM_EVENT_STA_LOST_IP:
        if(_started) {
            _needReconnect2AP = true;
        }
        break;
#ifdef ETH_FEATURE
    case SYSTEM_EVENT_ETH_CONNECTED: {
        output.printMSG ("Cable connected");
    }
    break;
    case SYSTEM_EVENT_ETH_DISCONNECTED: {
        output.printMSG ("Cable disconnected");
    }
    break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        output.printMSG (ETH.localIP().toString().c_str());
        break;
#endif //ETH_FEATURE
#endif //ARDUINO_ARCH_ESP32
    default:
        break;
    }
}

void NetConfig::setMode(uint8_t mode)
{
    _mode=mode;
}

uint8_t NetConfig::getMode()
{
    return _mode;
}

/**
 * begin WiFi setup
 */
bool NetConfig::begin()
{
    bool res = false;
    //clear everything
    end();
    int8_t espMode =Settings_ESP3D::read_byte(ESP_RADIO_MODE);
    ESP3DOutput output(ESP_ALL_CLIENTS);
    log_esp3d("Starting Network");
    if (espMode != ESP_NO_NETWORK) {
        if (Settings_ESP3D::isVerboseBoot()) {
            output.printMSG("Starting Network");
        }
    }
    //setup events
    if(!_events_registered) {
#ifdef ARDUINO_ARCH_ESP8266
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        WiFi.onEvent(NetConfig::onWiFiEvent, WIFI_EVENT_ANY);
#pragma GCC diagnostic pop
#endif
#ifdef ARDUINO_ARCH_ESP32
        WiFi.onEvent(NetConfig::onWiFiEvent);

#endif
        _events_registered = true;
    }
    //Get hostname
    _hostname = Settings_ESP3D::read_string(ESP_HOSTNAME);
    _mode = espMode;
    if (espMode == ESP_NO_NETWORK) {
        output.printMSG("Disable Network");
        WiFi.mode(WIFI_OFF);
        ESP3DOutput::toScreen(ESP_OUTPUT_IP_ADDRESS,nullptr);
        if (Settings_ESP3D::isVerboseBoot()) {
            ESP3DOutput output(ESP_ALL_CLIENTS);
            output.printMSG(RADIO_OFF_MSG);
            output.flush();
        }
        return true;
    }
#if defined (WIFI_FEATURE)
    if ((espMode == ESP_AP_SETUP) || (espMode == ESP_WIFI_AP) || (espMode == ESP_WIFI_STA)) {
        output.printMSG("Setup wifi");
        res = WiFiConfig::begin(espMode);
    }
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
    //if ((espMode == ESP_ETH_STA) || (espMode == ESP_ETH_SRV)) {
    if ((espMode == ESP_ETH_STA)) {
        WiFi.mode(WIFI_OFF);
        res = EthConfig::begin(espMode);
    }
#endif //ETH_FEATURE

#if defined (BLUETOOTH_FEATURE)
    if (espMode == ESP_BT) {
        WiFi.mode(WIFI_OFF);
        String msg = "BT On";
        ESP3DOutput::toScreen(ESP_OUTPUT_STATUS, msg.c_str());
        res = bt_service.begin();
    }
#else
    //if BT and no BT enabled let's go to no network
    if (espMode == ESP_BT) {
        espMode = ESP_NO_NETWORK;
    }
#endif //BLUETOOTH_FEATURE

    if (espMode == ESP_NO_NETWORK) {
        output.printMSG("Disable Network");
        WiFi.mode(WIFI_OFF);
        ESP3DOutput::toScreen(ESP_OUTPUT_IP_ADDRESS,nullptr);
        if (Settings_ESP3D::isVerboseBoot()) {
            ESP3DOutput output(ESP_ALL_CLIENTS);
            output.printMSG(RADIO_OFF_MSG);
            output.flush();
        }
        return true;
    }
    //if network is up, let's start services
    if (res) {
        _started = true;
        bool start_services = false;
#if defined (ETH_FEATURE)
        if (EthConfig::started()) {
            start_services = true;
        }
#endif //ETH_FEATURE
#if defined (WIFI_FEATURE)
        if (WiFiConfig::started()) {
            start_services = true;
        }
#endif //WIFI_FEATURE
        if (start_services) {
            log_esp3d("Starting service");
            res = NetServices::begin();
        }
    }
    //work around as every services seems reset the AP name
#ifdef ARDUINO_ARCH_ESP32
#if defined (WIFI_FEATURE)
    if (WiFi.getMode() == WIFI_AP) {
        WiFi.softAPsetHostname(_hostname.c_str());
    }
#endif //WIFI_FEATURE
#endif //ARDUINO_ARCH_ESP32
    DEBUG_ESP3D_NETWORK_INIT
    if (res) {
        log_esp3d("Network config started");
    } else {
        end();
        log_esp3d("Network config failed");
    }
    ESP3DOutput::toScreen(ESP_OUTPUT_IP_ADDRESS,nullptr);
    return res;
}

/**
 * End WiFi
 */

void NetConfig::end()
{
    NetServices::end();
    DEBUG_ESP3D_NETWORK_END
    _mode = ESP_NO_NETWORK;
#if defined (WIFI_FEATURE)
    WiFiConfig::end();
    _needReconnect2AP=false;
#else
    WiFi.mode(WIFI_OFF);
#endif //WIFI_FEATURE

#if defined (ETH_FEATURE)
    EthConfig::end();
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
    bt_service.end();
#endif //BLUETOOTH_FEATURE
    _started = false;
}

const char* NetConfig::hostname(bool fromsettings)
{
    if (fromsettings) {
        _hostname = Settings_ESP3D::read_string(ESP_HOSTNAME);
        return _hostname.c_str();
    }
#if defined (WIFI_FEATURE)
    if(WiFi.getMode()!= WIFI_OFF) {
        _hostname =  WiFiConfig::hostname();
        return _hostname.c_str();
    }
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
    if(EthConfig::started()) {
        return ETH.getHostname();
    }
#endif //ETH_FEATURE

#if defined (BLUETOOTH_FEATURE)
    if(bt_service.started()) {
        return bt_service.hostname();
    }
#endif //BLUETOOTH_FEATURE
    return _hostname.c_str();
}

/**
 * Handle not critical actions that must be done in sync environement
 */

void NetConfig::handle()
{
    if (_started) {
#if defined (WIFI_FEATURE)
        if(_needReconnect2AP) {

            if(WiFi.getMode()!= WIFI_OFF) {
                begin();
            }
        }
        WiFiConfig::handle();
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
        EthConfig::handle();
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
        bt_service.handle();
#endif //BLUETOOTH_FEATURE
        NetServices::handle();
        //Debug
        DEBUG_ESP3D_NETWORK_HANDLE
    }
}

bool NetConfig::isIPModeDHCP (uint8_t mode)
{
    bool started = false;
#ifdef ARDUINO_ARCH_ESP32
    tcpip_adapter_dhcp_status_t dhcp_status;
    tcpip_adapter_dhcpc_get_status ((mode == ESP_WIFI_STA)?TCPIP_ADAPTER_IF_STA:(mode == ESP_WIFI_AP)?TCPIP_ADAPTER_IF_AP:TCPIP_ADAPTER_IF_ETH, &dhcp_status);
    started = (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED);
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
    (void)mode;
    started = (wifi_station_dhcpc_status() == DHCP_STARTED);
#endif //ARDUINO_ARCH_ESP8266
    return started;
}

bool NetConfig::isDHCPServer (uint8_t mode)
{
    bool itis = false;
#ifdef ARDUINO_ARCH_ESP32
    tcpip_adapter_dhcp_status_t dhcp_status;
    tcpip_adapter_dhcps_get_status ((mode == ESP_WIFI_STA)?TCPIP_ADAPTER_IF_STA:(mode == ESP_WIFI_AP)?TCPIP_ADAPTER_IF_AP:TCPIP_ADAPTER_IF_ETH, &dhcp_status);
    itis = (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED);
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
    (void)mode;
    itis = (wifi_softap_dhcps_status() == DHCP_STARTED);
#endif //ARDUINO_ARCH_ESP8266
    return itis;
}

#endif // WIFI_FEATURE || ETH_FEATURE

