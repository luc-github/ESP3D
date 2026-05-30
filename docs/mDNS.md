# mDNS Services

ESP3D uses mDNS (DNS-SD) for network discovery. Services are registered at boot via `addESP3DServices()` in [mDNS.cpp](../esp3d/src/modules/mDNS/mDNS.cpp).

Requires `MDNS_FEATURE` to be defined. Works independently of HTTP — any active network service (Telnet, WebSocket, etc.) is sufficient.

---

## Registered services

| Service | Type | Port | Condition |
|---------|------|------|-----------|
| `_esp3d._tcp` | TCP | first active service port | always |
| `_http._tcp` | TCP | HTTP port | `HTTP_FEATURE` |
| `_ftp._tcp` | TCP | FTP control port | `FTP_FEATURE` |
| `_telnet._tcp` | TCP | Telnet port | `TELNET_FEATURE` |
| `_webdav._tcp` | TCP | WebDAV port | `WEBDAV_FEATURE` |
| `_websocket._tcp` | TCP | WebSocket port | `WS_DATA_FEATURE` |
| `_device-info._tcp` | TCP | first active service port | always (see constraints) |

### Port determination

The main port (used by `_esp3d` and `_device-info`) is auto-detected in this priority order:  
**HTTP → FTP → Telnet → WebDAV → WebSocket → 8080 (fallback)**

A specific port can be forced by calling `addESP3DServices(port)` directly.

> **Note:** The ESP Arduino mDNS stack rejects port 0 at service registration even though DNS-SD standard allows it. Port 8080 is used as last-resort fallback.

---

## `_esp3d._tcp` TXT records

| Key | Value | Source |
|-----|-------|--------|
| `firmware` | `ESP3D` or `ESP3DLib` | compile-time |
| `version` | e.g. `3.1.1` | compile-time |

---

## `_websocket._tcp` TXT records

| Key | Value |
|-----|-------|
| `uri` | `/` |
| `subprotocol` | `arduino` |

---

## `_device-info._tcp` TXT records

This service provides device identification and capability discovery. It carries no real TCP endpoint — its port mirrors the main service port.

### Fixed records (always present on ESP32)

| Key | Value | Source |
|-----|-------|--------|
| `name` | device hostname | runtime |
| `board` | e.g. `ESP32`, `ESP32-S3` | `ESP3DSettings::TargetBoard()` |
| `chip` | e.g. `ESP32-S3` *(ESP32 only)* | `ESP.getChipModel()` |
| `fw` | `ESP3D` or `ESP3DLib` | compile-time |
| `ver` | e.g. `3.1.1` | compile-time |
| `target` | `marlin`, `grbl`, `smoothieware`, `repetier`, `unknown` | runtime (settings) |
| `sdk` | e.g. `v5.1.2` | `ESP.getSdkVersion()` |
| `core` | Arduino core version | `ESP3DHal::arduinoVersion()` |
| `flash` | e.g. `4.00 MB` | `ESP.getFlashChipSize()` |
| `auth` | `enabled` or `disabled` | compile-time |
| `log` | e.g. `serial0/debug`, `telnet/verbose`, `none` | compile-time |

> `auth` and `log` are always present — `auth` because its absence would silently hide a security configuration, `log` to allow quick diagnosis of communication issues without needing physical access.

### Conditional records (absent = feature not compiled)

| Key | Value | Condition | Type |
|-----|-------|-----------|------|
| `psram` | e.g. `8.00 MB` | ESP32 + `BOARD_HAS_PSRAM` | compile-time |
| `fs` | e.g. `1.44 MB` | `FILESYSTEM_FEATURE` | compile-time |
| `notification` | `telegram`, `email`, `pushover`, `ifttt`, `homeassistant`, `whatsapp`, `none` | `NOTIFICATION_FEATURE` | runtime (settings) |
| `camera` | e.g. `ESP32 Cam`, `ESP Eye`, `M5Stack` | `CAMERA_DEVICE` | compile-time |
| `bt` | `hostname/AA:BB:CC:DD:EE:FF` | `BLUETOOTH_FEATURE` | runtime |
| `sensor` | `DHT11`, `DHT22`, `ANALOG`, `BMP280`, `BME280` | `SENSOR_DEVICE` | runtime (settings) |
| `sd-update` | `ON` or `OFF` | `SD_UPDATE_FEATURE` | runtime (settings) |
| `web-update` | `enabled` | `WEB_UPDATE_FEATURE` | compile-time |
| `time` | `ntp` or `manual` | `TIMESTAMP_FEATURE` | runtime (settings) |
| `sd` | `SPI`, `SDIO`, `SPI-SdFat` | `SD_DEVICE` | compile-time |
| `lua` | `enabled` | `LUA_INTERPRETER_FEATURE` | compile-time |
| `usb` | `enabled` | `USB_SERIAL_FEATURE` | compile-time |
| `ssdp` | `enabled` | `SSDP_FEATURE` | compile-time |

---

## ESP8266 constraints

On ESP8266, the `_device-info._tcp` service is registered with **one record only** (`board`):

| Key | Value |
|-----|-------|
| `board` | `ESP82XX` |

**Reason:** The ESP8266 has ~40-50 KB of free heap at runtime. Registering many TXT records exhausts available memory, causing OOM crashes in the mDNS packet parser (`MDNSResponder::_readRRAnswer`) when processing the first incoming mDNS packet after boot.

---

## Related commands

| Command | Description |
|---------|-------------|
| `[ESP420]` | Show device status including mDNS state and log output |
| `[ESP450]` | Query mDNS network for other `_esp3d._tcp` services |
