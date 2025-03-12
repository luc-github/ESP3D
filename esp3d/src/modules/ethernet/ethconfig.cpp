/*
  ethconfig.cpp -  ethernet functions class

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
#if defined(ETH_FEATURE)
#ifdef ARDUINO_ARCH_ESP32
#include "dhcpserver/dhcpserver_options.h"
#include "esp_eth.h"

#endif  // ARDUINO_ARCH_ESP32
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../network/netconfig.h"
#include "ethconfig.h"
#ifdef ETHERNET_SPI_USE_SPI
#define ETH_SPI SPI
#endif  // ETHERNET_SPI_USE_SPI
#if ETHERNET_SPI_USE_SPI2
#define ETH_SPI SPI2
#endif  // ETHERNET_SPI_USE_SPI2
#ifndef ETH_SPI
#define ETH_SPI SPI
#endif  // ETH_SPI
#

#if defined(GCODE_HOST_FEATURE)
#include "../gcode_host/gcode_host.h"
#endif  // GCODE_HOST_FEATURE

bool EthConfig::_started = false;
bool EthConfig::_connected = false;
uint8_t EthConfig::_ipMode = DHCP_MODE;
const uint8_t DEFAULT_AP_MASK_VALUE[] = {255, 255, 255, 0};

bool EthConfig::StartSTA() {
  bool res = true;
  if (_ipMode == STATIC_IP_MODE) {
    int32_t IP = ESP3DSettings::read_IP(ESP_ETH_STA_IP_VALUE);
    int32_t GW = ESP3DSettings::read_IP(ESP_ETH_STA_GATEWAY_VALUE);
    int32_t MK = ESP3DSettings::read_IP(ESP_ETH_STA_MASK_VALUE);
    int32_t DNS = ESP3DSettings::read_IP(ESP_ETH_STA_DNS_VALUE);
    IPAddress ip(IP), mask(MK), gateway(GW), dns(DNS);
    res = ETH.config(ip, gateway, mask, dns);
  }
  return res;
}

bool EthConfig::linkUp() { return _connected; }

uint8_t EthConfig::ipMode(bool fromsettings) {
  if (fromsettings) {
    _ipMode = (ESP3DSettings::readByte(ESP_ETH_STA_IP_MODE) != DHCP_MODE);
  }
  return _ipMode;
}

/**
 * begin WiFi setup
 */
bool EthConfig::begin(int8_t& espMode) {
  bool res = false;
  ipMode(true);
  end();
#if ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_LAN8720
  esp3d_log("ETH PHY Type %d", ESP3D_ETH_PHY_TYPE);
  _started = ETH.begin();
#endif  // ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_LAN8720
#if ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_TLK110 ||  \
    ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_RTL8201 || \
    ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_DP83848 || \
    ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_KSZ8041 || \
    ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_KSZ8081
  eth_phy_type_t phytype = ETH_PHY_TLK110;
  if (ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_RTL8201) {
    phytype = ETH_PHY_RTL8201;
  }
  if (ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_DP83848) {
    phytype = ETH_PHY_DP83848;
  }
  if (ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_KSZ8041) {
    phytype = ETH_PHY_KSZ8041;
  }
  if (ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_KSZ8081) {
    phytype = ETH_PHY_KSZ8081;
  }
  esp3d_log("ETH PHY Type %d", phytype);
  _started = ETH.begin(phytype, ESP3D_ETH_PHY_ADDR, ESP3D_ETH_PHY_POWER_PIN,
                       ESP3D_ETH_PHY_MDC_PIN, ESP3D_ETH_PHY_MDIO_PIN,
                       ESP3D_ETH_CLK_MODE_PIN);
#endif  // ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_TLK110 || ESP3D_ETH_PHY_TYPE ==
        // TYPE_ETH_PHY_RTL8201 || ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_DP83848 ||
        // ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_KSZ8041 || ESP3D_ETH_PHY_TYPE ==
        // TYPE_ETH_PHY_KSZ8081
#if ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_W5500
  esp3d_log("ETH spi PHY Type %d", ESP3D_ETH_PHY_TYPE);
  ETH_SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);
  _started = ETH.begin(ETH_PHY_W5500, ESP3D_ETH_PHY_ADDR, ETH_PHY_CS,
                       ETH_PHY_IRQ, ETH_PHY_RST, ETH_SPI);
#endif  // ESP3D_ETH_PHY_TYPE == TYPE_ETH_PHY_W5500

  if (_started) {
    esp3d_log("Starting ethernet success");
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch("Starting ethernet", ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    res = true;
  } else {
    esp3d_commands.dispatch("Failed starting ethernet failed",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
    esp3d_log("Failed starting ethernet failed");
    return false;
  }
  ETH.setHostname(NetConfig::hostname(true));
  // DHCP is only for Client
  if (espMode == ESP_ETH_STA) {
    if (!StartSTA()) {
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_log("Starting fallback mode");
        esp3d_commands.dispatch(
            "Starting fallback mode", ESP3DClientType::all_clients, no_id,
            ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
      }
      espMode = ESP3DSettings::readByte(ESP_ETH_STA_FALLBACK_MODE);
      res = false;
      return res;
    } else {
      esp3d_log("Client started");
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch("Client started", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }

      res = true;
    }
  }
  if (res) {
    // Static IP or DHCP client ?
    if ((ESP3DSettings::readByte(ESP_ETH_STA_IP_MODE) != DHCP_MODE)) {
      int32_t IP = ESP3DSettings::read_IP(ESP_ETH_STA_IP_VALUE);
      int32_t GW = ESP3DSettings::read_IP(ESP_ETH_STA_GATEWAY_VALUE);
      int32_t MK = ESP3DSettings::read_IP(ESP_ETH_STA_MASK_VALUE);
      int32_t DNS = ESP3DSettings::read_IP(ESP_ETH_STA_DNS_VALUE);
      IPAddress ip(IP), mask(MK), gateway(GW), dns(DNS);
      ETH.config(ip, gateway, mask, dns);
    } else {     
      uint64_t start = millis();
      String ip = ETH.localIP().toString();
      esp3d_log("IP");
      esp3d_log("Waiting for IP");
      while (millis() - start < 10000 && ip == "0.0.0.0") {
        ip = ETH.localIP().toString();
        ESP3DHal::wait(100);
      }
      if (ip != "0.0.0.0") {
        esp3d_log("Show IP");
        esp3d_commands.dispatch(
            ETH.localIP().toString().c_str(), ESP3DClientType::all_clients,
            no_id, ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
      } else {
        esp3d_log_e("Failed to get IP");
        res = false;
      }
    }
  } else {
    esp3d_log_e("Failed starting ethernet");
  }

  return res;
}

/**
 * End WiFi
 */

void EthConfig::end() {
  // ETH.end();
  _started = false;
  _ipMode = DHCP_MODE;
  _connected = false;
}

bool EthConfig::started() { return _started; }
/**
 * Handle not critical actions that must be done in sync environement
 */

void EthConfig::handle() {}

#endif  // ETH_FEATURE
