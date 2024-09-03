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
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BLUETOOTH_FEATURE)
#ifdef ARDUINO_ARCH_ESP32
#define WIFI_EVENT_STAMODE_CONNECTED ARDUINO_EVENT_WIFI_STA_CONNECTED
#define WIFI_EVENT_STAMODE_DISCONNECTED ARDUINO_EVENT_WIFI_STA_DISCONNECTED
#define WIFI_EVENT_STAMODE_GOT_IP ARDUINO_EVENT_WIFI_STA_GOT_IP
#define WIFI_EVENT_SOFTAPMODE_STACONNECTED ARDUINO_EVENT_WIFI_AP_STACONNECTED
#define RADIO_OFF_MSG "Radio Off"
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#define RADIO_OFF_MSG "WiFi Off"
#endif  // ARDUINO_ARCH_ESP8266
#include "netconfig.h"
#if defined(WIFI_FEATURE)
#include "../wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
#include "../ethernet/ethconfig.h"
#endif  // ETH_FEATURE
#if defined(BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "netservices.h"
#if defined(GCODE_HOST_FEATURE)
#include "../gcode_host/gcode_host.h"
#endif  // GCODE_HOST_FEATURE

#if defined(ARDUINO_ARCH_ESP32)
esp_netif_t *get_esp_interface_netif(esp_interface_t interface);
#endif  // ARDUINO_ARCH_ESP32

String NetConfig::_hostname = "";
bool NetConfig::_needReconnect2AP = false;
bool NetConfig::_events_registered = false;
bool NetConfig::_started = false;
uint8_t NetConfig::_mode = ESP_NO_NETWORK;

// just simple helper to convert mac address to string
char* NetConfig::mac2str(uint8_t mac[8]) {
  static char macstr[18];
  if (0 > sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
                  mac[2], mac[3], mac[4], mac[5])) {
    strcpy(macstr, "00:00:00:00:00:00");
  }
  return macstr;
}

/**
 * Helper to convert  IP string to int
 */
uint32_t NetConfig::IP_int_from_string(const char* s) {
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
String NetConfig::IP_string_from_int(uint32_t ip_int) {
  IPAddress ipaddr(ip_int);
  return ipaddr.toString();
}

/**
 * Get IP Integer what ever is enabled
 */
IPAddress NetConfig::localIPAddress() {
  IPAddress current_ip = IPAddress(0, 0, 0, 0);
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_STA) {
    current_ip = WiFi.localIP();
  } else if (WiFi.getMode() == WIFI_AP) {
    current_ip = WiFi.softAPIP();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    current_ip = ETH.localIP();
  }
#endif  // ETH_FEATURE

  return current_ip;
}

/**
 * Get IP string what ever is enabled
 */
String NetConfig::localIP() {
  static String currentIP = "";
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_STA) {
    currentIP = WiFi.localIP().toString();
  } else if (WiFi.getMode() == WIFI_AP) {
    currentIP = WiFi.softAPIP().toString();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    currentIP = ETH.localIP().toString();
  }
#endif  // ETH_FEATURE
  if (currentIP.length() == 0) {
    currentIP = "0.0.0.0";
  }
  return currentIP;
}

/**
 * Get Gateway IP string what ever is enabled
 */
String NetConfig::localGW() {
  static String currentIP = "";
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_STA) {
    currentIP = WiFi.localIP().toString();
  } else if (WiFi.getMode() == WIFI_AP) {
    currentIP = WiFiConfig::getAPGateway().toString();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    currentIP = ETH.localIP().toString();
  }
#endif  // ETH_FEATURE
  if (currentIP.length() == 0) {
    currentIP = "0.0.0.0";
  }
  return currentIP;
}

/**
 * Get Network mask string what ever is enabled
 */
String NetConfig::localMSK() {
  static String currentIP = "";
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_STA) {
    currentIP = WiFi.subnetMask().toString();
  } else if (WiFi.getMode() == WIFI_AP) {
    currentIP = WiFiConfig::getAPSubnet().toString();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    currentIP = ETH.subnetMask().toString();
  }
