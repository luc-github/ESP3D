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
#if defined (ETH_FEATURE)
#ifdef ARDUINO_ARCH_ESP32
#include "esp_eth.h"
#include "dhcpserver/dhcpserver_options.h"
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#endif //ARDUINO_ARCH_ESP8266
#include "../../core/esp3doutput.h"
#include "../../core/settings_esp3d.h"
#include "../network/netconfig.h"
#include "ethconfig.h"
bool EthConfig::_started = false;
const uint8_t DEFAULT_AP_MASK_VALUE[]  =      {255, 255, 255, 0};

bool EthConfig::StartSTA()
{
    bool res = true;
    if ((Settings_ESP3D::read_byte(ESP_STA_IP_MODE) != DHCP_MODE)) {
        int32_t IP = Settings_ESP3D::read_IP(ESP_STA_IP_VALUE);
        int32_t GW = Settings_ESP3D::read_IP(ESP_STA_GATEWAY_VALUE);
        int32_t MK = Settings_ESP3D::read_IP(ESP_STA_MASK_VALUE);
        int32_t DNS = Settings_ESP3D::read_IP(ESP_STA_DNS_VALUE);
        IPAddress ip(IP), mask(MK), gateway(GW), dns(DNS);
        res = ETH.config(ip, gateway,mask,dns);
    }
    return res;
}
/*bool EthConfig::StartSRV()
{
    bool res = true;
    //static IP
    int32_t IP = Settings_ESP3D::read_IP(ESP_AP_IP_VALUE);
    IPAddress ip(IP), mask(DEFAULT_AP_MASK_VALUE), gateway(IP);
    if (!ETH.config(ip, gateway,mask)) {
        res = false;
        log_esp3d("Set static IP error");
    }
    //start DHCP server
    if(res) {
        dhcps_lease_t lease;
        lease.enable = true;
        lease.start_ip.addr = static_cast<uint32_t>(IP) + (1 << 24);
        lease.end_ip.addr = static_cast<uint32_t>(IP) + (11 << 24);
        tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_ETH);
        tcpip_adapter_dhcps_option(
            (tcpip_adapter_option_mode_t)TCPIP_ADAPTER_OP_SET,
            (tcpip_adapter_option_id_t)REQUESTED_IP_ADDRESS,
            (void*)&lease, sizeof(dhcps_lease_t)
        );

        if (tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_ETH) != ESP_OK){
            res = false;
            log_esp3d("Start DHCP server failed");
        }
    }
    return res;
}*/

/**
 * begin WiFi setup
 */
bool EthConfig::begin(int8_t & espMode)
{
    bool res = false;
    ESP3DOutput output(ESP_ALL_CLIENTS);
    end();
    _started = ETH.begin();
    if (_started) {
        if (Settings_ESP3D::isVerboseBoot()) {
            output.printMSG("Starting Ethernet");
        }
        res=true;
    } else {
        output.printERROR("Failed Starting Ethernet");
    }
    ETH.setHostname(NetConfig::hostname(true));

    //DHCP is only for Client
    if (espMode == ESP_ETH_STA) {
        if(!StartSTA()) {
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG("Starting fallback mode");
            }
            espMode =  Settings_ESP3D::read_byte(ESP_STA_FALLBACK_MODE);
            res = true;
        } else {
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG ("Client started");
            }

        }

    } else {
        //if(!StartSRV()){
        //   res = false;
        //   output.printMSG ("Failed Starting Server");
        //} else {
        //    output.printMSG ("Server started");
        //}
    }

    //if ((Settings_ESP3D::read_byte(ESP_STA_IP_MODE) != DHCP_MODE) || (espMode == ESP_ETH_SRV)){
    if ((Settings_ESP3D::read_byte(ESP_STA_IP_MODE) != DHCP_MODE)) {
        //as no event to display static IP
        output.printMSG (ETH.localIP().toString().c_str());
    }

    //Let wait cable is connected
    uint32_t start = millis();
    String stmp ="Checking connection";
    output.printMSG (stmp.c_str());
    while (!ETH.linkUp() && ((millis()-start) < 10000)) {
        Hal::wait(1000);
        stmp +=".";
        output.printMSG (stmp.c_str());
    }
    if (!ETH.linkUp()) {
        output.printMSG ("Cable disconnected");
    }
    return res;
}

/**
 * End WiFi
 */

void EthConfig::end()
{
    //esp_eth_disable();
    _started = false;
}

bool EthConfig::started()
{
    return _started;
}
/**
 * Handle not critical actions that must be done in sync environement
 */

void EthConfig::handle()
{
}


#endif // ETH_FEATURE

