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

#include "netservices.h"

#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../include/esp3d_config.h"
#include "netconfig.h"

#ifdef MDNS_FEATURE
#include "../mDNS/mDNS.h"
#endif  // MDNS_FEATURE
#if defined(ARDUINO_ARCH_ESP8266)
#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif  // SSDP_FEATURE
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#ifdef SSDP_FEATURE
#include <ESP32SSDP.h>
#endif  // SSDP_FEATURE
#endif  // ARDUINO_ARCH_ESP32
#ifdef OTA_FEATURE
#include <ArduinoOTA.h>
#endif  // OTA_FEATURE
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#ifdef TELNET_FEATURE
#include "../telnet/telnet_server.h"
#endif  // TELNET_FEATURE
#ifdef FTP_FEATURE
#include "../ftp/FtpServer.h"
#endif  // FP_FEATURE
#ifdef WEBDAV_FEATURE
#include "../webdav/webdav_server.h"
#endif  // WEBDAV_FEATURE
#ifdef HTTP_FEATURE
#include "../http/http_server.h"
#endif  // HTTP_FEATURE
#if defined(HTTP_FEATURE) || defined(WS_DATA_FEATURE)
#include "../websocket/websocket_server.h"
#endif  // HTTP_FEATURE || WS_DATA_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif  // CAPTIVE_PORTAL_FEATURE
#ifdef TIMESTAMP_FEATURE
#include "../time/time_service.h"
#endif  // TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
#include "../notifications/notifications_service.h"
#endif  // NOTIFICATION_FEATURE
#ifdef CAMERA_DEVICE
#include "../camera/camera.h"
#endif  // CAMERA_DEVICE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

bool NetServices::_started = false;
bool NetServices::_restart = false;