#endif  // ETH_FEATURE
  if (currentIP.length() == 0) {
    currentIP = "0.0.0.0";
  }
  return currentIP;
}

/**
 * Get DNS IP string what ever is enabled
 */
String NetConfig::localDNS() {
  static String currentIP = "";
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_STA) {
    currentIP = WiFi.dnsIP().toString();
  } else if (WiFi.getMode() == WIFI_AP) {
    currentIP = WiFi.softAPIP().toString();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    currentIP = ETH.dnsIP().toString();
  }
#endif  // ETH_FEATURE
  if (currentIP.length() == 0) {
    currentIP = "0.0.0.0";
  }
  return currentIP;
}

// wifi event
void NetConfig::onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_CONNECTED:
      _needReconnect2AP = false;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED: {
      if (_started) {
        esp3d_commands.dispatch("Disconnected", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
        //_needReconnect2AP = true;
      }
    } break;
    case WIFI_EVENT_STAMODE_GOT_IP: {
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
#if defined(ESP_GOT_IP_HOOK) && defined(GCODE_HOST_FEATURE)
      String ipMsg = esp3d_string::expandString(ESP_GOT_IP_HOOK);
      esp3d_log("Got IP, sending hook: %s", ipMsg.c_str());
      esp3d_gcode_host.processScript(ipMsg.c_str(),
                                     ESP3DAuthenticationLevel::admin);
#endif  // #if defined (ESP_GOT_IP_HOOK) && defined (GCODE_HOST_FEATURE)
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
    } break;
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED: {
      esp3d_commands.dispatch("New client", ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    } break;
#ifdef ARDUINO_ARCH_ESP32
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      if (_started) {
        _needReconnect2AP = true;
        esp3d_log("WiFi STA lost IP");
      }
      break;
#ifdef ETH_FEATURE
    case ARDUINO_EVENT_ETH_START: {
      esp3d_log("Ethernet started");
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(
            "Checking connection", ESP3DClientType::all_clients, no_id,
            ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
      }
    } break;
    case ARDUINO_EVENT_ETH_CONNECTED: {
      esp3d_commands.dispatch("Cable connected", ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
      esp3d_log("Ethernet connected");
      EthConfig::setConnected(true);
    } break;
    case ARDUINO_EVENT_ETH_DISCONNECTED: {
      esp3d_log("Ethernet disconnected");
      esp3d_commands.dispatch("Cable disconnected",
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
      EthConfig::setConnected(false);
    } break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      esp3d_log("Ethernet lost IP");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP: {
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
#if defined(ESP_GOT_IP_HOOK) && defined(GCODE_HOST_FEATURE)
      ESP3DHal::wait(500);
      String ipMsg = esp3d_string::expandString(ESP_GOT_IP_HOOK);
      esp3d_log("Got IP, sending hook: %s", ipMsg.c_str());
      esp3d_gcode_host.processScript(ipMsg.c_str(),
                                     ESP3DAuthenticationLevel::admin);
#endif  // #if defined (ESP_GOT_IP_HOOK) && defined (GCODE_HOST_FEATURE)
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
      EthConfig::setConnected(true);
    } break;

    case ARDUINO_EVENT_ETH_STOP:
      esp3d_log("Ethernet stopped");
      EthConfig::setConnected(false);
      break;
#endif  // ETH_FEATURE
#endif  // ARDUINO_ARCH_ESP32
    default:
      break;
  }
}

void NetConfig::setMode(uint8_t mode) { _mode = mode; }

uint8_t NetConfig::getMode() { return _mode; }

/**
 * begin WiFi setup
 */
bool NetConfig::begin() {
  bool res = false;
  // clear everything
  end();
  int8_t espMode = ESP3DSettings::readByte(ESP_RADIO_MODE);
  esp3d_log("Starting Network");
  if (espMode != ESP_NO_NETWORK) {
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch("Starting Network", ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
  }
  // setup events
  if (!_events_registered) {
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
  // Get hostname
  _hostname = ESP3DSettings::readString(ESP_HOSTNAME);
  _mode = espMode;
  if (espMode == ESP_NO_NETWORK) {
    esp3d_commands.dispatch("Disable Network", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    WiFi.mode(WIFI_OFF);
#if defined(DISPLAY_DEVICE)
    ESP3DRequest reqId = {
        .id = ESP_OUTPUT_IP_ADDRESS,
    };
    esp3d_commands.dispatch(" ", ESP3DClientType::rendering, reqId,
                            ESP3DMessageType::unique);
#endif  // DISPLAY_DEVICE
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch(RADIO_OFF_MSG, ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    return true;
  }
#if defined(WIFI_FEATURE)
  if ((espMode == ESP_AP_SETUP) || (espMode == ESP_WIFI_AP) ||
      (espMode == ESP_WIFI_STA)) {
    esp3d_commands.dispatch("Setup wifi", ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    res = WiFiConfig::begin(espMode);
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  // if ((espMode == ESP_ETH_STA) || (espMode == ESP_ETH_SRV)) {
  if ((espMode == ESP_ETH_STA)) {
    WiFi.mode(WIFI_OFF);
    res = EthConfig::begin(espMode);
  }
#else
  // if Eth and no Eth enabled let's go to no network
  if (espMode == ESP_ETH_STA) {
    espMode = ESP_NO_NETWORK;
  }
#endif  // ETH_FEATURE

#if defined(BLUETOOTH_FEATURE)
  if (espMode == ESP_BT) {
    WiFi.mode(WIFI_OFF);
    String msg = "BT On";
#if defined(DISPLAY_DEVICE)
    ESP3DRequest reqId = {
        .id = ESP_OUTPUT_STATUS,
    };
    esp3d_commands.dispatch(msg.c_str(), ESP3DClientType::rendering, reqId,
                            ESP3DMessageType::unique);
#endif  // DISPLAY_DEVICE
    res = bt_service.begin();
  }
#else
  // if BT and no BT enabled let's go to no network
  if (espMode == ESP_BT) {
    espMode = ESP_NO_NETWORK;
  }
#endif  // BLUETOOTH_FEATURE

  if (espMode == ESP_NO_NETWORK) {
    esp3d_commands.dispatch("Disable Network", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    WiFi.mode(WIFI_OFF);
#if defined(DISPLAY_DEVICE)
    ESP3DRequest reqId = {
        .id = ESP_OUTPUT_IP_ADDRESS,
    };
    esp3d_commands.dispatch(" ", ESP3DClientType::rendering, reqId,
                            ESP3DMessageType::unique);
#endif  // DISPLAY_DEVICE
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch(RADIO_OFF_MSG, ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    return true;
  }
  // if network is up, let's start services
  if (res) {
    _started = true;
    bool start_services = false;
#if defined(ETH_FEATURE)
    if (EthConfig::started()) {
      start_services = true;
    }
#endif  // ETH_FEATURE
#if defined(WIFI_FEATURE)
    if (WiFiConfig::started()) {
      start_services = true;
    }
#endif  // WIFI_FEATURE
    if (start_services) {
      esp3d_log("Starting service");
      res = NetServices::begin();
    }
  }
  // work around as every services seems reset the AP name
#ifdef ARDUINO_ARCH_ESP32
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() == WIFI_AP) {
    WiFi.softAPsetHostname(_hostname.c_str());
  }
#endif  // WIFI_FEATURE
#endif  // ARDUINO_ARCH_ESP32
  ESP3D_LOG_NETWORK_INIT_FN
  if (res) {
    esp3d_log("Network config started");

  } else {
    end();
    esp3d_log_e("Network config failed");
  }
#if defined(DISPLAY_DEVICE)
  ESP3DRequest reqId = {
      .id = ESP_OUTPUT_IP_ADDRESS,
  };
  esp3d_commands.dispatch(" ", ESP3DClientType::rendering, reqId,
                          ESP3DMessageType::unique);
#endif  // DISPLAY_DEVICE
  return res;
}

/**
 * End WiFi
 */

void NetConfig::end() {
  NetServices::end();
  ESP3D_LOG_NETWORK_END_FN
  _mode = ESP_NO_NETWORK;
#if defined(WIFI_FEATURE)
  WiFiConfig::end();
  _needReconnect2AP = false;
#else
  WiFi.mode(WIFI_OFF);
#endif  // WIFI_FEATURE

#if defined(ETH_FEATURE)
  EthConfig::end();
#endif  // ETH_FEATURE
#if defined(BLUETOOTH_FEATURE)
  bt_service.end();
#endif  // BLUETOOTH_FEATURE
  _started = false;
}

const char* NetConfig::hostname(bool fromsettings) {
  if (fromsettings) {
    _hostname = ESP3DSettings::readString(ESP_HOSTNAME);
    return _hostname.c_str();
  }
#if defined(WIFI_FEATURE)
  if (WiFi.getMode() != WIFI_OFF) {
    _hostname = WiFiConfig::hostname();
    return _hostname.c_str();
  }
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
  if (EthConfig::started()) {
    return ETH.getHostname();
  }
#endif  // ETH_FEATURE

#if defined(BLUETOOTH_FEATURE)
  if (bt_service.started()) {
    return bt_service.hostname();
  }
#endif  // BLUETOOTH_FEATURE
  return _hostname.c_str();
}

/**
 * Handle not critical actions that must be done in sync environement
 */

void NetConfig::handle() {
  if (_started) {
#if defined(WIFI_FEATURE)
    if (_needReconnect2AP) {
      if (WiFi.getMode() != WIFI_OFF) {
        begin();
      }
    }
    WiFiConfig::handle();
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
    EthConfig::handle();
#endif  // ETH_FEATURE
#if defined(BLUETOOTH_FEATURE)
    bt_service.handle();
#endif  // BLUETOOTH_FEATURE
    NetServices::handle();
    // Debug
    ESP3D_LOG_NETWORK_HANDLE_FN
  }
}

bool NetConfig::isIPModeDHCP(uint8_t mode) {
  bool started = false;
#ifdef ARDUINO_ARCH_ESP32
if (mode == ESP_WIFI_STA || mode == ESP_WIFI_AP) {
  esp_netif_dhcp_status_t dhcp_status;
  esp_netif_dhcpc_get_status((mode == ESP_WIFI_STA)  ? get_esp_interface_netif(ESP_IF_WIFI_STA)
                                 :  get_esp_interface_netif(ESP_IF_WIFI_AP),
                                 &dhcp_status);
  esp3d_log("DHCP Status %d", (int)dhcp_status);
  started = (dhcp_status == ESP_NETIF_DHCP_STARTED);
} 
#if defined(ETH_FEATURE)
if (mode == ESP_ETH_STA) {
  started = (EthConfig::ipMode()==DHCP_MODE);
}
#endif  // ETH_FEATURE

#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  (void)mode;
  started = (wifi_station_dhcpc_status() == DHCP_STARTED);
#endif  // ARDUINO_ARCH_ESP8266
  return started;
}

bool NetConfig::isDHCPServer(uint8_t mode) {
  bool itis = false;
#ifdef ARDUINO_ARCH_ESP32
  //Fzor some reason esp_netif_dhcps_get_status() give always !DHCP_MODE for Ethernet even if it is set to DHCP
  if (mode == ESP_WIFI_AP) {
  esp_netif_dhcp_status_t dhcp_status;
  esp_netif_dhcps_get_status(get_esp_interface_netif(ESP_IF_WIFI_AP),
                                 &dhcp_status);
  itis = (dhcp_status == ESP_NETIF_DHCP_STARTED);
  } 
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  (void)mode;
  itis = (wifi_softap_dhcps_status() == DHCP_STARTED);
#endif  // ARDUINO_ARCH_ESP8266
  return itis;
}

#endif  // WIFI_FEATURE || ETH_FEATURE
