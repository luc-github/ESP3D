/*
  printer_link_service.cpp - Exclusive access to the printer connection

  Copyright (c) 2026 ESP3D contributors

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#include "printer_link_service.h"

#if defined(HTTP_FEATURE)
#include "../websocket/websocket_server.h"
#endif

PrinterLinkService printer_link_service;

bool PrinterLinkService::acquire(const char *owner,
                                 PrinterLinkRxHandler handler,
                                 void *context) {
  if (!owner || !owner[0] || _captured) return false;

  _ownerKey = owner;
  _owner = owner;
  _owner.trim();
  _owner.replace(":", "_");
  _owner.replace("\r", "_");
  _owner.replace("\n", "_");
  if (!_owner.length()) {
    _ownerKey = "";
    return false;
  }
  if (_owner.length() > 32) _owner.remove(32);

  _handler = handler;
  _context = context;
  _captured = true;
  notifyWebUi();
  return true;
}

bool PrinterLinkService::release(const char *owner) {
  if (!_captured || !owner || _ownerKey != owner) return false;

  // Stop diverting RX before invalidating the callback and its context.
  _captured = false;
  _handler = nullptr;
  _context = nullptr;
  _ownerKey = "";
  _owner = "";
  notifyWebUi();
  return true;
}

bool PrinterLinkService::consumeRx(const uint8_t *data, size_t size) {
  if (!_captured) return false;
  if (_handler && data && size) _handler(data, size, _context);
  return true;
}

String PrinterLinkService::eventMessage() const {
  if (_captured) return "printerLink:captured:" + _owner;
  return "printerLink:released";
}

void PrinterLinkService::notifyWebUi() const {
#if defined(HTTP_FEATURE)
  if (websocket_terminal_server.started())
    websocket_terminal_server.pushMSG(eventMessage().c_str());
#endif
}

void PrinterLinkService::notifyWebUi(uint8_t clientId) const {
#if defined(HTTP_FEATURE)
  if (websocket_terminal_server.started())
    websocket_terminal_server.pushMSG(clientId, eventMessage().c_str());
#else
  (void)clientId;
#endif
}
