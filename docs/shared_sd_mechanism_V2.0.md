# SD Card Sharing Mechanism — V2.2

Implementation reference for the shared SD mechanism.
For the V1.0 ESP3D mechanism, see [shared_sd_mechanism_V1.0.md](shared_sd_mechanism_V1.0.md).
For the design discussion and rationale, see [discussion_shared_sd_mechanism.md](discussion_shared_sd_mechanism.md).

## Changelog V2.0 → V2.2

| Issue | Change |
|-------|--------|
| ISSUE-1 (critical) | Added `SDActionFn` type and `setMCUReleaseCallback` / `setMCURemountCallback` callbacks |
| ISSUE-2 (high) | FYSETC SPI isolation now registered via `setMCUReleaseCallback` / `setMCURemountCallback` |
| ISSUE-3 (medium) | `FAILED` state removed from diagram — not implemented, not needed |
| ISSUE-4 (medium) | `nullptr` callback logs a warning; restrictive enforcement recommended for future |
| ISSUE-5 (medium) | Noted coexistence of `_state` and `_sharedState` |
| ISSUE-6 (low) | Combined CS Sense + firmware callback example added |
| Accuracy | Constant names updated to match actual `board_config.h` values |
| Accuracy | `_beginShared()` documented (was missing from V2.0) |
| Accuracy | Watchdog integrated in `accessFS()` (not a separate `handle()`) |

## Changelog V2.1 → V2.2

| Point | Change |
|-------|--------|
| Race condition (code + doc) | `_sharedState = ACQUIRING` déplacé **avant** `_mcuReleaseCallback()` — ferme la fenêtre de concurrence quand le callback est bloquant (Option B UART) |
| Watchdog limitation (doc) | Note ajoutée : le watchdog ne se déclenche que sur le prochain `accessFS()` |
| CS Sense overwrite (doc) | Note ajoutée sur `setMCUBusyCallback()` et en section 10 : les callbacks ne s'accumulent pas, `setMCUBusyCallback()` écrase l'auto-registration de `_beginShared()` |

---

## Table of Contents

