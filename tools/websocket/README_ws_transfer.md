# ws_transfer_test.py — data WebSocket client

**Same folder:** **`wsterm.html`** — profile **Data /wsdata** matches this script (**`/wsdata`** + **`esp3d-v1`**: TEXT, **SR**, V1 upload/download). Profile **WebUI /ws** is for **`/ws`** stream sniffing (see `docs/websockets_protocol.md`). **Log raw BIN (hex)** toggles verbose hex on data socket. Appends **`\\n`** on text send. **[`docs/tools.md`](../../docs/tools.md)** lists all `tools/`.

Tests **`/wsdata`** (subprotocol **`esp3d-v1`**): **ESP3D WebSocket Protocol V1** binary file transfer + text commands **[ESP720]** / **[ESP740]**.

**Not** the WebUI socket **`/ws`** (`webui-v3`): that endpoint uses binary for the **machine stream** and text for UI messages — **V1 file transfer does not exist there**. Use **`/wsdata` + `esp3d-v1`** for this tool.

## Setup

```bash
pip install websockets
```

## Quick reference

| Command | Notes |
|--------|--------|
| `list <path>` | Default: `json=yes`. Flash root: `/` (not `/fs` in the command; `/fs` is normalized by the tool). `--text` legacy lines; `--raw-json` dump JSON. |
| `upload <local_file> <remote_dir>` | Remote name = basename of local file. |
| `download <remote_dir> <remote_name> [local_path]` | Omit last arg → save as `./<remote_name>`. |
| `status` | **(1)** WebSocket handshake to **`/wsdata`** + **`esp3d-v1`**, then read welcome TEXT. **(2)** Binary **`SR` → `RS`**. Idle V1 servers answer with `O` + version byte `1`. Timeout or no `RS` ⇒ **no** V1 binary support (see `docs/esp3d_websocket_protocol_v1.md`). |
| `roundtrip <local_file> <remote_dir>` | Upload then download + MD5. |

Global: `--host`, `--port` (often `80` with built-in HTTP).  
`--binary-timeout SEC` — probe timeout (default 3).  
`--skip-binary-check` — skip probe before upload/download/roundtrip (emergency only).

Errors from the ESP (missing file, busy, etc.) print **`ERROR:`** on stderr and exit **1** without a Python traceback.

## Protocol

- **V1 (normative probe + server guide):** `docs/esp3d_websocket_protocol_v1.md`
- **Frame layout / opcodes:** `docs/websockets_protocol.md` (data WebSocket → binary mode)
