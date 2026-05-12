 # WebSocket Protocol

There are **two separate WebSocket endpoints** on the same HTTP port. They use **different URI paths and subprotocols**; a client **must** use the correct pair. Negotiating the wrong subprotocol or path means the connection is handled by the **wrong** service — file transfer will **not** work.

| Endpoint | Typical URI | Subprotocol (`Sec-WebSocket-Protocol`) | Role |
|----------|-------------|----------------------------------------|------|
| **Terminal / WebUI** | **`/ws`** | **`webui-v3`** | **BINARY** = raw machine/serial bytes (no V1 header). **TEXT** = `PING`, `NOTIFICATION:`, `currentID:`, etc. |
| **Data (external tools)** | **`/wsdata`** | **`esp3d-v1`** | ESP commands + G-code in **TEXT**; **BINARY** = V1 status and file upload/download (`SR`, `SU`, …). |

**Common mistake:** sending V1 binary opcodes on `/ws`. The server treats binary as stream data, not file-transfer frames — **transfers are impossible on that socket by design**.

## Subprotocols vs capabilities

Both endpoints use the same WebSocket transport (HTTP upgrade, TEXT and BINARY frames). What changes is which service handles the connection — fixed by URI path and negotiated subprotocol — and therefore what each frame type means.

| Subprotocol (with path) | TEXT frames | BINARY frames | File transfer (V1) |
|-------------------------|-------------|---------------|--------------------|
| **`webui-v3`** on **`/ws`** | WebUI protocol (`PING`, `NOTIFICATION:`, …), G-code / commands | Machine / serial stream (raw bytes, no V1 header) | **No** — do not send `SR`/`SU`/… here |
| **`esp3d-v1`** on **`/wsdata`** | ESP commands (e.g. `[ESP720]`), G-code relay | V1 — status (`SR`/`RS`) and upload/download framing | **Yes** |

## Implementation differences: ESP-IDF vs Arduino

Depending on the underlying HTTP server framework, the deployment of these two endpoints differs:

- **ESP-IDF (`httpd_ws`)**: Both `/ws` and `/wsdata` operate on the **same HTTP port** (port 80). The server routes connections to the correct handler strictly based on the requested URI path.
- **Arduino (`WebSocketsServer`)**: The server cannot natively multiplex WebSockets on the main HTTP port 80. Instead, ESP3D creates **two separate TCP listeners on different ports**. Typically, the Terminal WebSocket uses `HTTP_PORT + 1` (e.g., 81), and the Data WebSocket uses the port configured in settings (`ESP_WEBSOCKET_PORT`, typically 82). The URI path (`/ws` vs `/wsdata`) is ignored; the client must connect to the correct **port** directly.

---

## 1. Terminal WebSocket — `/ws`, subprotocol `webui-v3`

### Text mode

Reserved messages between WebUI and ESP. Format: `<label>:<message>`

**ESP → WebUI:**

- `currentID:<id>` — sent on connect; last used ID, becomes the active ID.
- `activeID:<id>` — broadcast when a new client connects; clients without this ID should close.
- `PING:<time left>:<time out>` — **when authentication is enabled**: response to client `PING:<sessionId>`.
- `PONG:<server_millis>` — **when authentication is disabled** (or client sends bare `PING` without sessionId): firmware answers with monotonic millisecond value. Used as keepalive reply.
- `ERROR:<code>:<message>` — upload error; informs client to stop uploading when HTTP cancellation is not possible.
- `NOTIFICATION:<message>` — forwards `[ESP600]` message to WebUI toast system.
- `SENSOR: <value>[<unit>] <value2>[<unit2>] …` — sensor data (e.g. DHT22).

**WebUI → ESP:**

- `PING:<sessionId>` — **when authentication is enabled**: session id so ESP can respond with time left.
- `PING:0` — optional keepalive **without authentication**; ESP answers `PONG:<server_millis>`.

### Keepalive / heartbeat

- Clients should read TEXT frames from the server (`PONG:…`, `currentID`, stream lines, etc.).
- On **ESP-IDF 5.4.x**, WebSocket `DATA` handlers may see `req->content_len == 0` while a TEXT frame is still pending; the firmware must use `httpd_ws_recv_frame()` to read it, otherwise client `PING` is never handled and heartbeats fail.

### Binary mode — machine stream (not V1)

**ESP → client:** payloads are **raw bytes** from the CNC/serial stream forwarded to the WebUI. There is **no** application header — not the 4-byte `opcode + u16 length` layout used on `/wsdata`.

**How the embedded WebUI decodes binary** (`embedded/src/index.js`): each byte mapped with `String.fromCharCode(byte)` (ISO-8859-1 style). Buffered until `\n` (0x0A) or `\r` (0x0D), then flushed as a line to the console.

