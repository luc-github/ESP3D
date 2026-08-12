/*
 handle-marlin-bft.cpp - Marlin Binary File Transfer HTTP API

 Copyright (c) 2026 ESP3D contributors

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
*/

#include "../../../include/esp3d_config.h"

#if defined(HTTP_FEATURE) && defined(MARLIN_BINARY_FILE_TRANSFER_FEATURE)

#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif

#include "../http_server.h"
#include "../../authentication/authentication_service.h"
#include "../../marlin_bft/marlin_bft_service.h"

void HTTP_Server::handleMarlinBft() {
  set_http_headers();
  if (AuthenticationService::getAuthenticatedLevel() ==
      ESP3DAuthenticationLevel::guest) {
    _webserver->send(401, "application/json",
                     "{\"status\":\"error\",\"error\":\"Wrong authentication\"}");
    return;
  }

  String action = _webserver->hasArg("action")
                      ? _webserver->arg("action")
                      : "status";
  action.toLowerCase();

  if (action == "start") {
    if (_webserver->method() != HTTP_POST) {
      _webserver->send(405, "application/json",
                       "{\"status\":\"error\",\"error\":\"Use POST to start a transfer\"}");
      return;
    }
    if (!_webserver->hasArg("source") ||
        !_webserver->hasArg("destination")) {
      _webserver->send(400, "application/json",
                       "{\"status\":\"error\",\"error\":\"source and destination are required\"}");
      return;
    }
    const String compression = _webserver->hasArg("compression")
                                   ? _webserver->arg("compression")
                                   : "auto";
    const bool dummy = _webserver->hasArg("dummy") &&
                       _webserver->arg("dummy") == "true";
    const bool started = marlin_bft_service.start(
        _webserver->arg("source").c_str(),
        _webserver->arg("destination").c_str(), compression.c_str(), dummy);
    _webserver->send(started ? 202 : 409, "application/json",
                     marlin_bft_service.statusJson());
    return;
  }
  if (action == "cancel") {
    if (_webserver->method() != HTTP_POST) {
      _webserver->send(405, "application/json",
                       "{\"status\":\"error\",\"error\":\"Use POST to cancel a transfer\"}");
      return;
    }
    marlin_bft_service.cancel();
    _webserver->send(202, "application/json",
                     marlin_bft_service.statusJson());
    return;
  }
  if (action != "status") {
    _webserver->send(400, "application/json",
                     "{\"status\":\"error\",\"error\":\"Unknown action\"}");
    return;
  }
  _webserver->send(200, "application/json", marlin_bft_service.statusJson());
}

#endif  // HTTP_FEATURE && MARLIN_BINARY_FILE_TRANSFER_FEATURE
