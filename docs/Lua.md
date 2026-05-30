# Lua Interpreter

ESP3D embeds a Lua 5.4 scripting engine for user automation — GPIO control, command injection, serial echo, button handling, etc. Scripts run asynchronously in a dedicated FreeRTOS task and communicate with the ESP3D message bus via a FIFO queue.

---

## Compile conditions

Enable the feature in `esp3d/configuration.h`:

```c
#define ESP_LUA_INTERPRETER_FEATURE
```

**Platform support:** ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6 only.  
**Not supported:** ESP8266, ESP8285 — compilation blocked by sanity check (`esp3d_sanity.h` line 217).

**Optional but useful:**

| Define | Effect |
|--------|--------|
| `FILESYSTEM_FEATURE` | Load scripts from flash filesystem |
| `SD_DEVICE` | Load scripts from SD card |
| `GCODE_HOST_FEATURE` | Enables `ESP_AUTOSTART_SCRIPT` / `ESP_AUTOSTART_SCRIPT_FILE` at boot |

---

## File structure

```
esp3d/src/modules/lua_interpreter/
├── lua_interpreter_service.h   — LuaInterpreter class declaration
└── lua_interpreter_service.cpp — Implementation (539 lines)

libraries/EspLuaEngine-1.0.3/   — Embedded Lua 5.4.7 engine library
├── EspLuaEngine.h
└── EspLuaEngine.cpp

lua_script_examples/
├── echo-serial/detectpush.lua  — Echo serial input back to output
└── push_button/detectpush.lua  — Detect GPIO button and send ESP3D command
```

---

## Dependencies