**Client → ESP:** inbound binary on `/ws` is **not interpreted** (stub). User commands and G-code are sent as TEXT frames with `\n`/`\r` line discipline.

**Debugging:** use [`tools/websocket/wsterm.html`](../tools/websocket/wsterm.html) — profile **WebUI /ws (stream)**.

---

## 2. Data WebSocket — `/wsdata`, subprotocol `esp3d-v1`

This endpoint carries the **ESP3D WebSocket Protocol V1**: binary file-transfer and status frames, plus a text channel for ESP commands.

### Scope

| In scope | Out of scope |
|----------|-------------|
| Binary frames: status, upload, download, abort | Terminal WebSocket (`webui-v3`) |
| Text channel on the same socket (ESP commands, listings) | HTTP upload / WebDAV |
| Ordering, timeouts, capability probe | WebSocket session / cookie auth (future) |

### Authentication

**V1 is specified for the no-WebSocket-auth case:** after connect, the client reads the welcome TEXT frame, then may send binary **SR** or text commands without tokens.

Builds with `ESP3D_AUTHENTICATION_FEATURE` may expose `/wsdata` without the same checks as HTTP until a future revision documents the handshake. Simplest interop: **open socket → welcome → SR / SU / …** with no extra steps.

### Protocol version

V1 is identified explicitly when the server answers SR while idle: see [Capability probe](#capability-probe) below. Future revisions may increment the version byte or add opcodes; clients must ignore unknown opcodes if they only need a subset.

### Two client checks

| Step | What it validates | Failure means |
|------|-------------------|---------------|
| **(1) WebSocket handshake** | TCP + HTTP upgrade to `/wsdata` with `esp3d-v1` negotiated | Wrong host/port, 404, subprotocol rejected — no data channel |
| **(2) Status command SR → RS** | After socket open, send binary **SR** and receive **RS** within timeout | Endpoint open but no V1 binary handler — **do not infer V1 from handshake alone** |

### On connect

The server sends **one TEXT frame** first: a welcome line (`Welcome to ESP3D-X V…`). Clients must **read** it before assuming the next message is a command reply.

### Text mode

Used for ESP commands and G-code relay (e.g. `[ESP720]` list flash, `[ESP740]` list SD). Use **`json=yes`** for directory listings — one JSON object, faster and unambiguous. Plain-text listings end with `Total:…` / `Available:…` lines without a final `ok`.

**Line termination:** client TEXT is accumulated until `\n` or `\r`. A frame without a line ending is received but does not run the command. Tools should send `[ESP720]…\n`.

**Flash paths:** `[ESP720]` takes paths relative to the flash logical root; use `/` for root.

### Binary mode — V1 frame format

Packet data size defaults to **1024 bytes** (`ESP3D_WS_TRANSFER_PACKET_SIZE`).

All frames share the same header:

| Offset | Size | Content |
|--------|------|---------|
| 0 | 2 | Opcode ASCII (e.g. `SR`, `RS`) |
| 2 | 2 | Payload length, **little-endian** |
| 4 | N | Payload |

**Start / ACK status byte** (first byte of **US**, **DS**, **PU**, **UE** where applicable):

| Byte | Hex | Meaning |
|------|-----|---------|
| `O` | 0x4F | ok / idle |
| `E` | 0x45 | error |
| `B` | 0x42 | busy |
| `A` | 0x41 | abort |

---

### Frame types

#### SR — Status Request (client → ESP)

```
| S | R | 0 | 0 |
```
(hex: `53 52 00 00`)

---

#### RS — Status Response (ESP → client)

Response to **SR**. First payload byte = transfer state:

| Byte | Hex | Meaning |
|------|-----|---------|
| `O` | 0x4F | idle / ok |
| `B` | 0x42 | busy |
| `E` | 0x45 | error |
| `A` | 0x41 | abort |
| `D` | 0x44 | download ongoing |
| `U` | 0x55 | upload ongoing |

**Idle (V1):** payload = `O` (0x4F) + `1` (0x01) — protocol version byte.
```
| R | S | 0 | 2 | O | 1 |
```
(hex: `52 53 02 00 4F 01`)

Legacy servers send only `O` (1-byte payload, no version).

**Error:** `E` + 1-byte error code + 1-byte string size + string
```
| R | S | x | x | E | code | len | string... |
```

**Upload in progress:** `U` + path size + path + filename size + filename + total size (4B LE) + processed bytes (4B LE) + last packet id (4B LE)

**Download in progress:** same layout as upload with `D`.

---

#### SU — Start Upload (client → ESP)

```
| S | U | x | x | path_len | path... | name_len | name... | total_size(4B LE) |
```

**US — Upload Start ACK (ESP → client):**
```
| U | S | 0 | 1 | O |   ← transfer can start
```

---

#### UP — Upload Packet (client → ESP)

```
| U | P | x | x | packet_id(4B LE) | data... |
```

**PU — Packet Upload ACK (ESP → client):**

Success (5 bytes): `O` + packet_id (4B LE)
```
| P | U | 0 | 5 | O | id | id | id | id |
```

Error (5 bytes): `E` + packet_id (4B LE). If UP payload was too short to contain an id, id = `0xFFFFFFFF`.
```
| P | U | 0 | 5 | E | id | id | id | id |
```
Clients should treat **PU** with length 1 and `E` only as legacy (pre-correlation).

---

#### EU — End Upload (client → ESP) / UE — Upload End ACK (ESP → client)

Standard end-of-transfer exchange. First byte of **UE** uses `O`/`E`/`B`/`A`.

---

#### SD — Start Download (client → ESP)

```
| S | D | x | x | path_len | path... | name_len | name... | total_size(4B LE) |
```

**DS — Download Start ACK (ESP → client):**
```
| D | S | 0 | 1 | O |   ← transfer can start
```

---

#### DP — Download Packet (ESP → client)

```
| D | P | x | x | packet_id(4B LE) | data... |
```

**PD — Packet Download ACK (client → ESP):**
```
| P | D | 0 | 5 | O | id | id | id | id |
```

---

#### ED — End Download (ESP → client) / DE — Download End ACK (client → ESP)

Standard end-of-download exchange.

---

#### CM — Command (client → ESP)

```
| C | M | 0 | 1 | cmd |
```

| Code | Hex | Meaning |
|------|-----|---------|
| `A` | 0x41 | abort |

Abort frame:
```
| C | M | 0 | 1 | A |
```

---

### Capability probe

Clients **must** assume no binary file-transfer support until check **(2)** succeeds, even if **(1)** succeeded.

1. After WebSocket is open, consume the **mandatory welcome TEXT** frame.
2. Send binary **SR** with empty payload (header only: `S`, `R`, length `0` LE).
3. Wait for a binary response (recommended timeout: **2–5 s**).
4. Interpretation:
   - No binary `RS` before timeout → **no binary support** (wrong endpoint/subprotocol or old firmware).
   - Binary `RS` with non-empty payload → binary status channel present; first payload byte = transfer state.
   - Idle (`O`), payload ≥ 2 bytes: byte 1 = protocol version; value `1` = V1 as defined here.
   - Idle (`O`), payload 1 byte: legacy server — clients may still attempt V1 but explicit version byte is preferred.

---

### Server implementation guide

**Connection:**
- Path: `/wsdata` (distinct from `/ws`).
- Subprotocol: negotiate `esp3d-v1`. Reject clients offering only `webui-v3` for this URI.
- Send one TEXT welcome frame immediately after accept.

**Minimum V1 behavior:**

1. **SR → RS** — idle: `O` + version `1` (2-byte payload). Upload/download in progress: state byte + path + filename + sizes.
2. **Upload sequence** — accept **SU**, **UP**, **EU**; reply **US**, **PU** (5-byte payload), **UE**.
3. **Download sequence** — accept **SD**; reply **DS** with file size, then **DP** / **ED**; client sends **PD** / **DE**.
4. **Concurrency** — one active transfer per connection; return `B` on **US**/**DS** if busy.
5. **Text channel** — same socket carries TEXT frames for `[ESP720]`/`[ESP740]` and G-code.

**Error handling:**
- Invalid opcode: log/drop; optional TEXT error is product-specific.
- Payload too short: respond with `E` on the relevant ACK if the protocol defines one for that step.

### Testing

```bash
python tools/websocket/ws_transfer_test.py --host <ip> --port <port> status
python tools/websocket/ws_transfer_test.py --host <ip> --port <port> roundtrip <file> /fs
```

Browser: [`tools/websocket/wsterm.html`](../tools/websocket/wsterm.html) — profile **Data /wsdata (V1 tools)**.
Other utilities: [`docs/guides/tools.md`](../guides/tools.md).

### Reference implementation

- Server: `main/modules/websocket_server/esp3d_ws_data_service.cpp` / `.h`
- Constants: `WS_OP_*`, `WS_STATUS_*`, `WS_BINARY_PROTOCOL_V1`
- Packet size: `ESP3D_WS_TRANSFER_PACKET_SIZE`

### Client note

The pendant's WebSocket **client** build targets text streaming to the CNC; it does **not** implement the V1 binary file-transfer client. Third-party servers on the machine tool can implement this document so PC or phone clients perform file transfer over `ws://`.

---

*Document updated 2026-04-17 — merged from `websockets_protocol.md` and `esp3d_websocket_protocol_v1.md`*