bool NetServices::begin() {
  bool res = true;
  _started = false;
  String hostname = ESP3DSettings::readString(ESP_HOSTNAME);
  end();
#ifdef TIMESTAMP_FEATURE
  if (WiFi.getMode() != WIFI_AP) {
    if (!timeService.begin()) {
      if (timeService.isInternetTime()) {
        esp3d_commands.dispatch(
            "Failed contact time servers!", ESP3DClientType::all_clients, no_id,
            ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
        esp3d_log_e("Failed contact time servers!");
      }
    } else {
      String tmp = "Current time :";
      tmp += timeService.getCurrentTime();
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(tmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // TIMESTAMP_FEATURE

#if defined(MDNS_FEATURE) && defined(ARDUINO_ARCH_ESP8266)
  esp3d_mDNS.begin(hostname.c_str());
#endif  // MDNS_FEATURE && ARDUINO_ARCH_ESP8266

#ifdef OTA_FEATURE
  if (WiFi.getMode() != WIFI_AP) {
    ArduinoOTA.onStart([]() {
      String type = "Start OTA updating ";
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type += "sketch";
      } else {  // U_SPIFFS or any FS
        // NOTE: if updating FS this would be the place to unmount FS using
        // FS.end()
        type += "filesystem";
#if defined(FILESYSTEM_FEATURE)
        ESP_FileSystem::end();
#endif  // FILESYSTEM_FEATURE
      }
      esp3d_commands.dispatch(type.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    });
    ArduinoOTA.onEnd([]() {
      esp3d_commands.dispatch("End OTA", ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      String prg = "OTA Progress ";
      prg += String(progress / (total / 100)) + "%";
      esp3d_commands.dispatch(prg.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    });
    ArduinoOTA.onError([](ota_error_t error) {
      String stmp = "OTA Error: " + String(error);
      esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
      if (error == OTA_AUTH_ERROR) {
        esp3d_commands.dispatch("Auth Failed", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
        esp3d_log_e("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        esp3d_log_e("Begin Failed");
        esp3d_commands.dispatch("Begin Failed", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      } else if (error == OTA_CONNECT_ERROR) {
        esp3d_log_e("Connect Failed");
        esp3d_commands.dispatch("Connect Failed", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      } else if (error == OTA_RECEIVE_ERROR) {
        esp3d_log_e("Receive Failed");
        esp3d_commands.dispatch("Receive Failed", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      } else if (error == OTA_END_ERROR) {
        esp3d_log_e("End Failed");
        esp3d_commands.dispatch("End Failed", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    });
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch("OTA service started",
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
    String lhostname = hostname;
    lhostname.toLowerCase();
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.begin();
  }
#endif

#if defined(MDNS_FEATURE) && defined(ARDUINO_ARCH_ESP32)
  esp3d_mDNS.begin(hostname.c_str());
#endif  // MDNS_FEATURE && ARDUINO_ARCH_ESP8266

#ifdef CAPTIVE_PORTAL_FEATURE
  if (NetConfig::getMode() == ESP_AP_SETUP) {
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    if (dnsServer.start(DNS_PORT, "*", WiFi.softAPIP())) {
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(
            "Captive Portal started", ESP3DClientType::all_clients, no_id,
            ESP3DMessageType::unique, ESP3DClientType::system,
            ESP3DAuthenticationLevel::admin);
      }
    } else {
      esp3d_log_e("Failed start Captive Portal");
      esp3d_commands.dispatch("Failed start Captive Portal",
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
  }
#endif  // CAPTIVE_PORTAL_FEATURE

#ifdef HTTP_FEATURE
  if (!HTTP_Server::begin()) {
    res = false;
    esp3d_log_e("HTTP server failed");
    esp3d_commands.dispatch("HTTP server failed", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    if (HTTP_Server::started()) {
      String stmp = "HTTP server started port " + String(HTTP_Server::port());
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
  if (!telnet_server.begin()) {
    res = false;
    esp3d_log_e("Telnet server failed");
    esp3d_commands.dispatch("Telnet server failed",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    if (telnet_server.started()) {
      String stmp =
          "Telnet server started port " + String(telnet_server.port());
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // TELNET_FEATURE
#ifdef FTP_FEATURE
  if (!ftp_server.begin()) {
    res = false;
    esp3d_log_e("Ftp server failed");
    esp3d_commands.dispatch("Ftp server failed", ESP3DClientType::all_clients,
                            no_id, ESP3DMessageType::unique,
                            ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    if (ftp_server.started()) {
      String stmp =
          "Ftp server started ports: " + String(ftp_server.ctrlport()) + "," +
          String(ftp_server.dataactiveport()) + "," +
          String(ftp_server.datapassiveport());
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // FTP_FEATURE
#ifdef WS_DATA_FEATURE
  if (!websocket_data_server.begin(
          ESP3DSettings::readUint32(ESP_WEBSOCKET_PORT))) {
    esp3d_log_e("Failed start Terminal Web Socket");
    esp3d_commands.dispatch("Failed start Terminal Web Socket",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    if (websocket_data_server.started()) {
      String stmp = "Websocket server started port " +
                    String(websocket_data_server.port());
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // WS_DATA_FEATURE
#ifdef WEBDAV_FEATURE
  if (!webdav_server.begin()) {
    esp3d_log("Failed start Terminal Web Socket");
    esp3d_commands.dispatch("Failed start Terminal Web Socket",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  } else {
    if (webdav_server.started()) {
      String stmp =
          "WebDav server started port " + String(webdav_server.port());
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
    }
  }
#endif  // WEBDAV_FEATURE
#if defined(HTTP_FEATURE)
  if (!websocket_terminal_server.begin()) {
    esp3d_log_e("Failed start Terminal Web Socket");
    esp3d_commands.dispatch("Failed start Terminal Web Socket",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
#endif  // HTTP_FEATURE
#if defined(MDNS_FEATURE) && defined(HTTP_FEATURE)
  esp3d_mDNS.addESP3DServices(HTTP_Server::port());
#endif  // MDNS_FEATURE
#if defined(MDNS_FEATURE) && defined(HTTP_FEATURE)
#ifdef SSDP_FEATURE
  // SSDP service presentation
  if (WiFi.getMode() != WIFI_AP && HTTP_Server::started()) {
    // Add specific for SSDP
    String stmp = String(ESP3DHal::getChipID());
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(HTTP_Server::port());
    SSDP.setName(hostname.c_str());
    SSDP.setURL("/");
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setSerialNumber(stmp.c_str());
    // Any customization could be here
    SSDP.setModelName(ESP_MODEL_NAME);
#if defined(ESP_MODEL_DESCRIPTION)
    // this one is optional because windows doesn't care about this field
    SSDP.setModelDescription(ESP_MODEL_DESCRIPTION);
#endif  // ESP_MODEL_DESCRIPTION
    SSDP.setModelURL(ESP_MODEL_URL);
    SSDP.setModelNumber(ESP_MODEL_NUMBER);
    SSDP.setManufacturer(ESP_MANUFACTURER_NAME);
    SSDP.setManufacturerURL(ESP_MANUFACTURER_URL);
    SSDP.begin();
    stmp = "SSDP started with '" + hostname + "'";
    if (ESP3DSettings::isVerboseBoot()) {
      esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    }
  }
#endif  // defined(MDNS_FEATURE) && defined(HTTP_FEATURE)
#endif  // MDNS_FEATURE
#ifdef NOTIFICATION_FEATURE
  notificationsservice.begin();
  notificationsservice.sendAutoNotification(NOTIFICATION_ESP_ONLINE);
#endif  // NOTIFICATION_FEATURE
#ifdef CAMERA_DEVICE
  if (!esp3d_camera.begin()) {
    esp3d_log_e("Failed start camera streaming server");
    esp3d_commands.dispatch("Failed start camera streaming server",
                            ESP3DClientType::all_clients, no_id,
                            ESP3DMessageType::unique, ESP3DClientType::system,
                            ESP3DAuthenticationLevel::admin);
  }
#endif  // CAMERA_DEVICE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  MKSService::begin();
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
  if (!res) {
    end();
  }
  ESP3DHal::wait(1000);
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
  esp3d_commands.dispatch(NetConfig::localIP().c_str(),
                          ESP3DClientType::all_clients, no_id,
                          ESP3DMessageType::unique, ESP3DClientType::system,
                          ESP3DAuthenticationLevel::admin);
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
  _started = res;
  return _started;
}
void NetServices::end() {
  _restart = false;
  if (!_started) {
    return;
  }
  _started = false;
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  MKSService::end();
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef CAMERA_DEVICE
  esp3d_camera.end();
#endif  // CAMERA_DEVICE
#ifdef NOTIFICATION_FEATURE
  notificationsservice.end();
#endif  // NOTIFICATION_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
  if (NetConfig::getMode() == ESP_AP_SETUP) {
    dnsServer.stop();
  }
#endif  // CAPTIVE_PORTAL_FEATURE
#ifdef SSDP_FEATURE
#if defined(ARDUINO_ARCH_ESP32)
  SSDP.end();
#endif  // ARDUINO_ARCH_ESP32
#endif  // SSDP_FEATURE
#ifdef MDNS_FEATURE
  esp3d_mDNS.end();
#endif  // MDNS_FEATURE

#ifdef OTA_FEATURE
#if defined(ARDUINO_ARCH_ESP32)
  if (WiFi.getMode() != WIFI_AP) {
    ArduinoOTA.end();
  }
#endif  // ARDUINO_ARCH_ESP32
#endif  // OTA_FEATURE
#if defined(HTTP_FEATURE)
  websocket_terminal_server.end();
#endif  // HTTP_FEATURE
#ifdef WEBDAV_FEATURE
  webdav_server.end();
#endif  // WEBDAV_FEATURE
#ifdef HTTP_FEATURE
  HTTP_Server::end();
#endif  // HTTP_FEATURE
#ifdef WS_DATA_FEATURE
  websocket_data_server.end();
#endif  // WS_DATA_FEATURE
#ifdef TELNET_FEATURE
  telnet_server.end();
#endif  // TELNET_FEATURE
#ifdef FTP_FEATURE
  ftp_server.end();
#endif  // FTP_FEATURE
}

void NetServices::handle() {
  if (_started) {
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    MKSService::handle();
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef MDNS_FEATURE
    esp3d_mDNS.handle();
#endif  // MDNS_FEATURE
#ifdef OTA_FEATURE
    ArduinoOTA.handle();
#endif  // OTA_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
    if (NetConfig::getMode() == ESP_AP_SETUP) {
      dnsServer.processNextRequest();
    }
#endif  // CAPTIVE_PORTAL_FEATURE
#ifdef HTTP_FEATURE
    HTTP_Server::handle();
#endif  // HTTP_FEATURE
#ifdef WEBDAV_FEATURE
    webdav_server.handle();
#endif  // WEBDAV_FEATURE
#ifdef WS_DATA_FEATURE
    websocket_data_server.handle();
#endif  // WS_DATA_FEATURE
#if defined(HTTP_FEATURE)
    websocket_terminal_server.handle();
#endif  // HTTP_FEATURE
#ifdef TELNET_FEATURE
    telnet_server.handle();
#endif  // TELNET_FEATURE
#ifdef FTP_FEATURE
    ftp_server.handle();
#endif  // FTP_FEATURE
#ifdef NOTIFICATION_FEATURE
    notificationsservice.handle();
#endif  // NOTIFICATION_FEATURE
#if defined(TIMESTAMP_FEATURE) && \
    (defined(ESP_GOT_IP_HOOK) || defined(ESP_GOT_DATE_TIME_HOOK))
    timeService.handle();
#endif  // TIMESTAMP_FEATURE
  }
  if (_restart) {
    begin();
  }
}
