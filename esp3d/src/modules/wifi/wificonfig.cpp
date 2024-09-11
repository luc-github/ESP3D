/*
  wificonfig.cpp -  wifi functions class

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
#if defined(WIFI_FEATURE)
#ifdef ARDUINO_ARCH_ESP32
#include <esp_wifi.h>
#if ESP_ARDUINO_VERSION_MAJOR == 3
#include <esp_wifi_ap_get_sta_list.h>
#endif  // ESP_ARDUINO_VERSION_MAJOR == 3
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#endif  // ARDUINO_ARCH_ESP8266
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../network/netconfig.h"
#include "../wifi/wificonfig.h"

#if defined(ARDUINO_ARCH_ESP32)
esp_netif_t *get_esp_interface_netif(esp_interface_t interface);
#endif  // ARDUINO_ARCH_ESP32

const uint8_t DEFAULT_AP_MASK_VALUE[] = {255, 255, 255, 0};

IPAddress WiFiConfig::_ap_gateway;
IPAddress WiFiConfig::_ap_subnet;

const char* WiFiConfig::hostname() {
  static String tmp;
#if defined(ARDUINO_ARCH_ESP8266)
  if (WiFi.getMode() == WIFI_AP) {
    // No API for AP
    tmp = NetConfig::hostname(true);
  } else {
    tmp = WiFi.hostname();
  }
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
  if (WiFi.getMode() == WIFI_AP) {
    // tmp = NetConfig::hostname(true);
    // Set API is not working so far
    tmp = WiFi.softAPgetHostname();
  } else {
    tmp = WiFi.getHostname();
  }
#endif  // ARDUINO_ARCH_ESP8266
  return tmp.c_str();
}

/*
 * Get WiFi signal strength
 */

int32_t WiFiConfig::getSignal(int32_t RSSI, bool filter) {
  if (RSSI < MIN_RSSI && filter) {
    return 0;
  }
  if (RSSI <= -100 && !filter) {
    return 0;
  }
  if (RSSI >= -50) {
    return 100;
  }
  return (2 * (RSSI + 100));
}

/*
 * Connect client to AP
 */