1. [Design Principles](#1-design-principles)
2. [State Model](#2-state-model)
3. [Configuration Defines](#3-configuration-defines)
4. [Header — Types and Declarations](#4-header--types-and-declarations)
5. [`_beginShared()`](#5-_beginshared)
6. [`_enableSharedSD()`](#6-_enablesharedsd)
7. [`_disableSharedSD()`](#7-_disablesharedsd)
8. [`accessFS()`](#8-accessfs)
9. [`releaseFS()`](#9-releasefs)
10. [Firmware Integration — Callbacks](#10-firmware-integration--callbacks)
11. [grblHAL Plugin — STM32 + ESP32-C3](#11-grblhal-plugin--stm32--esp32-c3)
12. [Implementation Status](#12-implementation-status)

---

## 1. Design Principles

1. **Single source of truth**: one state enum fully describes SD bus ownership.
2. **Atomic transitions**: software state and GPIO state are always changed together.
3. **Safe by default**: any transition to ESP is refused if the MCU is active, regardless of firmware.
4. **Self-recovery**: a watchdog in `accessFS()` forces release if an access is held too long.
5. **Firmware extensibility**: all MCU interactions are registered callbacks — no `#ifdef` firmware type in the sharing logic.
6. **Three-callback model**: query (is MCU busy?), release hook (tell MCU to unmount), remount hook (tell MCU to remount). Each is independent and optional.
7. **Hardware isolation via callbacks**: board-specific SPI bus teardown/reinit (FYSETC) is registered as release/remount callbacks, not hardcoded in the sharing logic.

---

## 2. State Model

```
                     ┌─────────────────────────────────────────┐
                     │                                         │
                     ▼                                         │
              ┌─────────────┐  _enableSharedSD()  ┌──────────────┐
  boot ──────►│  MCU_OWNS   │───────────────────►│  ACQUIRING   │
              │ GPIO→MCU    │                    │ (transition) │
              └─────────────┘                   └──────┬───────┘
                     ▲                                 │ ok
                     │                                 ▼
                     │                   ┌──────────────────────┐
                     │ _disableSharedSD() │      ESP_OWNS        │
                     │◄───────────────── │  GPIO→ESP, BUSY      │
                     │                   └──────────────────────┘
                     │                            │
                     │                watchdog    │ timeout → _disableSharedSD()
                     └────────────────────────────┘
```

**Boot state:** `MCU_OWNS` — GPIO set to idle value, SD belongs to the MCU.

There is no persistent `FAILED` state. Failed acquisitions (`_enableSharedSD()` returns false) leave `_sharedState` in `MCU_OWNS` — the safe default.

---

## 3. Configuration Defines

All defines live in `board_config.h` for the active target board.

```cpp
/* Multiplexer GPIO */
#define SD_SHARED_MUX_PIN           GPIO_NUM_26   // GPIO_NUM_NC to disable hardware switching
#define SD_SHARED_MUX_ESP_VALUE     0             // LOW  = ESP owns the SD bus
#define SD_SHARED_MUX_IDLE_VALUE    1             // HIGH = MCU owns the SD bus (explicit, never !)
#define SD_SHARED_SWITCH_DELAY_MS   10            // Multiplexer stabilization delay in ms
#define SD_SHARED_WATCHDOG_MS       30000         // Watchdog timeout in ms (0 = disabled)

/* CS Sense — passive MCU activity detection (FYSETC) */
#define SD_CS_SENSE_PIN             GPIO_NUM_32   // Omit or GPIO_NUM_NC if not present
#define SD_CS_SENSE_ACTIVE_VALUE    0             // LOW = MCU is asserting SD CS

/* SD Power control (FYSETC) */
#define SD_POWER_PIN                GPIO_NUM_27   // Omit or GPIO_NUM_NC if not present
#define SD_POWER_ACTIVE_VALUE       0             // LOW = power on

/* D1/D2 isolation — SDIO 4-bit lines held INPUT_PULLUP when MCU owns (FYSETC) */
#define SD_SHARED_D1_PIN            GPIO_NUM_4    // Omit or GPIO_NUM_NC if not present
#define SD_SHARED_D2_PIN            GPIO_NUM_12   // Omit or GPIO_NUM_NC if not present
```

The PiBot pendant sets `SD_SHARED_MUX_PIN = GPIO_NUM_NC` — the GPIO switch is skipped but
the software state machine and callbacks remain active.

### Naming: Arduino (V1.0) vs IDF (V2.x)

| Arduino (V1.0 — obsolete) | IDF (V2.x — current code) | Description |
|---------------------------|---------------------------|-------------|
| `ESP_FLAG_SHARED_SD_PIN` | `SD_SHARED_MUX_PIN` | GPIO driving the multiplexer |
| `ESP_FLAG_SHARED_SD_VALUE` | `SD_SHARED_MUX_ESP_VALUE` | Level = ESP owns the bus |
| `ESP_FLAG_SHARED_SD_IDLE_VALUE` | `SD_SHARED_MUX_IDLE_VALUE` | Level = MCU owns the bus |
| `ESP_SHARED_SD_SWITCH_DELAY_MS` | `SD_SHARED_SWITCH_DELAY_MS` | Mux stabilization delay |
| `ESP_SHARED_SD_WATCHDOG_MS` | `SD_SHARED_WATCHDOG_MS` | Watchdog timeout |

The `ESP_FLAG_` prefix was the Arduino ESP3D convention (V1.0). The IDF implementation uses
the `SD_SHARED_*` prefix, consistent with the `SD_*` grouping in `board_config.h`.

---

## 4. Header — Types and Declarations

```cpp
#if ESP3D_SD_SHARED_FEATURE

// Passive MCU busy query — return true if MCU is currently using the SD.
using IsMCUBusyFn = bool(*)();

// Active MCU lifecycle hooks.
// setMCUReleaseCallback: fired BEFORE the GPIO switch — tell MCU firmware to unmount SD.
// setMCURemountCallback: fired AFTER the GPIO is restored — tell MCU firmware to remount SD.
using SDActionFn = void(*)();

// Replaces any previously registered busy callback (including the CS Sense
// auto-registration from _beginShared()). Callbacks do not accumulate.
void setMCUBusyCallback(IsMCUBusyFn cb);
void setMCUReleaseCallback(SDActionFn cb);
void setMCURemountCallback(SDActionFn cb);

bool isSharedEnabled() const { return _sharedState == SharedSDState::ESP_OWNS; }

private:
  enum class SharedSDState : uint8_t { MCU_OWNS, ACQUIRING, ESP_OWNS };
  bool _beginShared();
  bool _enableSharedSD();
  bool _disableSharedSD();
  SharedSDState _sharedState;
  uint64_t      _acquireTimestamp;      // µs from esp_timer_get_time() / 1000
  IsMCUBusyFn   _mcuBusyCallback;
  SDActionFn    _mcuReleaseCallback;
  SDActionFn    _mcuRemountCallback;

#endif  // ESP3D_SD_SHARED_FEATURE
```

> **Note on `_state` vs `_sharedState`** (ISSUE-5): `_state` (idle/busy/not_present/unknown)
> and `_sharedState` (MCU_OWNS/ACQUIRING/ESP_OWNS) coexist and are set independently.
> In shared mode `_state == busy` and `_sharedState == ESP_OWNS` should always coincide.
> They are set by different functions (`accessFS` for `_state`, `_enableSharedSD` for
> `_sharedState`) — a desync is possible if code paths diverge. A future cleanup could
> unify or cross-assert them.

---

## 5. `_beginShared()`

Called once from `begin()`. Configures all GPIO for the shared mechanism and auto-registers
the CS sense callback if `SD_CS_SENSE_PIN` is defined.

```cpp
bool ESP3DSd::_beginShared() {
#if defined(SD_POWER_PIN) && SD_POWER_PIN != GPIO_NUM_NC
  {
    gpio_config_t pwr_conf = {};
    pwr_conf.pin_bit_mask = (1ULL << SD_POWER_PIN);
    pwr_conf.mode = GPIO_MODE_OUTPUT;
    pwr_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    pwr_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pwr_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&pwr_conf);
    gpio_set_level((gpio_num_t)SD_POWER_PIN, SD_POWER_ACTIVE_VALUE);
    esp3d_log("SharedSD: power pin=%d → on (value=%d)", SD_POWER_PIN, SD_POWER_ACTIVE_VALUE);
  }
#endif

#if defined(SD_CS_SENSE_PIN) && SD_CS_SENSE_PIN != GPIO_NUM_NC
  {
    // Auto-register the CS sense busy callback.
    gpio_config_t sense_conf = {};
    sense_conf.pin_bit_mask = (1ULL << SD_CS_SENSE_PIN);
    sense_conf.mode = GPIO_MODE_INPUT;
    sense_conf.pull_up_en  = (SD_CS_SENSE_ACTIVE_VALUE == 0) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    sense_conf.pull_down_en = (SD_CS_SENSE_ACTIVE_VALUE == 1) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    sense_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&sense_conf);
    _mcuBusyCallback = []() -> bool {
      return gpio_get_level((gpio_num_t)SD_CS_SENSE_PIN) == SD_CS_SENSE_ACTIVE_VALUE;
    };
    esp3d_log("SharedSD: CS sense pin=%d active=%d, callback registered",
              SD_CS_SENSE_PIN, SD_CS_SENSE_ACTIVE_VALUE);
  }
#endif

#if defined(SD_SHARED_D1_PIN) && SD_SHARED_D1_PIN != GPIO_NUM_NC
  {
    // D1/D2: hold INPUT_PULLUP while MCU owns the bus (SDIO 4-bit isolation).
    gpio_config_t d1_conf = {};
    d1_conf.pin_bit_mask = (1ULL << SD_SHARED_D1_PIN);
    d1_conf.mode = GPIO_MODE_INPUT;
    d1_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    d1_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    d1_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&d1_conf);
  }
#endif

#if defined(SD_SHARED_D2_PIN) && SD_SHARED_D2_PIN != GPIO_NUM_NC
  {
    gpio_config_t d2_conf = {};
    d2_conf.pin_bit_mask = (1ULL << SD_SHARED_D2_PIN);
    d2_conf.mode = GPIO_MODE_INPUT;
    d2_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    d2_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    d2_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&d2_conf);
  }
#endif

#if defined(SD_SHARED_MUX_PIN) && SD_SHARED_MUX_PIN != GPIO_NUM_NC
  {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << SD_SHARED_MUX_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)SD_SHARED_MUX_PIN, SD_SHARED_MUX_IDLE_VALUE);
    esp3d_log("SharedSD: mux pin=%d → idle (%d)", SD_SHARED_MUX_PIN, SD_SHARED_MUX_IDLE_VALUE);
  }
#endif

  return true;
}
```

---

## 6. `_enableSharedSD()`

```cpp
bool ESP3DSd::_enableSharedSD() {
  // 1. Anti-concurrency check
  if (_sharedState != SharedSDState::MCU_OWNS) {
    esp3d_log("SharedSD: already owned, reject");
    return false;
  }

  // 2. MCU busy check.
  // If no callback is registered, access is permitted (permissive default).
  // Warning: always register a callback when active sharing is in use to avoid
  // silent SD corruption. The CS sense callback is auto-registered by _beginShared()
  // if SD_CS_SENSE_PIN is defined.
  if (!_mcuBusyCallback) {
    esp3d_log("SharedSD: no busy callback registered, access permitted (permissive)");
  } else if (_mcuBusyCallback()) {
    esp3d_log("SharedSD: MCU busy, deny");
    return false;
  }

  // 3. Claim ACQUIRING before the release callback.
  // This closes the race window when the callback blocks (e.g. UART handshake,
  // Option B). A concurrent call to _enableSharedSD() will now fail the
  // MCU_OWNS check above instead of entering in parallel.
  _sharedState = SharedSDState::ACQUIRING;

  // 4. Notify MCU firmware to release the SD (unmount, SPI teardown, etc.)
  // Fired AFTER the lock is taken, BEFORE the GPIO switch.
  if (_mcuReleaseCallback) {
    _mcuReleaseCallback();
  }

#if defined(SD_SHARED_MUX_PIN) && SD_SHARED_MUX_PIN != GPIO_NUM_NC
  gpio_set_level((gpio_num_t)SD_SHARED_MUX_PIN, SD_SHARED_MUX_ESP_VALUE);
  vTaskDelay(pdMS_TO_TICKS(SD_SHARED_SWITCH_DELAY_MS));
#endif

  // 5. Confirm transition and record timestamp for watchdog
  _sharedState = SharedSDState::ESP_OWNS;
  _acquireTimestamp = esp_timer_get_time() / 1000;
  esp3d_log("SharedSD: acquired at %llu ms", _acquireTimestamp);
  return true;
}
```

---

## 7. `_disableSharedSD()`

```cpp
bool ESP3DSd::_disableSharedSD() {
  if (_sharedState != SharedSDState::ESP_OWNS) {
    esp3d_log("SharedSD: not owned, skip");
    return false;
  }

  // 1. Restore multiplexer to MCU position
#if defined(SD_SHARED_MUX_PIN) && SD_SHARED_MUX_PIN != GPIO_NUM_NC
  gpio_set_level((gpio_num_t)SD_SHARED_MUX_PIN, SD_SHARED_MUX_IDLE_VALUE);
  vTaskDelay(pdMS_TO_TICKS(SD_SHARED_SWITCH_DELAY_MS));
#endif

  // 2. Restore D1/D2 isolation lines (SDIO 4-bit: held INPUT_PULLUP while MCU owns)
#if defined(SD_SHARED_D1_PIN) && SD_SHARED_D1_PIN != GPIO_NUM_NC
  {
    gpio_config_t d1_conf = {};
    d1_conf.pin_bit_mask = (1ULL << SD_SHARED_D1_PIN);
    d1_conf.mode = GPIO_MODE_INPUT;
    d1_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    d1_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    d1_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&d1_conf);
  }
#endif
#if defined(SD_SHARED_D2_PIN) && SD_SHARED_D2_PIN != GPIO_NUM_NC
  {
    gpio_config_t d2_conf = {};
    d2_conf.pin_bit_mask = (1ULL << SD_SHARED_D2_PIN);
    d2_conf.mode = GPIO_MODE_INPUT;
    d2_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    d2_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    d2_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&d2_conf);
  }
#endif

  // 3. Confirm transition
  _sharedState = SharedSDState::MCU_OWNS;
  esp3d_log("SharedSD: released after %llu ms", (esp_timer_get_time() / 1000) - _acquireTimestamp);

  // 4. Notify MCU firmware to remount SD (SPI reinit, card.mount(), etc.)
  // Fired AFTER the GPIO is restored and state is MCU_OWNS.
  if (_mcuRemountCallback) {
    _mcuRemountCallback();
  }

  return true;
}
```

---

## 8. `accessFS()`

The watchdog check runs at the top of every `accessFS()` call — no separate `handle()` required.

```cpp
bool ESP3DSd::accessFS(ESP3DFileSystemType FS, const char *vfs_path) {
  (void)FS;

#if ESP3D_SD_SHARED_FEATURE && defined(SD_SHARED_WATCHDOG_MS) && SD_SHARED_WATCHDOG_MS > 0
  // Watchdog: force-release if ESP has held the SD too long (e.g. after a crash/exception).
  // Design limitation: fires only on the next accessFS() call. If no new access is
  // attempted (e.g. only the blocked task uses SD), the watchdog never triggers.
  // Acceptable for this project — add a periodic timer if stricter recovery is needed.
  if (_state == ESP3DSdState::busy && _sharedState == SharedSDState::ESP_OWNS) {
    uint64_t elapsed = (esp_timer_get_time() / 1000) - _acquireTimestamp;
    if (elapsed > SD_SHARED_WATCHDOG_MS) {
      esp3d_log_e("SharedSD watchdog after %llu ms, forcing release", elapsed);
      _disableSharedSD();
      _state = ESP3DSdState::idle;
    }
  }
#endif

  if (getState() != ESP3DSdState::idle) {
    esp3d_log("SDCard not idle");
    return false;
  }

#if ESP3D_SD_SHARED_FEATURE
  if (!_enableSharedSD()) {
    esp3d_log("SharedSD: acquire failed");
    return false;
  }
#endif

  _state = ESP3DSdState::busy;
  return true;
}
```

---

## 9. `releaseFS()`

```cpp
void ESP3DSd::releaseFS(ESP3DFileSystemType FS, const char *vfs_path) {
  (void)FS;
  esp3d_log("SD: release");
  setState(ESP3DSdState::idle);

#if ESP3D_SD_SHARED_FEATURE
  _disableSharedSD();
#endif
}
```

---

## 10. Firmware Integration — Callbacks

### Three-callback model

```
_enableSharedSD()                        _disableSharedSD()
      │                                        │
      ├─ isMCUBusy?     ← passive query        ├─ GPIO → MCU_IDLE_VALUE
      │                                        ├─ vTaskDelay(stabilize)
      ├─ mcuRelease()   ← active hook          ├─ D1/D2 → INPUT_PULLUP
      ├─ GPIO → ESP_VALUE                      └─ mcuRemount()  ← active hook
      └─ vTaskDelay(stabilize)
```

Register callbacks at startup. The SD sharing code has no knowledge of the firmware type.

---

### CS Sense (auto-registered by `_beginShared()` when `SD_CS_SENSE_PIN` is defined)

```cpp
// Registered automatically — no manual call needed on FYSETC board.
_mcuBusyCallback = []() -> bool {
  return gpio_get_level((gpio_num_t)SD_CS_SENSE_PIN) == SD_CS_SENSE_ACTIVE_VALUE;
};
```

> **Overwrite behaviour:** `setMCUBusyCallback()` replaces whatever `_beginShared()` registered —
> callbacks do not accumulate. If you need CS Sense AND a firmware state check, combine both
> in a single callback (see FYSETC + Marlin example below). Calling `setMCUBusyCallback()`
> after `begin()` silently discards the auto-registration.

---

### grblHAL — full integration (busy + release + remount)

grblHAL does not have an explicit SD unmount API. The busy check is the primary guard;
release/remount hooks handle any application-level notification needed.

```cpp
// Busy: deny access while the machine is actively running a job.
// STATE_CYCLE covers homing, jogging, probing, and motion — all states where
// the MCU may be streaming from SD.
sd.setMCUBusyCallback([]() -> bool {
  sys_state_t state = state_get();
  return state == STATE_CYCLE   ||
         state == STATE_HOMING  ||
         state == STATE_JOG;
});

// Release: signal grblHAL to suspend any pending SD stream before the switch.
// If grblHAL exposes a suspend/feed-hold API, call it here.
sd.setMCUReleaseCallback([]() {
  // grbl.on_sd_releasing();  // project-specific hook, if available
});

// Remount: signal grblHAL that the SD is available again.
sd.setMCURemountCallback([]() {
  // grbl.on_sd_remounted();  // project-specific hook, if available
});
```

### FluidNC — status-based detection

```cpp
sd.setMCUBusyCallback([]() -> bool {
  return FluidNC::machineIsRunning();  // adapt to actual FluidNC state API
});
sd.setMCUReleaseCallback([]() {
  // FluidNC SD suspend hook if available
});
sd.setMCURemountCallback([]() {
  // FluidNC SD resume hook if available
});
```

---

### Marlin — full integration (busy + release + remount)

```cpp
sd.setMCUBusyCallback([]() -> bool {
  return card.isMounted() &&
         (IS_SD_PRINTING() || IS_SD_FETCHING() ||
          IS_SD_PAUSED()   || IS_SD_FILE_OPEN());
});
sd.setMCUReleaseCallback([]() {
  card.release();
});
sd.setMCURemountCallback([]() {
  card.mount();
});
```

---

### FYSETC + Marlin — combined (CS Sense auto-registered; add release/remount manually)

On FYSETC the busy callback is auto-registered by `_beginShared()`. If running Marlin,
add only the lifecycle hooks:

```cpp
// _mcuBusyCallback already registered by _beginShared() via CS_SENSE_PIN.
// Optionally reinforce with firmware state (belt-and-suspenders):
sd.setMCUBusyCallback([]() -> bool {
  if (gpio_get_level((gpio_num_t)SD_CS_SENSE_PIN) == SD_CS_SENSE_ACTIVE_VALUE) {
    return true;  // hardware check first — fastest, no firmware dependency
  }
  return card.isMounted() &&
         (IS_SD_PRINTING() || IS_SD_FETCHING() ||
          IS_SD_PAUSED()   || IS_SD_FILE_OPEN());
});
sd.setMCUReleaseCallback([]() { card.release(); });
sd.setMCURemountCallback([]() { card.mount(); });
```

---

### FYSETC — SPI bus isolation (if `spi_bus_free` / `spi_bus_initialize` required)

If the hardware requires full SPI bus teardown before switching the mux (board-specific):

```cpp
sd.setMCUReleaseCallback([]() {
  spi_bus_free(SD_SPI_HOST_IDX);
});
sd.setMCURemountCallback([]() {
  spi_bus_config_t bus_cfg = { ... };
  spi_bus_initialize(SD_SPI_HOST_IDX, &bus_cfg, SPI_DMA_CH_AUTO);
});
```

---

## 11. grblHAL Plugin — STM32 + ESP32-C3

### Context

Target: STM32 running grblHAL + ESP32-C3 running this ESP-IDF firmware, shared SD card.

The grblHAL plugin runs on the STM32. It monitors machine state and manages the SD bus
handover. The ESP32-C3 registers the three callbacks against this plugin at startup.
No modification to grblHAL core — plugin API only.

---

### Communication channel

Two options depending on available hardware. Choose one before implementing.

#### Option A — Single GPIO (recommended, simplest)

One output pin from STM32 to ESP32-C3: `MCU_SD_BUSY`.

```
STM32 (grblHAL plugin)          ESP32-C3
        │                            │
  MCU_SD_BUSY ──────────────────────►│ input
  HIGH = MCU owns SD / machine busy  │
  LOW  = SD released, ESP can take   │
```

The plugin drives `MCU_SD_BUSY` HIGH when the machine is running (STATE_CYCLE, HOMING, JOG)
or when the SD is actively mounted. It lowers it only after a clean SD unmount.

On the ESP32-C3 side:

```cpp
// board_config.h (ESP32-C3 target)
#define SD_CS_SENSE_PIN         GPIO_NUM_x   // connected to STM32 MCU_SD_BUSY
#define SD_CS_SENSE_ACTIVE_VALUE  1          // HIGH = MCU busy (adapt to wiring)
```

The CS sense mechanism auto-registers the busy callback in `_beginShared()` — no manual
`setMCUBusyCallback()` call needed. Release/remount callbacks remain optional (no
acknowledge signal on this option — ESP trusts the LOW level).

```cpp
// Optional: explicit sd_unmount/sd_mount notification if plugin exposes them via UART
sd.setMCUReleaseCallback([]() { /* send UART "SD_REQ" command to STM32 */ });
sd.setMCURemountCallback([]() { /* send UART "SD_DONE" command to STM32 */ });
```

---

#### Option B — UART command protocol

Uses the existing UART link between ESP32-C3 and STM32 (no extra GPIO).
The plugin registers a custom grblHAL realtime command or uses a dedicated prefix.

```
ESP32-C3                          STM32 (grblHAL plugin)
    │── "SD_REQ\n" ───────────────────►│ request SD access
    │                                  │ check state
    │                                  │ if busy: feed-hold, wait idle
    │                                  │ sd_unmount()
    │◄─────────────────── "SD_RDY\n" ──│ SD released, ESP can take
    │
    │  [ mux switch + SD access ]
    │
    │── "SD_DONE\n" ──────────────────►│ release notification
    │                                  │ sd_mount()
    │◄────────────────── "SD_ACK\n" ───│ (optional acknowledge)
```

On the ESP32-C3 side:

```cpp
sd.setMCUBusyCallback([]() -> bool {
  // Fast path: check a local flag set by the UART receive task
  return !grblhal_sd_ready_flag;
});

sd.setMCUReleaseCallback([]() {
  uart_send("SD_REQ\n");
  // Block until "SD_RDY\n" received (with timeout)
  wait_for_sd_ready(SD_HANDSHAKE_TIMEOUT_MS);
});

sd.setMCURemountCallback([]() {
  uart_send("SD_DONE\n");
});
```

---

### grblHAL plugin skeleton (STM32 side)

Applies to both options — adapt GPIO / UART calls to the target.

```cpp
// grblhal_sd_share_plugin.c

#include "grbl/grbl.h"
#include "grbl/protocol.h"
#include "grbl/state_machine.h"

// Option A: GPIO output
#define MCU_SD_BUSY_PIN   GPIO_PIN_x
#define MCU_SD_BUSY_PORT  GPIOx

static void sd_busy_set(bool busy) {
  HAL_GPIO_WritePin(MCU_SD_BUSY_PORT, MCU_SD_BUSY_PIN,
                    busy ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// Tracks whether the SD is safe to release (machine idle, no active job)
static bool sd_release_allowed(void) {
  sys_state_t state = state_get();
  return state == STATE_IDLE || state == STATE_ALARM || state == STATE_SLEEP;
}

// grblHAL state change callback — keep MCU_SD_BUSY in sync
static void on_state_change(sys_state_t state) {
  sd_busy_set(!sd_release_allowed());
}

// Option B: handle "SD_REQ" / "SD_DONE" commands from ESP32-C3
static bool on_realtime_command(char c) {
  // Custom single-char command, or intercept full-line via on_execute_command
  return false;  // return true if consumed
}

// Plugin entry point — called at grblHAL startup
void grblhal_sd_share_plugin_init(void) {
  // Init GPIO
  sd_busy_set(true);  // MCU owns SD at boot

  // Register state change hook
  grbl.on_state_change = on_state_change;

  // Option B: register UART command handler
  // grbl.on_realtime_command = on_realtime_command;
}
```

> **Note:** grblHAL plugin API (`grbl.on_state_change`, `grbl.on_execute_command`, etc.)
> may vary between grblHAL versions. Refer to the grblHAL driver/plugin documentation
> for the target STM32 board. The skeleton above uses the common callback pattern.

---

### SD unmount / remount in grblHAL

grblHAL does not have a portable `sd_unmount()` / `sd_mount()` API in core.
Options for the plugin implementation:

| Approach | Description |
|----------|-------------|
| **FatFS direct** | `f_mount(NULL, "0:", 0)` to unmount, `f_mount(&fs, "0:", 1)` to remount |
| **Board driver hook** | Some grblHAL drivers expose `sdcard_release()` / `sdcard_mount()` |
| **None (GPIO-only)** | If the plugin controls `MCU_SD_BUSY` and grblHAL is idle, no explicit unmount needed — the SD driver just stops being called |

The safest approach is FatFS direct unmount before lowering `MCU_SD_BUSY`.

---

## 12. Implementation Status

### Implemented in `esp3d_sd.h` / `esp3d_sd.cpp`

| Feature | Status |
|---------|--------|
| `SharedSDState` enum (MCU_OWNS / ACQUIRING / ESP_OWNS) | ✅ |
| `_beginShared()` — GPIO init, CS sense auto-callback | ✅ |
| `_enableSharedSD()` — concurrency check, busy check, release hook, GPIO switch | ✅ |
| `_disableSharedSD()` — GPIO restore, D1/D2 isolation restore, remount hook | ✅ |
| Watchdog in `accessFS()` | ✅ |
| `IsMCUBusyFn` / `SDActionFn` types | ✅ |
| `setMCUBusyCallback()` / `setMCUReleaseCallback()` / `setMCURemountCallback()` | ✅ |
| ESP-IDF API throughout (`gpio_config`, `gpio_set_level`, `vTaskDelay`, `esp_timer_get_time`) | ✅ |

### Remaining open point

| Feature | Priority | Note |
|---------|----------|------|
| Restrictive null-callback enforcement | Medium | Current: permissive (access permitted if no busy callback). Safer: deny access when `SD_SHARED_MUX_PIN != GPIO_NUM_NC` and no callback registered. Deferred — CS sense auto-registration in `_beginShared()` covers the FYSETC case. |

### Design notes

- **`_mcuReleaseCallback` call site**: after `_sharedState = ACQUIRING` is set (lock claimed), before the GPIO switch. The lock is taken first to close the race window when the callback blocks (e.g. Option B UART handshake).
- **`_mcuRemountCallback` call site**: after `_sharedState = MCU_OWNS`, at the very end of `_disableSharedSD()`.
- **ISSUE-5** (`_state` / `_sharedState` coexistence): both variables must remain consistent.
  `_state = busy` ↔ `_sharedState = ESP_OWNS` in shared mode. Enforce this with
  `assert` in debug builds if this becomes a maintenance concern.

---

*Document: shared_sd_mechanism_V2.0.md (revision V2.2) — 2026-05-12*
