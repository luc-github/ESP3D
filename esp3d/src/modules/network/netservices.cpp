/*
  netservices.cpp -  network services functions class

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

#include "netconfig.h"
#include "netservices.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#if defined( ARDUINO_ARCH_ESP8266)
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif //MDNS_FEATURE
#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif //SSDP_FEATURE
#endif //ARDUINO_ARCH_ESP8266
#if defined( ARDUINO_ARCH_ESP32)
#ifdef MDNS_FEATURE
#include <ESPmDNS.h>
#endif //MDNS_FEATURE
#ifdef SSDP_FEATURE
#include <ESP32SSDP.h>
#endif //SSDP_FEATURE
#endif //ARDUINO_ARCH_ESP32
#ifdef OTA_FEATURE
#include <ArduinoOTA.h>
#endif //OTA_FEATURE
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
#endif //FILESYSTEM_FEATURE
#ifdef TELNET_FEATURE
#include "../telnet/telnet_server.h"
#endif //TELNET_FEATURE
#ifdef FTP_FEATURE
#include "../ftp/FtpServer.h"
#endif //FP_FEATURE
#ifdef WEBDAV_FEATURE
#include "../webdav/webdav_server.h"
#endif //WEBDAV_FEATURE
#ifdef HTTP_FEATURE
#include "../http/http_server.h"
#endif //HTTP_FEATURE
#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../websocket/websocket_server.h"
#endif //HTTP_FEATURE || WS_DATA_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif //CAPTIVE_PORTAL_FEATURE
#ifdef TIMESTAMP_FEATURE
#include "../time/time_server.h"
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
#include "../notifications/notifications_service.h"
#endif //NOTIFICATION_FEATURE
#ifdef CAMERA_DEVICE
#include "../camera/camera.h"
#endif //CAMERA_DEVICE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL

bool NetServices::_started = false;
bool NetServices::_restart = false;

bool NetServices::begin()
{
    bool res = true;
    _started = false;
    String hostname = Settings_ESP3D::read_string(ESP_HOSTNAME);
    ESP3DOutput output(ESP_ALL_CLIENTS);
    end();
#ifdef TIMESTAMP_FEATURE
    if (WiFi.getMode() != WIFI_AP) {
        if(!timeserver.begin()) {
            if(timeserver.is_internet_time()) {
                output.printERROR("Failed contact time servers!");
            }
        } else {
            String tmp = "Current time :";
            tmp+=timeserver.current_time();
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(tmp.c_str());
            }
        }
    }
#endif //TIMESTAMP_FEATURE

#if defined(MDNS_FEATURE) && defined(ARDUINO_ARCH_ESP8266)
    if(WiFi.getMode() != WIFI_AP) {
        String lhostname =hostname;
        lhostname.toLowerCase();
        log_esp3d("Start mdsn for %s", hostname.c_str());
        if (!MDNS.begin(hostname.c_str())) {
            output.printERROR("mDNS failed to start");
            _started =false;
        } else {
            String stmp = "mDNS started with '" + lhostname + ".local'";
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //MDNS_FEATURE && ARDUINO_ARCH_ESP8266

#ifdef OTA_FEATURE
    if(WiFi.getMode() != WIFI_AP) {
        ArduinoOTA.onStart([]() {
            ESP3DOutput output(ESP_ALL_CLIENTS);
            String type = "Start OTA updating ";
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type += "sketch";
            } else { // U_SPIFFS or any FS
                // NOTE: if updating FS this would be the place to unmount FS using FS.end()
                type += "filesystem";
#if defined(FILESYSTEM_FEATURE)
                ESP_FileSystem::end();
#endif //FILESYSTEM_FEATURE

            }
            output.printMSG(type.c_str());
        });
        ArduinoOTA.onEnd([]() {
            ESP3DOutput output(ESP_ALL_CLIENTS);
            output.printMSG("End OTA");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            String prg = "OTA Progress ";
            ESP3DOutput output(ESP_ALL_CLIENTS);
            prg += String(progress / (total / 100)) + "%";
            output.printMSG(prg.c_str());
        });
        ArduinoOTA.onError([](ota_error_t error) {
            String stmp = "OTA Error: " + String(error);
            ESP3DOutput output(ESP_ALL_CLIENTS);
            output.printERROR(stmp.c_str());
            if (error == OTA_AUTH_ERROR) {
                output.printERROR("Auth Failed");
            } else if (error == OTA_BEGIN_ERROR) {
                output.printERROR("Begin Failed");
            } else if (error == OTA_CONNECT_ERROR) {
                output.printERROR("Connect Failed");
            } else if (error == OTA_RECEIVE_ERROR) {
                output.printERROR("Receive Failed");
            } else if (error == OTA_END_ERROR) {
                output.printERROR("End Failed");
            }
        });
        if (Settings_ESP3D::isVerboseBoot()) {
            output.printMSG("OTA service started");
        }
        String lhostname =hostname;
        lhostname.toLowerCase();
        ArduinoOTA.setHostname(hostname.c_str());
        ArduinoOTA.begin();
    }
#endif

#if defined(MDNS_FEATURE) && defined(ARDUINO_ARCH_ESP32)
    if(WiFi.getMode() != WIFI_AP) {
        String lhostname =hostname;
        lhostname.toLowerCase();
        if (!MDNS.begin(hostname.c_str())) {
            output.printERROR("mDNS failed to start");
            _started =false;
        } else {
            String stmp = "mDNS started with '" + lhostname + ".local'";
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //MDNS_FEATURE && ARDUINO_ARCH_ESP8266

#ifdef CAPTIVE_PORTAL_FEATURE
    if(NetConfig::getMode() == ESP_AP_SETUP) {
        // if DNSServer is started with "*" for domain name, it will reply with
        // provided IP to all DNS request
        if (dnsServer.start(DNS_PORT, "*", WiFi.softAPIP())) {
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG("Captive Portal started");
            }
        } else {
            output.printERROR("Failed start Captive Portal");
        }
    }
#endif //CAPTIVE_PORTAL_FEATURE

#ifdef HTTP_FEATURE
    if (!HTTP_Server::begin()) {
        res= false;
        output.printERROR("HTTP server failed");
    } else {
        if(HTTP_Server::started()) {
            String stmp = "HTTP server started port " + String(HTTP_Server::port());
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //HTTP_FEATURE
#ifdef TELNET_FEATURE
    if (!telnet_server.begin()) {
        res= false;
        output.printERROR("Telnet server failed");
    } else {
        if(telnet_server.started()) {
            String stmp = "Telnet server started port " + String(telnet_server.port());
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //TELNET_FEATURE
#ifdef FTP_FEATURE
    if (!ftp_server.begin()) {
        res= false;
        output.printERROR("Ftp server failed");
    } else {
        if(ftp_server.started()) {
            String stmp = "Ftp server started ports: " + String(ftp_server.ctrlport()) + ","+ String(ftp_server.dataactiveport()) + ","+ String(ftp_server.datapassiveport());
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //FTP_FEATURE
#ifdef WS_DATA_FEATURE
    if (!websocket_data_server.begin(Settings_ESP3D::read_uint32(ESP_WEBSOCKET_PORT))) {
        output.printMSG("Failed start Terminal Web Socket");
    } else {
        if (websocket_data_server.started()) {
            String stmp = "Websocket server started port " + String(websocket_data_server.port());
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
    if (!webdav_server.begin()) {
        output.printMSG("Failed start Terminal Web Socket");
    } else {
        if (webdav_server.started()) {
            String stmp = "WebDav server started port " + String(webdav_server.port());
            if (Settings_ESP3D::isVerboseBoot()) {
                output.printMSG(stmp.c_str());
            }
        }
    }
#endif //WEBDAV_FEATURE
#if defined(HTTP_FEATURE)
    if (!websocket_terminal_server.begin()) {
        output.printMSG("Failed start Terminal Web Socket");
    }
#endif //HTTP_FEATURE 
#ifdef MDNS_FEATURE
    if(WiFi.getMode() != WIFI_AP) {
        // Add service to MDNS-SD
        log_esp3d("Add mdns service http / tcp port %d", HTTP_Server::port());
        MDNS.addService("http", "tcp", HTTP_Server::port());
        //ESP3D service
        //TODO list all services available (http/tcp/ws/ftp/webdav/etc...)
        MDNS.addService("esp3d", "tcp", HTTP_Server::port());
        MDNS.addServiceTxt("esp3d", "tcp", "version", FW_VERSION);
        //Add TXT records
        MDNS.addServiceTxt("http", "tcp", "ESP3D", FW_VERSION);
    }
#endif //MDNS_FEATURE
#ifdef SSDP_FEATURE
    //SSDP service presentation
    if(WiFi.getMode() != WIFI_AP && HTTP_Server::started()) {
        //Add specific for SSDP
        String stmp = String(Hal::getChipID());
        SSDP.setSchemaURL ("description.xml");
        SSDP.setHTTPPort (HTTP_Server::port());
        SSDP.setName (hostname.c_str());
        SSDP.setURL ("/");
        SSDP.setDeviceType ("upnp:rootdevice");
        SSDP.setSerialNumber (stmp.c_str());
        //Any customization could be here
        SSDP.setModelName (ESP_MODEL_NAME);
        SSDP.setModelURL (ESP_MODEL_URL);
        SSDP.setModelNumber (ESP_MODEL_NUMBER);
        SSDP.setManufacturer (ESP_MANUFACTURER_NAME);
        SSDP.setManufacturerURL (ESP_MANUFACTURER_URL);
        SSDP.begin();
        stmp = "SSDP started with '" + hostname + "'";
        if (Settings_ESP3D::isVerboseBoot()) {
            output.printMSG(stmp.c_str());
        }
    }
#endif //SSDP_FEATURE
#ifdef NOTIFICATION_FEATURE
    notificationsservice.begin();
    notificationsservice.sendAutoNotification(NOTIFICATION_ESP_ONLINE);
#endif //NOTIFICATION_FEATURE
#ifdef CAMERA_DEVICE
    if (!esp3d_camera.begin()) {
        output.printMSG("Failed start camera streaming server");
    }
#endif //CAMERA_DEVICE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    MKSService::begin();
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
    if (!res) {
        end();
    }
    Hal::wait(1000);
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
    output.printMSG(NetConfig::localIP().c_str());
#endif //#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    _started = res;
    return _started;
}
void NetServices::end()
{
    _restart = false;
    if(!_started) {
        return;
    }
    _started = false;
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    MKSService::end();
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef CAMERA_DEVICE
    esp3d_camera.end();
#endif //CAMERA_DEVICE
#ifdef NOTIFICATION_FEATURE
    notificationsservice.end();
#endif //NOTIFICATION_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
    if(NetConfig::getMode() == ESP_AP_SETUP) {
        dnsServer.stop();
    }
#endif //CAPTIVE_PORTAL_FEATURE
#ifdef SSDP_FEATURE
#if defined(ARDUINO_ARCH_ESP32)
    SSDP.end();
#endif //ARDUINO_ARCH_ESP32
#endif //SSDP_FEATURE
#ifdef MDNS_FEATURE
    if(WiFi.getMode() != WIFI_AP) {
#if defined(ARDUINO_ARCH_ESP8266)
        String hostname = Settings_ESP3D::read_string(ESP_HOSTNAME);
        hostname.toLowerCase();
        log_esp3d("Remove mdns for %s", hostname.c_str());
        if (!MDNS.removeService(hostname.c_str(),"http", "tcp")) {
            log_esp3d("failed");
        }
        if (!MDNS.removeService(hostname.c_str(),"esp3d", "tcp")) {
            log_esp3d("failed");
        }
#endif // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        mdns_service_remove("_http", "_tcp");
        mdns_service_remove("_esp3d", "_tcp");
#endif // ARDUINO_ARCH_ESP32
        MDNS.end();
    }
#endif //MDNS_FEATURE

#ifdef OTA_FEATURE
#if defined(ARDUINO_ARCH_ESP32)
    if(WiFi.getMode() != WIFI_AP) {
        ArduinoOTA.end();
    }
#endif // ARDUINO_ARCH_ESP32
#endif //OTA_FEATURE
#if defined(HTTP_FEATURE)
    websocket_terminal_server.end();
#endif //HTTP_FEATURE
#ifdef WEBDAV_FEATURE
    webdav_server.end();
#endif //WEBDAV_FEATURE
#ifdef HTTP_FEATURE
    HTTP_Server::end();
#endif //HTTP_FEATURE
#ifdef WS_DATA_FEATURE
    websocket_data_server.end();
#endif //WS_DATA_FEATURE
#ifdef TELNET_FEATURE
    telnet_server.end();
#endif //TELNET_FEATURE
#ifdef FTP_FEATURE
    ftp_server.end();
#endif //FTP_FEATURE
}

void NetServices::handle()
{
    if (_started) {
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
        MKSService::handle();
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef MDNS_FEATURE
#if defined(ARDUINO_ARCH_ESP8266)
        MDNS.update();
#endif //ARDUINO_ARCH_ESP8266
#endif //MDNS_FEATURE
#ifdef OTA_FEATURE
        ArduinoOTA.handle();
#endif //OTA_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
        if (NetConfig::getMode() == ESP_AP_SETUP) {
            dnsServer.processNextRequest();
        }
#endif //CAPTIVE_PORTAL_FEATURE
#ifdef HTTP_FEATURE
        HTTP_Server::handle();
#endif //HTTP_FEATURE
#ifdef WEBDAV_FEATURE
        webdav_server.handle();
#endif //WEBDAV_FEATURE
#ifdef WS_DATA_FEATURE
        websocket_data_server.handle();
#endif //WS_DATA_FEATURE
#if defined(HTTP_FEATURE)
        websocket_terminal_server.handle();
#endif //HTTP_FEATURE
#ifdef TELNET_FEATURE
        telnet_server.handle();
#endif //TELNET_FEATURE
#ifdef FTP_FEATURE
        ftp_server.handle();
#endif //FTP_FEATURE
#ifdef NOTIFICATION_FEATURE
        notificationsservice.handle();
#endif //NOTIFICATION_FEATURE
    }
    if (_restart) {
        begin();
    }
}

