# websocket

WebSocket binary file transfer test tool for the ESP3D WebSocket Protocol V1.
Supports status probe, file listing, upload, and download over `/wsdata`.

See `docs/esp3d_websocket_protocol_v1.md` for protocol details.

## Usage

```bash
python ws_transfer_test.py --host <ip> --port <port> <command> [args]
```

Examples:

```bash
python ws_transfer_test.py --host 192.168.1.100 --port 8282 status
python ws_transfer_test.py --host 192.168.1.100 --port 8282 list /
python ws_transfer_test.py --host 192.168.1.100 --port 8282 upload myfile.txt /fs
python ws_transfer_test.py --host 192.168.1.100 --port 8282 download /fs test.txt
```

## Dependencies

| Package | Install name | Used for |
|---|---|---|
| `websockets` | `websockets` | Async WebSocket client |

```bash
pip3 install websockets
```
