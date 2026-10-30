/*
  usb_cdc_port.h -  native USB CDC port setup, shared by log and client

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

#pragma once
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32) && ARDUINO_USB_CDC_ON_BOOT
// With ARDUINO_USB_CDC_ON_BOOT, Serial is the native USB CDC:
// HWCDC (USB-Serial-JTAG) if ARDUINO_USB_MODE=1, USBCDC (TinyUSB) otherwise
// Serial0 stays UART0
#define ESP3D_USB_CDC_PORT Serial

#ifndef ESP3D_USB_CDC_RX_BUFFER_SIZE
#define ESP3D_USB_CDC_RX_BUFFER_SIZE 512
#endif  // ESP3D_USB_CDC_RX_BUFFER_SIZE

#ifndef ESP3D_USB_CDC_TX_TIMEOUT_MS
#define ESP3D_USB_CDC_TX_TIMEOUT_MS 10
#endif  // ESP3D_USB_CDC_TX_TIMEOUT_MS

// Can be called several times, setup is done only once
inline void esp3d_usb_cdc_port_begin() {
  static bool started = false;
  if (started) {
    return;
  }
  started = true;
  ESP3D_USB_CDC_PORT.setRxBufferSize(ESP3D_USB_CDC_RX_BUFFER_SIZE);
  // TinyUSB CDC is already started by core, HWCDC is not
  // begin() is safe to call in both cases
  ESP3D_USB_CDC_PORT.begin();
  // short timeout so writes do not block when host does not read the port
  // do not use 0: HWCDC uses it as retry counter (0 - 1 => wrap => hang),
  // with a small value HWCDC detects the stall and drops data instead
  ESP3D_USB_CDC_PORT.setTxTimeoutMs(ESP3D_USB_CDC_TX_TIMEOUT_MS);
}
#endif  // ARDUINO_ARCH_ESP32 && ARDUINO_USB_CDC_ON_BOOT