| Dependency | Type | Notes |
|------------|------|-------|
| `EspLuaEngine` (v1.0.3) | Library | Bundled in `libraries/`. Wraps Lua 5.4.7. Repo: [luc-github/EspLuaEngine](https://github.com/luc-github/EspLuaEngine) |
| FreeRTOS | Platform | `xTaskCreatePinnedToCore`, `vTaskDelete`, `xSemaphoreCreateMutex` |
| `ESP3DMessageFIFO` | Internal | Message queue for Lua ↔ ESP3D communication |
| `ESP3DMessage` | Internal | ESP3D message bus |

---

## Architecture

```
Main loop
    │
    ▼
LuaInterpreter::handle()          ← flushes _messageOutFIFO to ESP3D bus
    │
    │  [ESP300] command
    ▼
LuaInterpreter::executeScriptAsync()
    │
    ▼
xTaskCreatePinnedToCore()         ← FreeRTOS task, core 1, 8192 bytes stack, priority 1
    │
    ▼
scriptExecutionTask()
    ├── Load file from FS or SD (max 2048 bytes)
    ├── EspLuaEngine::executeScript()
    │       └── Lua debug hook every 1000 instructions → check pause/stop flags
    └── deleteScriptTask() on completion or error
```

### Message flow

```
ESP3D bus  ──→  _messageInFIFO  ──→  l_available() / l_readData()  (Lua reads)
               (max 10 messages)

Lua print()  ──→  l_print()  ──→  _messageOutFIFO  ──→  handle()  ──→  ESP3D bus
```

`print()` inside a script injects a message into the ESP3D command pipeline — it behaves exactly like a command typed on the serial port.

### LuaInterpreter class

```cpp
extern LuaInterpreter esp3d_lua_interpreter;
```

| Method | Description |
|--------|-------------|
| `begin()` | No-op (setup done in constructor) |
| `end()` | Delete task, free buffer |
| `handle()` | Flush output FIFO, called every main loop cycle |
| `executeScriptAsync(path)` | Start script execution in background task |
| `abortScript()` | Stop running script immediately |
| `pauseScript()` | Pause at next hook point |
| `resumeScript()` | Resume from pause |
| `isScriptRunning()` | True if task active |
| `isScriptPaused()` | True if paused |
| `getCurrentScriptName()` | Path of running script |
| `getExecutionTime()` | Elapsed ms (pauses excluded) |
| `getLastError()` | Last Lua error string |
| `dispatch(message)` | Push message into `_messageInFIFO` |

---

## Initialization

**Constructor** (`LuaInterpreter::LuaInterpreter()`): called at static init time.
- Creates `_stateMutex` (FreeRTOS mutex)
- Calls `setupFunctions()` and `registerConstants()`
- Sets `_messageInFIFO` max size to 10, `_messageOutFIFO` unlimited

**Main loop** (`esp3d.cpp`):
```cpp
#ifdef ESP_LUA_INTERPRETER_FEATURE
  esp3d_lua_interpreter.handle();
#endif
```

**Autostart** (requires `GCODE_HOST_FEATURE`):
```c
// In configuration.h — execute a script command at boot
#define ESP_AUTOSTART_SCRIPT "[ESP300]/FS/myscript.lua"
// Or execute a gcode file containing [ESP300] commands
#define ESP_AUTOSTART_SCRIPT_FILE "/FS/init.gcode"
```

---

## Exposed Lua API

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `print` | `print(str)` | Send string as ESP3D command/output |
| `available` | `available() → int` | Number of messages waiting in input queue |
| `readData` | `readData() → string` | Read next message from input queue |
| `yield` | `yield()` | Yield to FreeRTOS scheduler (required in infinite loops) |
| `delay` | `delay(ms)` | Pause script; checks abort/pause state during wait |
| `millis` | `millis() → int` | Milliseconds since boot |
| `pinMode` | `pinMode(pin, mode)` | Set GPIO pin mode |
| `digitalWrite` | `digitalWrite(pin, val)` | Write HIGH/LOW to GPIO |
| `digitalRead` | `digitalRead(pin) → int` | Read GPIO pin value |
| `analogWrite` | `analogWrite(pin, val)` | PWM output |
| `analogRead` | `analogRead(pin) → int` | ADC read |

### Constants

| Constant | Value |
|----------|-------|
| `HIGH` | 1 |
| `LOW` | 0 |
| `INPUT` | GPIO input mode |
| `OUTPUT` | GPIO output mode |
| `INPUT_PULLUP` | GPIO input with pull-up |
| `INPUT_PULLDOWN` | GPIO input with pull-down |

### Loaded Lua standard libraries

`base`, `table`, `string`, `math`, `utf8`.

**Not loaded:** `io`, `os`, `package`, `debug` — intentionally excluded for security and embedded resource constraints.

---

## Script limits

| Limit | Value |
|-------|-------|
| Max script file size | 2048 bytes (`ESP_LUA_MAX_SCRIPT_SIZE`) |
| Task stack size | 8192 bytes |
| Task priority | 1 |
| Task core | Core 1 (pinned) |
| Input FIFO depth | 10 messages |
| Output FIFO depth | unlimited |
| Hook interval | every 1000 Lua instructions |
| Concurrent scripts | 1 (enforced by `isScriptRunning()` check) |

---

## Commands

### `[ESP300]` — Execute script

```
[ESP300]<path>
```

Executes a Lua script asynchronously. Only one script can run at a time.

**Path formats:**

| Path | Filesystem |
|------|------------|
| `/FS/script.lua` | Flash (LittleFS / SPIFFS) |
| `[ESP_FLASH_FS_HEADER]script.lua` | Flash (explicit header) |
| `[ESP_SD_FS_HEADER]script.lua` | SD card |

**Authentication:** admin level required.

**Examples:**
```
[ESP300]/FS/detectpush.lua
[ESP300]/SD/automation.lua
```

---

### `[ESP301]` — Query and control script

```
[ESP301]                          → query current status
[ESP301]action=PAUSE              → pause running script
[ESP301]action=RESUME             → resume paused script
[ESP301]action=ABORT              → stop running script
```

**Status response (plain text):**
```
Status: running Script: /FS/myscript.lua Duration: 00:01:23
Status: idle
Status: error Error: attempt to call nil value
```

**Status response (JSON with `json` param):**
```json
{"status":"running","script":"/FS/myscript.lua","duration":"00:01:23"}
{"status":"idle"}
{"status":"error","error":"attempt to call nil value"}
```

---

## Script examples

### Echo serial (`lua_script_examples/echo-serial/echo-serial.lua`)

Reads messages from the ESP3D input queue and echoes them back prefixed with `echo:`.

```lua
local availableMsg = 0
local message = ""
while (true) do
    availableMsg = available()
    if (availableMsg > 0) then
        message = "echo:" .. readData()
        print(message)
    end
    yield()
end
```

**Use case:** debug serial communication, test the message bus, or proxy commands.

---

### Push button (`lua_script_examples/push_button/detectpush.lua`)

Monitors a GPIO pin and sends an ESP3D command when a button is pressed.

```lua
local pin = 0
local trigger_value = LOW
local command = "[ESP212]IP:%ESP_IP%\n"
local pinval
local lastpush = millis()

pinMode(pin, INPUT_PULLUP)

while (true) do
    pinval = digitalRead(pin)
    if (pinval == trigger_value) then
        if ((millis() - lastpush) > 1000) then
            lastpush = millis()
            print(command)
        end
    end
    yield()
end
```

**Use case:** physical button to display device IP on a printer screen via `[ESP212]`. Debounced to 1 second minimum between sends.

---

## Porting to ESP-IDF (without Arduino)

The Lua interpreter module has a thin coupling to Arduino. Here are the key adaptation points:

### 1. EspLuaEngine

The engine uses `vTaskDelay(1)` on ESP32 (not Arduino `delay()`). It is already IDF-compatible at the core. The `executeScript()` method and hook mechanism use `std::atomic<bool>` — compatible with IDF's C++ runtime.

Replace the `#include <Arduino.h>` in `lua_interpreter_service.h` with IDF equivalents:

```cpp
// Replace:
#include <Arduino.h>
// With:
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
```

### 2. FreeRTOS

The task creation call is already IDF-native:

```cpp
xTaskCreatePinnedToCore(
    scriptExecutionTask, "LuaScriptTask",
    8192, this, 1, &_scriptTask, 1
);
```

No change needed for IDF.

### 3. GPIO functions

Replace Arduino-style GPIO calls in the C wrappers with IDF driver calls:

| Arduino (current) | IDF equivalent |
|-------------------|----------------|
| `pinMode(pin, OUTPUT)` | `gpio_set_direction(pin, GPIO_MODE_OUTPUT)` |
| `digitalWrite(pin, HIGH)` | `gpio_set_level(pin, 1)` |
| `digitalRead(pin)` | `gpio_get_level(pin)` |
| `analogRead(pin)` | `adc_oneshot_read(...)` |
| `analogWrite(pin, val)` | `ledc_set_duty(...)` + `ledc_update_duty(...)` |

### 4. `millis()`

```cpp
// Replace millis() with:
(uint32_t)(esp_timer_get_time() / 1000)
```

### 5. `print()` / message bus

The `l_print()` function dispatches to `esp3d_commands`. In an IDF project, replace this with your own message dispatch (UART write, event queue post, etc.).

### 6. File loading

`scriptExecutionTask()` uses `ESP_FileSystem` and `ESP_SD` abstractions. Replace with IDF `stdio.h` or `esp_vfs` calls:

```c
FILE* f = fopen(path, "r");
fread(buffer, 1, size, f);
fclose(f);
```

### 7. String class

Several members use Arduino `String`. Replace with `std::string` or plain `char[]`:

```cpp
// Arduino:
String _currentScriptName;
// IDF:
std::string _currentScriptName;
// or:
char _currentScriptName[64];
```

### 8. `ESP3DMessageFIFO`

This is an ESP3D-specific queue. Replace with a FreeRTOS `QueueHandle_t` or a `std::queue<std::string>` protected by a mutex.

### 9. Integration checklist for IDF

- [ ] Add `EspLuaEngine` source to CMakeLists.txt component
- [ ] Remove `#include <Arduino.h>` from all Lua module files
- [ ] Replace GPIO wrappers with IDF driver calls
- [ ] Replace `millis()` with `esp_timer_get_time() / 1000`
- [ ] Replace `ESP_FileSystem` / `ESP_SD` with IDF VFS or SPIFFS/FAT driver
- [ ] Replace `ESP3DMessageFIFO` with a FreeRTOS queue
- [ ] Replace `String` with `std::string`
- [ ] Wire `handle()` into your main loop or a timer callback
- [ ] Wire `dispatch()` into your message input path if bidirectional communication is needed

---

## Related commands

| Command | Description |
|---------|-------------|
| `[ESP300]<path>` | Execute a Lua script |
| `[ESP301]` | Query script status |
| `[ESP301]action=PAUSE\|RESUME\|ABORT` | Control running script |
| `[ESP420]` | Shows `lua: enabled` in device status if feature is compiled |
