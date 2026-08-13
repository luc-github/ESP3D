/*
  printer_link_service.h - Exclusive access to the printer connection

  Copyright (c) 2026 ESP3D contributors

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "../../include/esp3d_config.h"

using PrinterLinkRxHandler =
    void (*)(const uint8_t *data, size_t size, void *context);

class PrinterLinkService final {
 public:
  // Capture all printer RX and block normal printer TX. The owner string is
  // also exposed to the WebUI. A null handler captures and discards RX.
  bool acquire(const char *owner, PrinterLinkRxHandler handler = nullptr,
               void *context = nullptr);

  // Release only when the caller supplies the owner used by acquire().
  bool release(const char *owner);

  bool captured() const { return _captured; }
  const char *owner() const { return _owner.c_str(); }

  // Returns true when RX belongs to the current exclusive owner.
  bool consumeRx(const uint8_t *data, size_t size);

  // Send current state to one newly connected WebUI client.
  void notifyWebUi(uint8_t clientId) const;

 private:
  void notifyWebUi() const;
  String eventMessage() const;

  volatile bool _captured = false;
  String _ownerKey;
  String _owner;
  PrinterLinkRxHandler _handler = nullptr;
  void *_context = nullptr;
};

extern PrinterLinkService printer_link_service;