bool WiFiConfig::ConnectSTA2AP() {
  String msg, msg_out;
  uint8_t count = 0;
  uint8_t dot = 0;
  wl_status_t status = WiFi.status();
  esp3d_log("Connecting");
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
  if (!ESP3DSettings::isVerboseBoot()) {
    esp3d_commands.dispatch("Connecting", ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
  while (status != WL_CONNECTED && count < 40) {
    switch (status) {
      case WL_NO_SSID_AVAIL:
        msg = "No SSID";
        break;
      case WL_CONNECT_FAILED:
        msg = "Connection failed";
        break;
      case WL_CONNECTED:
        break;
      default:
        if ((dot > 3) || (dot == 0)) {
          dot = 0;
          msg_out = "Connecting";
        }
        msg_out += ".";
        msg = msg_out;
        esp3d_log("...");
        dot++;
        break;
    }
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch(msg.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    ESP3DHal::wait(500);
    count++;
    status = WiFi.status();
  }
  return (status == WL_CONNECTED);
}

/*
 * Start client mode (Station)
 */
bool WiFiConfig::StartSTA() {
  esp3d_log("StartSTA");
  if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
    WiFi.softAPdisconnect();
  }
  WiFi.enableAP(false);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
#if defined(ARDUINO_ARCH_ESP32)
  esp_wifi_start();
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
#if defined(ESP32_WIFI_TX_POWER)
  WiFi.setTxPower(ESP32_WIFI_TX_POWER);
  delay(100);
#endif  // ESP32_WIFI_TX_POWER
#endif  // ARDUINO_ARCH_ESP32
  // Get parameters for STA
  String SSID = ESP3DSettings::readString(ESP_STA_SSID);
  String password = ESP3DSettings::readString(ESP_STA_PASSWORD);

  if (ESP3DSettings::readByte(ESP_STA_IP_MODE) != DHCP_MODE) {
    int32_t IP = ESP3DSettings::read_IP(ESP_STA_IP_VALUE);
    int32_t GW = ESP3DSettings::read_IP(ESP_STA_GATEWAY_VALUE);
    int32_t MK = ESP3DSettings::read_IP(ESP_STA_MASK_VALUE);
    int32_t DNS = ESP3DSettings::read_IP(ESP_STA_DNS_VALUE);
    IPAddress ip(IP), mask(MK), gateway(GW), dns(DNS);
    WiFi.config(ip, gateway, mask, dns);
  }

  if (ESP3DSettings::isVerboseBoot()) {
    String stmp;
    stmp = "Connecting to '" + SSID + "'";
    esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
  if (WiFi.begin(SSID.c_str(),
                 (password.length() > 0) ? password.c_str() : nullptr)) {
#if defined(ARDUINO_ARCH_ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.hostname(NetConfig::hostname(true));
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    WiFi.setSleep(false);
    WiFi.setHostname(NetConfig::hostname(true));
#endif  // ARDUINO_ARCH_ESP32
    return ConnectSTA2AP();
  } else {
    esp3d_log_e("Starting client failed");
    return false;
  }
}

/**
 * Setup and start Access point
 */

bool WiFiConfig::StartAP(bool setupMode) {
  // Sanity check
  if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
    if (WiFi.isConnected()) {
      WiFi.disconnect();
    }
  }
  if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
    WiFi.softAPdisconnect();
  }
  WiFi.enableAP(true);
  WiFi.enableSTA(false);
  WiFi.mode(WIFI_AP);
  // Set Sleep Mode to none
#if defined(ARDUINO_ARCH_ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#endif  // ARDUINO_ARCH_ESP8266

  String SSID = ESP3DSettings::readString(ESP_AP_SSID);
  String password = ESP3DSettings::readString(ESP_AP_PASSWORD);
  // channel
  int8_t channel = ESP3DSettings::readByte(ESP_AP_CHANNEL);
  // IP
  int32_t IP = ESP3DSettings::read_IP(ESP_AP_IP_VALUE);

  IPAddress ip(IP);
  IPAddress gw(0, 0, 0, 0);
  IPAddress mask(DEFAULT_AP_MASK_VALUE);
#if defined(ARDUINO_ARCH_ESP8266)
  esp3d_log("Use: %s / %s / %s", ip.toString().c_str(), ip.toString().c_str(),
            mask.toString().c_str());
  if (!WiFi.softAPConfig(ip, setupMode ? ip : gw, mask)) {
    esp3d_log_e("Set IP to AP failed");
    esp3d_commands.dispatch("Set IP to AP failed", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    esp3d_commands.dispatch(ip.toString().c_str(), ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
  _ap_gateway = setupMode ? ip : gw;
  _ap_subnet = mask;
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ESP32_WIFI_TX_POWER) && defined(ARDUINO_ARCH_ESP32)
  WiFi.setTxPower(ESP32_WIFI_TX_POWER);
  delay(100);
#endif  // ESP32_WIFI_TX_POWER

  // Start AP
  if (WiFi.softAP(SSID.c_str(),
                  (password.length() > 0) ? password.c_str() : nullptr,
                  channel)) {
    String stmp = "AP SSID: '" + SSID;
    if (password.length() > 0) {
      stmp += "' is started and protected by password";
    } else {
      stmp += " is started not protected by password";
    }
    if (setupMode) {
      stmp += " (setup mode)";
    }
    esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    esp3d_log("%s", stmp.c_str());
#if defined(ARDUINO_ARCH_ESP32)
    // must be done after starting AP not before
    // https://github.com/espressif/arduino-esp32/issues/4222
    // on some phone 100 is ok but on some other it is not enough so 2000 is ok
    ESP3DHal::wait(2000);
    // Set static IP
    esp3d_log("Use: %s / %s / %s", ip.toString().c_str(), ip.toString().c_str(),
              mask.toString().c_str());
    if (!WiFi.softAPConfig(ip, setupMode ? ip : gw, mask)) {
      esp3d_log_e("Set IP to AP failed");
      esp3d_commands.dispatch("Set IP to AP failed",
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    } else {
      esp3d_commands.dispatch(ip.toString().c_str(),
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    WiFi.setSleep(false);
    WiFi.softAPsetHostname(NetConfig::hostname(true));
    _ap_gateway = setupMode ? ip : gw;
    _ap_subnet = mask;
#endif  // ARDUINO_ARCH_ESP32
    return true;
  } else {
    esp3d_commands.dispatch("Starting AP failed", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    esp3d_log_e("Starting AP failed");
    return false;
  }
}

bool WiFiConfig::started() { return (WiFi.getMode() != WIFI_OFF); }

/**
 * begin WiFi setup
 */
bool WiFiConfig::begin(int8_t& espMode) {
  bool res = false;
  end();
  esp3d_log("Starting Wifi Config");
  if (ESP3DSettings::isVerboseBoot()) {
    esp3d_commands.dispatch("Starting WiFi", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
  int8_t wifiMode = espMode;
  if (wifiMode == ESP_WIFI_AP || wifiMode == ESP_AP_SETUP) {
    esp3d_log("Starting AP mode");
    res = StartAP(wifiMode == ESP_AP_SETUP);
  } else if (wifiMode == ESP_WIFI_STA) {
    esp3d_log("Starting STA");
    res = StartSTA();
    // AP is backup mode
    if (!res) {
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(
            "Starting fallback mode", ESP3DClientType::all_clients, no_id,
            ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
      }
      espMode = ESP3DSettings::readByte(ESP_STA_FALLBACK_MODE);
      NetConfig::setMode(espMode);
      if (espMode == ESP_AP_SETUP) {
        esp3d_log("Starting AP mode in setup mode");
        res = StartAP(true);
      } else {
        // let setup to handle the change
        res = true;
      }
    }
  }
  return res;
}

/**
 * End WiFi
 */

void WiFiConfig::end() {
  // Sanity check
  if ((WiFi.getMode() == WIFI_STA) || (WiFi.getMode() == WIFI_AP_STA)) {
    if (WiFi.isConnected()) {
      WiFi.disconnect(true);
    }
  }
  if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
    if (WiFi.isConnected()) {
      WiFi.softAPdisconnect(true);
    }
  }
  WiFi.mode(WIFI_OFF);
}

/**
 * Handle not critical actions that must be done in sync environement
 */

void WiFiConfig::handle() {
  // to avoid mixed mode
  if (WiFi.getMode() == WIFI_AP_STA) {
    if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
      WiFi.enableSTA(false);
    }
  }
}

const char* WiFiConfig::getSleepModeString() {
#ifdef ARDUINO_ARCH_ESP32
  if (WiFi.getSleep()) {
    return "modem";
  } else {
    return "none";
  }
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  WiFiSleepType_t ps_type = WiFi.getSleepMode();
  if (ps_type == WIFI_NONE_SLEEP) {
    return "none";
  } else if (ps_type == WIFI_LIGHT_SLEEP) {
    return "light";
  } else if (ps_type == WIFI_MODEM_SLEEP) {
    return "modem";
  } else {
    return "unknown";
  }
#endif  // ARDUINO_ARCH_ESP8266
}

const char* WiFiConfig::getPHYModeString(uint8_t wifimode) {
#ifdef ARDUINO_ARCH_ESP32
  uint8_t PhyMode;
  esp_wifi_get_protocol(
      (wifi_interface_t)((wifimode == WIFI_STA) ? ESP_IF_WIFI_STA
                                                : ESP_IF_WIFI_AP),
      &PhyMode);
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  (void)wifimode;
  WiFiPhyMode_t PhyMode = WiFi.getPhyMode();
#endif  // ARDUINO_ARCH_ESP8266
  if (PhyMode == (WIFI_PHY_MODE_11G)) {
    return "11g";
  } else if (PhyMode == (WIFI_PHY_MODE_11B)) {
    return "11b";
  } else if (PhyMode == (WIFI_PHY_MODE_11N)) {
    return "11n";
  } else {
    return "unknown";
  }
}

bool WiFiConfig::is_AP_visible() {
#ifdef ARDUINO_ARCH_ESP32
  wifi_config_t conf;
  esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_AP, &conf);
  return (conf.ap.ssid_hidden == 0);
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  struct softap_config apconfig;
  wifi_softap_get_config(&apconfig);
  return (apconfig.ssid_hidden == 0);
#endif  // ARDUINO_ARCH_ESP8266
}

const char* WiFiConfig::AP_SSID() {
  static String ssid;
#ifdef ARDUINO_ARCH_ESP32
  wifi_config_t conf;
  esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_AP, &conf);
  ssid = (const char*)conf.ap.ssid;
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  struct softap_config apconfig;
  wifi_softap_get_config(&apconfig);
  ssid = (const char*)apconfig.ssid;
#endif  // ARDUINO_ARCH_ESP8266
  return ssid.c_str();
}

const char* WiFiConfig::AP_Auth_String() {
  uint8_t mode = 0;
#ifdef ARDUINO_ARCH_ESP32
  wifi_config_t conf;
  esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_AP, &conf);
  mode = conf.ap.authmode;

#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  struct softap_config apconfig;
  wifi_softap_get_config(&apconfig);
  mode = apconfig.authmode;
#endif  // ARDUINO_ARCH_ESP8266
  if (mode == AUTH_OPEN) {
    return "none";
  } else if (mode == AUTH_WEP) {
    return "WEP";
  } else if (mode == AUTH_WPA_PSK) {
    return "WPA";
  } else if (mode == AUTH_WPA2_PSK) {
    return "WPA2";
  } else {
    return "WPA/WPA2";
  }
}

const char* WiFiConfig::AP_Gateway_String() {
  static String tmp;
#ifdef ARDUINO_ARCH_ESP32
  esp_netif_ip_info_t ip_AP;
  esp_netif_get_ip_info(get_esp_interface_netif(ESP_IF_WIFI_AP), &ip_AP);
  tmp = IPAddress(ip_AP.gw.addr).toString();
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  struct ip_info ip_AP;
  if (!wifi_get_ip_info(SOFTAP_IF, &ip_AP)) {
    esp3d_log_e("Error getting gateway ip");
  }
  tmp = IPAddress(ip_AP.gw).toString();
#endif  // ARDUINO_ARCH_ESP8266
  return tmp.c_str();
}

const char* WiFiConfig::AP_Mask_String() {
  static String tmp;
#ifdef ARDUINO_ARCH_ESP32
  esp_netif_ip_info_t ip_AP;
  esp_netif_get_ip_info(get_esp_interface_netif(ESP_IF_WIFI_STA), &ip_AP);
  tmp = IPAddress(ip_AP.netmask.addr).toString();
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  struct ip_info ip_AP;
  if (!wifi_get_ip_info(SOFTAP_IF, &ip_AP)) {
    esp3d_log_e("Error getting mask ip");
  }
  tmp = IPAddress(ip_AP.netmask).toString();
#endif  // ARDUINO_ARCH_ESP8266
  return tmp.c_str();
}

const char* WiFiConfig::getConnectedSTA(uint8_t* totalcount, bool reset) {
  static uint8_t count = 0;
  static uint8_t current = 0;
  static String data;
  data = "";

#ifdef ARDUINO_ARCH_ESP32
  if (current > count) {
    current = 0;
  }
static wifi_sta_list_t sta_list;
#if ESP_ARDUINO_VERSION_MAJOR == 3
static wifi_sta_mac_ip_list_t tcpip_sta_list;
#endif  // ESP_ARDUINO_VERSION_MAJOR == 3
#if ESP_ARDUINO_VERSION_MAJOR == 2
  static tcpip_adapter_sta_list_t tcpip_sta_list;
#endif  // ESP_ARDUINO_VERSION_MAJOR == 2 
  if (reset) {
    count = 0;
  }
  if (count == 0) {
    current = 0;
#if ESP_ARDUINO_VERSION_MAJOR == 3    
    if(esp_wifi_ap_get_sta_list(&sta_list)!=ESP_OK){
#endif  // ESP_ARDUINO_VERSION_MAJOR == 3
#if ESP_ARDUINO_VERSION_MAJOR == 2
    if(tcpip_adapter_get_sta_list(&sta_list, &tcpip_sta_list)!=ESP_OK){
#endif  // ESP_ARDUINO_VERSION_MAJOR == 2
  return "";
}
    if (sta_list.num > 0) {
#if ESP_ARDUINO_VERSION_MAJOR == 3
      ESP_ERROR_CHECK(
          esp_wifi_ap_get_sta_list_with_ip(&sta_list, &tcpip_sta_list));
#endif  // ESP_ARDUINO_VERSION_MAJOR == 3
    }
    count = sta_list.num;
  }
  if (count > 0) {
    data = IPAddress(tcpip_sta_list.sta[current].ip.addr).toString();
    data += "(";
    data += NetConfig::mac2str(tcpip_sta_list.sta[current].mac);
    data += ")";
    current++;
  }

#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  static struct station_info* station;
  if (current > count) {
    current = 0;
    count = 0;
  }
  if (reset) {
    count = 0;
  }
  if (count == 0) {
    current = 0;
    station = wifi_softap_get_station_info();
    struct station_info* station_tmp = station;
    while (station) {
      // go next record
      count++;
      station = STAILQ_NEXT(station, next);
    }
    station = station_tmp;
  }
  if ((count > 0) && station) {
    data = IPAddress((const uint8_t*)&station->ip).toString();
    data += "(";
    data += NetConfig::mac2str(station->bssid);
    data += ")";
    current++;
    station = STAILQ_NEXT(station, next);
    if ((current == count) || !station) {
      wifi_softap_free_station_info();
    }
  }
#endif  // ARDUINO_ARCH_ESP8266
  if (totalcount) {
    *totalcount = count;
  }
  return data.c_str();
}

#endif  // WIFI_FEATURE
