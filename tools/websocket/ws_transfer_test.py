#!/usr/bin/env python3
"""
WebSocket binary file transfer test — ESP3D WebSocket Protocol V1.

Normative probe + server notes: `docs/esp3d_websocket_protocol_v1.md`.
Frame reference: `docs/websockets_protocol.md`. CLI notes: `tools/websocket/README_ws_transfer.md`.

This tool uses **`/wsdata`** and subprotocol **`esp3d-v1`** only. The WebUI socket **`/ws`** / **`webui-v3`** uses binary for the live stream, not V1 file transfer — see docs.

Usage:
  pip install websockets

  From repo root: python tools/websocket/ws_transfer_test.py …
  Or cd tools/websocket then: python ws_transfer_test.py …

  # Query binary status / V1 probe (SR → RS; timeout ⇒ no binary support)
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 status
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 --binary-timeout 5 status

  # List files (default: json=yes, one JSON object — fast and structured)
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 list /
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 list /sd/gcodes
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 list / --raw-json
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 list / --text  # legacy lines

  # Upload a file to /fs/test.txt on the ESP
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 upload myfile.txt /fs

  # Download /fs/test.txt from the ESP (omit last arg → save as ./test.txt)
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 download /fs test.txt
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 download /fs test.txt ./received.txt

  # Round-trip test (upload then download and compare; SR/RS probe runs first)
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 roundtrip myfile.txt /fs
  python ws_transfer_test.py --host 192.168.1.100 --port 8282 --skip-binary-check upload f.txt /fs
"""

import argparse
import asyncio
import hashlib
import json
import os
import struct
import sys

try:
    import websockets
except ImportError:
    print("ERROR: install the 'websockets' package: pip install websockets")
    sys.exit(1)

# ---------------------------------------------------------------------------
# Protocol constants
# ---------------------------------------------------------------------------

SUBPROTOCOL = "esp3d-v1"
DATA_WS_PATH = "/wsdata"  # not /ws (webui-v3) — see docs/esp3d_websocket_protocol_v1.md
PACKET_SIZE = 1024  # data bytes per UP/DP packet (must match ESP3D_WS_TRANSFER_PACKET_SIZE)

# Opcodes
OP_STATUS_REQ      = b"SR"
OP_STATUS_RESP     = b"RS"
OP_UPLOAD_START    = b"SU"
OP_UPLOAD_STARTACK = b"US"
OP_UPLOAD_PKT      = b"UP"
OP_UPLOAD_PKTACK   = b"PU"
OP_UPLOAD_END      = b"EU"
OP_UPLOAD_ENDACK   = b"UE"
OP_DLOAD_START     = b"SD"
OP_DLOAD_STARTACK  = b"DS"
OP_DLOAD_PKT       = b"DP"
OP_DLOAD_PKTACK    = b"PD"
OP_DLOAD_END       = b"ED"
OP_DLOAD_ENDACK    = b"DE"
OP_NAK             = b"NK"
OP_COMMAND         = b"CM"

# Status bytes
S_OK    = ord('O')
S_ERROR = ord('E')
S_BUSY  = ord('B')
S_ABORT = ord('A')
S_UPLOAD = ord('U')
S_DLOAD  = ord('D')

STATUS_NAMES = {S_OK: "OK", S_ERROR: "ERROR", S_BUSY: "BUSY",
                S_ABORT: "ABORT", S_UPLOAD: "UPLOADING", S_DLOAD: "DOWNLOADING"}

DEFAULT_BINARY_PROBE_TIMEOUT = 3.0


def data_ws_url(host: str, port: int) -> str:
    return f"ws://{host}:{port}{DATA_WS_PATH}"


def print_handshake_ok(host: str, port: int, welcome_line: str) -> None:
    """(1) Handshake: /wsdata + subprotocol esp3d-v1; welcome confirms data handler."""
    print(f"  (1) Handshake: OK — {data_ws_url(host, port)}, subprotocol {SUBPROTOCOL!r}")
    disp = welcome_line.strip() if welcome_line else "(empty)"
    print(f"      Welcome TEXT: {disp}")


def print_status_probe_line(summary: str) -> None:
    """(2) Application probe: SR → RS (ESP3D WebSocket Protocol V1)."""
    print(f"  (2) Status command (SR→RS): {summary}")


# First payload byte: US/DS (start), PU (per-packet upload ACK), UE (upload end)
BINARY_ACK_STATUS_REASON = {
    S_ERROR: "rejected — file may be missing, path invalid, or I/O error",
    S_BUSY: "rejected — another transfer is already in progress",
    S_ABORT: "rejected — transfer aborted",
}


class WsTransferError(Exception):
    """Expected failure from the ESP (missing file, busy, etc.). Message is user-facing."""


def _start_ack_status_byte(payload: bytes) -> int | None:
    if not payload:
        return None
    return payload[0]


def _format_start_ack_failure(op_expected: str, op_got: str, payload: bytes,
                               remote_label: str) -> str:
    if op_got != op_expected:
        return (
            f"Expected {op_expected} (start ACK) for {remote_label}, "
            f"got opcode {op_got!r}"
        )
    st = _start_ack_status_byte(payload)
    if st is None:
        return f"Empty {op_expected} response for {remote_label}"
    if st == S_OK:
        return ""
    reason = BINARY_ACK_STATUS_REASON.get(
        st, f"unknown status byte {st!r} (0x{st:02X})"
    )
    return f"{op_expected} {reason} ({remote_label})"


# ---------------------------------------------------------------------------
# Frame helpers
# ---------------------------------------------------------------------------

def make_frame(op: bytes, payload: bytes = b"") -> bytes:
    """Build a 4-byte header frame: opcode(2) + payload_len(2 LE) + payload."""
    return op + struct.pack("<H", len(payload)) + payload

def parse_frame(data: bytes) -> tuple[str, bytes]:
    """Parse a binary frame. Returns (opcode_str, payload_bytes)."""
    if len(data) < 4:
        raise ValueError(f"Frame too short: {len(data)} bytes")
    op = data[:2].decode("ascii")
    payload_len = struct.unpack("<H", data[2:4])[0]
    payload = data[4:4 + payload_len]
    return op, payload

async def recv_binary(ws, timeout: float = 10.0) -> tuple[str, bytes]:
    """Receive the next binary WebSocket frame and parse it.
    Skips (and prints) any interleaved text frames."""
    raw = await asyncio.wait_for(ws.recv(), timeout=timeout)
    if isinstance(raw, str):
        print(f"  [TEXT] {raw.strip()}")
        return await recv_binary(ws, timeout)
    return parse_frame(raw)


async def recv_text_responses(ws, timeout: float = 10.0,
                               end_marker: str = "ok",
                               stop_lines: tuple[str, ...] = (),
                               stop_prefixes: tuple[str, ...] = (),
                               end_line_prefixes: tuple[str, ...] = ()) -> list[str]:
    """Collect text frames until end_marker, a matching line prefix, error stop, or timeout.
    Binary frames received while waiting are silently discarded.
    stop_lines / stop_prefixes: ESP errors (e.g. 'No SD') that omit 'ok'.
    end_line_prefixes: e.g. ('Total:',) for ESP720/740 text list tail (no trailing 'ok')."""
    lines = []
    loop = asyncio.get_event_loop()
    deadline = loop.time() + timeout
    stop_lines_l = tuple(s.lower() for s in stop_lines)
    stop_prefixes_l = tuple(p.lower() for p in stop_prefixes)
    end_prefixes_l = tuple(p.lower() for p in end_line_prefixes)

    def should_stop(line_stripped: str) -> bool:
        if not line_stripped:
            return False
        el = line_stripped.lower()
        if el == end_marker.lower():
            return True
        for pref in end_prefixes_l:
            if el.startswith(pref):
                return True
        if el in stop_lines_l:
            return True
        return any(el.startswith(p) for p in stop_prefixes_l)

    while True:
        remaining = deadline - loop.time()
        if remaining <= 0:
            break
        try:
            raw = await asyncio.wait_for(ws.recv(), timeout=remaining)
        except asyncio.TimeoutError:
            break
        if isinstance(raw, bytes):
            # Binary frame during text exchange — skip
            continue
        for line in raw.splitlines():
            line = line.strip()
            if line:
                lines.append(line)
                if should_stop(line):
                    return lines
    return lines


async def recv_json_document(ws, timeout: float = 15.0) -> dict:
    """Read one or more TEXT frames until concatenated payload parses as JSON."""
    parts: list[str] = []
    loop = asyncio.get_event_loop()
    deadline = loop.time() + timeout
    last_err: Exception | None = None
    while loop.time() < deadline:
        remaining = deadline - loop.time()
        if remaining <= 0:
            break
        try:
            raw = await asyncio.wait_for(ws.recv(), timeout=remaining)
        except asyncio.TimeoutError:
            break
        if isinstance(raw, bytes):
            continue
        parts.append(raw)
        blob = "".join(parts).strip()
        if not blob:
            continue
        try:
            doc = json.loads(blob)
            if isinstance(doc, dict):
                return doc
        except json.JSONDecodeError as exc:
            last_err = exc
            continue
    blob = "".join(parts).strip()
    hint = f"{blob[:240]!r}…" if len(blob) > 240 else repr(blob)
    raise ValueError(f"invalid or incomplete JSON ({hint}); {last_err!r}")


def print_listing_from_json(doc: dict, *, raw: bool) -> None:
    """Pretty-print ESP720/740 json response (status ok/error)."""
    if raw:
        print(json.dumps(doc, indent=2))
        print("  --- done ---")
        return
    if doc.get("status") == "error":
        print(f"  ERROR: {doc.get('data')}")
        print("  --- failed ---")
        return
    data = doc.get("data")
    if not isinstance(data, dict):
        print(f"  Unexpected response: {doc!r}")
        print("  --- failed ---")
        return
    print(f"  path: {data.get('path', '')}")
    files = data.get("files")
    if isinstance(files, list):
        for ent in files:
            if not isinstance(ent, dict):
                continue
            name = ent.get("name", "?")
            size = ent.get("size", "")
            t = ent.get("time")
            extra = f"  {t}" if t else ""
            if size == "-1":
                print(f"  DIR   {name}{extra}")
            else:
                print(f"  FILE  {name}  [{size}]{extra}")
    for key in ("total", "used", "occupation"):
        if key in data:
            print(f"  {key}: {data[key]}")
    print("  --- done ---")


# ---------------------------------------------------------------------------
# Protocol operations
# ---------------------------------------------------------------------------

def interpret_rs(op: str, payload: bytes) -> dict:
    """Parse RS payload after client sent SR (ESP3D WebSocket Protocol V1)."""
    d: dict = {
        "binary_capable": False,
        "reason": "",
        "state_byte": None,
        "state_name": None,
        "protocol_version": None,
        "v1_announced": False,
        "total": None,
        "done": None,
        "last_id": None,
    }
    if op != "RS":
        d["reason"] = f"expected opcode RS, got {op!r}"
        return d
    if len(payload) < 1:
        d["reason"] = "empty RS payload"
        return d
    st = payload[0]
    if st not in STATUS_NAMES:
        d["reason"] = f"unknown status byte 0x{st:02X}"
        return d
    d["binary_capable"] = True
    d["state_byte"] = st
    d["state_name"] = STATUS_NAMES[st]
    if st == S_OK:
        if len(payload) >= 2:
            d["protocol_version"] = payload[1]
            d["v1_announced"] = True
        else:
            d["protocol_version"] = 1
            d["v1_announced"] = False
    if st in (S_UPLOAD, S_DLOAD) and len(payload) >= 13:
        d["total"] = struct.unpack_from("<I", payload, 1)[0]
        d["done"] = struct.unpack_from("<I", payload, 5)[0]
        d["last_id"] = struct.unpack_from("<I", payload, 9)[0]
    return d


async def exchange_sr_rs(ws, timeout: float) -> dict:
    """Send SR; return interpret_rs fields plus timeout/op/payload metadata."""
    await ws.send(make_frame(OP_STATUS_REQ))
    try:
        op, payload = await recv_binary(ws, timeout=timeout)
    except asyncio.TimeoutError:
        return {
            "binary_capable": False,
            "timeout": True,
            "reason": f"no RS binary frame within {timeout}s (no binary support)",
            "op": None,
            "payload": b"",
        }
    info = interpret_rs(op, payload)
    info["timeout"] = False
    info["op"] = op
    info["payload"] = payload
    return info


def format_rs_summary(info: dict) -> str:
    """One-line human summary for CLI status command."""
    if info.get("timeout"):
        return f"TIMEOUT — {info['reason']}"
    if not info["binary_capable"]:
        return f"NOT SUPPORTED — {info['reason']}"
    parts = [info["state_name"]]
    st = info["state_byte"]
    if st in (S_UPLOAD, S_DLOAD) and info["total"] is not None:
        total = info["total"]
        done = info["done"]
        last_id = info["last_id"]
        pct = done * 100 // total if total else 0
        parts.append(f"{done}/{total} bytes ({pct}%), last packet={last_id}")
    if st == S_OK:
        if info.get("v1_announced"):
            parts.append(f"[ESP3D WS Protocol V1, version byte={info['protocol_version']}]")
        else:
            parts.append("[legacy RS: single byte; assume V1 transfer opcodes]")
    return "  ".join(parts)


async def ensure_binary_file_transfer(ws, binary_timeout: float) -> dict:
    """Require SR/RS success before upload/download. Raises WsTransferError."""
    info = await exchange_sr_rs(ws, timeout=binary_timeout)
    if info["binary_capable"]:
        return info
    if info.get("timeout"):
        raise WsTransferError(
            "Binary file transfer not supported or server did not answer: "
            + info["reason"]
            + ". See docs/esp3d_websocket_protocol_v1.md (capability probe)."
        )
    raise WsTransferError(
        "Binary probe failed: "
        + info["reason"]
        + ". This socket may be text-only."
    )


async def upload_file(ws, local_path: str, remote_dir: str):
    """Upload local_path to remote_dir on the ESP."""
    if not os.path.isfile(local_path):
        raise WsTransferError(f"Local file does not exist: {local_path}")
    filename = os.path.basename(local_path)
    file_size = os.path.getsize(local_path)

    path_b = remote_dir.encode("utf-8")
    name_b = filename.encode("utf-8")
    su_payload = (
        bytes([len(path_b)]) + path_b +
        bytes([len(name_b)]) + name_b +
        struct.pack("<I", file_size)
    )
    print(f"  → SU  {remote_dir}/{filename} ({file_size} bytes)")
    await ws.send(make_frame(OP_UPLOAD_START, su_payload))

    op, payload = await recv_binary(ws)
    remote_label = f"{remote_dir}/{filename}"
    msg = _format_start_ack_failure("US", op, payload, remote_label)
    if msg:
        print(f"  ← US  ERROR")
        raise WsTransferError(msg)
    print("  ← US  OK — starting data transfer")

    packet_id = 0
    sent_bytes = 0
    with open(local_path, "rb") as f:
        while True:
            data = f.read(PACKET_SIZE)
            if not data:
                break
            pkt_payload = struct.pack("<I", packet_id) + data
            await ws.send(make_frame(OP_UPLOAD_PKT, pkt_payload))

            op, pl = await recv_binary(ws)
            if op != "PU":
                print("  ← PU  ERROR")
                raise WsTransferError(
                    f"Expected packet ACK (PU) for packet {packet_id}, "
                    f"got opcode {op!r} ({remote_label})"
                )
            if not pl:
                print("  ← PU  ERROR")
                raise WsTransferError(
                    f"Empty PU response for packet {packet_id} ({remote_label})"
                )
            if pl[0] != S_OK:
                st = pl[0]
                reason = BINARY_ACK_STATUS_REASON.get(
                    st, f"unknown status 0x{st:02X}"
                )
                srv_id = None
                if len(pl) >= 5:
                    srv_id = struct.unpack_from("<I", pl, 1)[0]
                id_note = ""
                if srv_id is not None and srv_id != 0xFFFFFFFF:
                    id_note = f", server PU id={srv_id}"
                elif srv_id == 0xFFFFFFFF:
                    id_note = ", server PU id=(unknown)"
                print(f"  ← PU  ERROR  packet={packet_id}")
                raise WsTransferError(
                    f"PU {reason} — sent packet {packet_id}{id_note} ({remote_label})"
                )
            ack_id = struct.unpack_from("<I", pl, 1)[0]
            if ack_id != packet_id:
                print(f"  ← PU  ERROR  packet={packet_id}")
                raise WsTransferError(
                    f"Packet ID mismatch: ESP ack {ack_id}, sent {packet_id} "
                    f"({remote_label})"
                )

            sent_bytes += len(data)
            pct = sent_bytes * 100 // file_size
            print(f"  ↔ UP/PU  packet={packet_id}  {sent_bytes}/{file_size} bytes ({pct}%)",
                  end="\r")
            packet_id += 1

    print()
    await ws.send(make_frame(OP_UPLOAD_END))
    op, pl = await recv_binary(ws)
    if op != "UE":
        print("  ← UE  ERROR")
        raise WsTransferError(
            f"Expected upload end ACK (UE), got opcode {op!r} ({remote_label})"
        )
    if not pl:
        print("  ← UE  ERROR")
        raise WsTransferError(f"Empty UE response ({remote_label})")
    if pl[0] != S_OK:
        st = pl[0]
        reason = BINARY_ACK_STATUS_REASON.get(
            st, f"unknown status 0x{st:02X}"
        )
        print("  ← UE  ERROR")
        raise WsTransferError(f"UE {reason} ({remote_label})")
    print(f"  ← UE  OK — upload complete ({packet_id} packets)")


async def download_file(ws, remote_dir: str, remote_name: str, local_path: str):
    """Download remote_dir/remote_name from the ESP to local_path."""
    path_b = remote_dir.encode("utf-8")
    name_b = remote_name.encode("utf-8")
    sd_payload = bytes([len(path_b)]) + path_b + bytes([len(name_b)]) + name_b

    print(f"  → SD  {remote_dir}/{remote_name}")
    await ws.send(make_frame(OP_DLOAD_START, sd_payload))

    op, payload = await recv_binary(ws)
    remote_label = f"{remote_dir}/{remote_name}"
    msg = _format_start_ack_failure("DS", op, payload, remote_label)
    if msg:
        print("  ← DS  ERROR")
        raise WsTransferError(msg)
    if len(payload) < 5:
        raise WsTransferError(
            f"DS response too short ({len(payload)} bytes) for {remote_label}"
        )
    file_size = struct.unpack_from("<I", payload, 1)[0]
    print(f"  ← DS  OK — file size = {file_size} bytes")

    received = 0
    last_id = None
    with open(local_path, "wb") as f:
        while True:
            op, pl = await recv_binary(ws, timeout=30.0)

            if op == "ED":
                # End of download — send ACK
                await ws.send(make_frame(OP_DLOAD_ENDACK, bytes([S_OK])))
                break

            if op != "DP":
                raise RuntimeError(f"Expected DP or ED, got {op}")
            if len(pl) < 4:
                raise RuntimeError("DP payload too short")

            packet_id = struct.unpack_from("<I", pl)[0]
            data = pl[4:]

            # NAK if unexpected packet
            if last_id is not None and packet_id != last_id + 1:
                print(f"\n  NAK: expected {last_id + 1}, got {packet_id}")
                nak_payload = struct.pack("<I", last_id + 1)
                await ws.send(make_frame(OP_NAK, nak_payload))
                continue

            f.write(data)
            received += len(data)
            last_id = packet_id
            pct = received * 100 // file_size if file_size else 0
            print(f"  ↔ DP/PD  packet={packet_id}  {received}/{file_size} bytes ({pct}%)",
                  end="\r")

            ack_payload = bytes([S_OK]) + struct.pack("<I", packet_id)
            await ws.send(make_frame(OP_DLOAD_PKTACK, ack_payload))

    print()
    print(f"  ← ED/DE  download complete ({received} bytes) → {local_path}")


async def abort_transfer(ws):
    """Send an abort command."""
    cmd_payload = bytes([ord('A')])
    await ws.send(make_frame(OP_COMMAND, cmd_payload))
    print("  → CM(A)  abort sent")


def normalize_path_for_list(path: str, flash_cmd: bool) -> str:
    """ESP720 expects flash paths relative to VFS root as '/' or '/subdir', not '/fs'
    (same as omitting the path: firmware defaults to '/'). Map '/fs' to '/' for convenience."""
    if not flash_cmd:
        return path
    p = path.strip()
    if not p:
        return "/"
    rootish = p.rstrip("/") or "/"
    if rootish.lower() == "/fs":
        return "/"
    return path


async def list_files(ws, path: str, *, text_mode: bool, raw_json: bool):
    """List files: default json=yes (ESP720/740); --text uses legacy line protocol."""
    if path.startswith("/sd"):
        cmd_name = "ESP740"
        send_path = path
    else:
        cmd_name = "ESP720"
        send_path = normalize_path_for_list(path, flash_cmd=True)

    if text_mode:
        command = f"[{cmd_name}]{send_path}\n"
    else:
        command = f"[{cmd_name}]{send_path} json=yes\n"

    if send_path != path.strip():
        print(f"  (flash list: {path!r} -> {send_path!r})")
    print(f"  → {command.strip()}")
    await ws.send(command)

    if not text_mode:
        try:
            doc = await recv_json_document(ws, timeout=15.0)
        except ValueError as exc:
            print(f"  ERROR: {exc}")
            return
        print_listing_from_json(doc, raw=raw_json)
        return

    list_stop_lines = (
        "No SD",
        "SD busy",
        "Flash not available",
        "Flash partition not mounted",
    )
    list_stop_prefixes = ("Cannot open :", "Path inccorrect", "Path incorrect")
    lines = await recv_text_responses(
        ws,
        timeout=10.0,
        end_marker="ok",
        stop_lines=list_stop_lines,
        stop_prefixes=list_stop_prefixes,
        end_line_prefixes=("Total:",),
    )
    if not lines:
        print("  (no response — timeout)")
        return
    for line in lines:
        if line.lower() == "ok":
            continue
        low = line.lower()
        if low == "no sd":
            print("  ERROR: No SD card (missing, not mounted, or firmware built without SD).")
            continue
        if low == "sd busy":
            print("  ERROR: SD busy — try again in a moment.")
            continue
        if low in ("flash not available", "flash partition not mounted"):
            print(f"  ERROR: {line}")
            continue
        if low.startswith("cannot open :") or low.startswith("path inccorrect") or low.startswith("path incorrect"):
            print(f"  ERROR: {line}")
            continue
        if line.startswith("D:") or line.startswith("F:"):
            tag, _, rest = line.partition(":")
            parts = rest.rsplit(None, 1)
            name = parts[0]
            size = f"  [{parts[1]} B]" if tag == "F" and len(parts) == 2 else ""
            kind = "DIR " if tag == "D" else "FILE"
            print(f"  {kind}  {name}{size}")
        else:
            print(f"  {line}")
    last = lines[-1].lower() if lines else ""
    list_complete = (
        last == "ok"
        or any(
            ln.lower().startswith("total:") and "available:" in ln.lower()
            for ln in lines
        )
    )
    if list_complete:
        print("  --- done ---")
    elif last in (
        "no sd",
        "sd busy",
        "flash not available",
        "flash partition not mounted",
    ) or last.startswith("cannot open :") or last.startswith("path inccorrect") or last.startswith("path incorrect"):
        print("  --- failed ---")
    else:
        print("  (incomplete — unexpected response end)")


# ---------------------------------------------------------------------------
# Main command handlers
# ---------------------------------------------------------------------------

def file_md5(path: str) -> str:
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


async def run_list(host, port, path, *, text_mode: bool, raw_json: bool):
    url = data_ws_url(host, port)
    async with websockets.connect(url, subprotocols=[SUBPROTOCOL]) as ws:
        welcome = await asyncio.wait_for(ws.recv(), timeout=5.0)
        w = welcome.strip() if isinstance(welcome, str) else str(welcome)
        print_handshake_ok(host, port, w)
        print("  (list uses TEXT [ESP720]/[ESP740] only; no SR/RS probe)")
        await list_files(ws, path, text_mode=text_mode, raw_json=raw_json)


async def run_status(host, port, binary_timeout: float):
    url = data_ws_url(host, port)
    async with websockets.connect(url, subprotocols=[SUBPROTOCOL]) as ws:
        welcome = await asyncio.wait_for(ws.recv(), timeout=5.0)
        w = welcome.strip() if isinstance(welcome, str) else str(welcome)
        print_handshake_ok(host, port, w)
        info = await exchange_sr_rs(ws, timeout=binary_timeout)
        print_status_probe_line(format_rs_summary(info))
        if info.get("binary_capable"):
            print("  → V1 binary file transfer may be used (upload/download).")
        else:
            print("  → Do not assume upload/download; fix handshake path/subprotocol or server.")


async def run_upload(host, port, local_path, remote_dir, *, binary_timeout: float = DEFAULT_BINARY_PROBE_TIMEOUT,
                     skip_binary_check: bool = False):
    url = data_ws_url(host, port)
    async with websockets.connect(url, subprotocols=[SUBPROTOCOL]) as ws:
        welcome = await asyncio.wait_for(ws.recv(), timeout=5.0)
        w = welcome.strip() if isinstance(welcome, str) else str(welcome)
        print_handshake_ok(host, port, w)
        if not skip_binary_check:
            probe = await ensure_binary_file_transfer(ws, binary_timeout)
            print_status_probe_line(format_rs_summary(probe))
        await upload_file(ws, local_path, remote_dir)


async def run_download(host, port, remote_dir, remote_name, local_path=None, *,
                       binary_timeout: float = DEFAULT_BINARY_PROBE_TIMEOUT,
                       skip_binary_check: bool = False):
    """If local_path is omitted, save in the current directory using remote_name (basename)."""
    used_default_name = local_path is None
    if local_path is None:
        local_path = os.path.basename(remote_name)
    url = data_ws_url(host, port)
    async with websockets.connect(url, subprotocols=[SUBPROTOCOL]) as ws:
        welcome = await asyncio.wait_for(ws.recv(), timeout=5.0)
        w = welcome.strip() if isinstance(welcome, str) else str(welcome)
        print_handshake_ok(host, port, w)
        if used_default_name:
            print(f"  → local file: {os.path.abspath(local_path)}")
        if not skip_binary_check:
            probe = await ensure_binary_file_transfer(ws, binary_timeout)
            print_status_probe_line(format_rs_summary(probe))
        await download_file(ws, remote_dir, remote_name, local_path)


async def run_roundtrip(host, port, local_path, remote_dir, *, binary_timeout: float = DEFAULT_BINARY_PROBE_TIMEOUT,
                        skip_binary_check: bool = False):
    """Upload a file, download it back, compare MD5."""
    filename = os.path.basename(local_path)
    received_path = local_path + ".received"
    url = data_ws_url(host, port)

    async with websockets.connect(url, subprotocols=[SUBPROTOCOL]) as ws:
        welcome = await asyncio.wait_for(ws.recv(), timeout=5.0)
        w = welcome.strip() if isinstance(welcome, str) else str(welcome)
        print_handshake_ok(host, port, w)
        if not skip_binary_check:
            probe = await ensure_binary_file_transfer(ws, binary_timeout)
            print_status_probe_line(format_rs_summary(probe))

        print("\n[1/2] Upload")
        await upload_file(ws, local_path, remote_dir)

        print("\n[2/2] Download")
        await download_file(ws, remote_dir, filename, received_path)

    md5_orig = file_md5(local_path)
    md5_recv = file_md5(received_path)
    print(f"\nOriginal MD5 : {md5_orig}")
    print(f"Received MD5 : {md5_recv}")
    if md5_orig == md5_recv:
        print("✓ Round-trip OK — files are identical")
    else:
        print("✗ MISMATCH — files differ!")
        sys.exit(1)
    os.remove(received_path)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _run_async(coro):
    """Run coroutine; turn WsTransferError into stderr + exit 1."""
    try:
        asyncio.run(coro)
    except WsTransferError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="ESP3D WebSocket Protocol V1 — data /wsdata file transfer test tool"
    )
    parser.add_argument("--host", default="192.168.1.100", help="ESP IP address")
    parser.add_argument("--port", type=int, default=8282, help="WebSocket port")
    parser.add_argument(
        "--binary-timeout",
        type=float,
        default=DEFAULT_BINARY_PROBE_TIMEOUT,
        metavar="SEC",
        help="Timeout for check (2) SR→RS only; handshake (1) uses welcome recv (default: %(default)s)",
    )
    parser.add_argument(
        "--skip-binary-check",
        action="store_true",
        help="Skip SR/RS probe before upload/download/roundtrip (not recommended)",
    )

    sub = parser.add_subparsers(dest="cmd", required=True)

    sub.add_parser("status", help="Query binary status (SR/RS) — ESP3D WS Protocol V1 probe")

    ls = sub.add_parser("list", help="List files via [ESP720]/[ESP740] (text channel)")
    ls.add_argument(
        "path",
        help="Remote path: flash root use / (or /fs as alias); SD e.g. /sd/gcodes",
    )
    ls.add_argument(
        "--text",
        action="store_true",
        help="Legacy plain-text listing instead of json=yes",
    )
    ls.add_argument(
        "--raw-json",
        action="store_true",
        help="Print full JSON document (implies structured mode; not with --text)",
    )

    up = sub.add_parser("upload", help="Upload a file to the ESP")
    up.add_argument("local_path", help="Local file to upload")
    up.add_argument("remote_dir", help="Remote directory, e.g. /fs")

    dl = sub.add_parser("download", help="Download a file from the ESP")
    dl.add_argument("remote_dir", help="Remote directory, e.g. /fs")
    dl.add_argument("remote_name", help="Filename on the ESP")
    dl.add_argument(
        "local_path",
        nargs="?",
        default=None,
        help="Local destination path (default: ./<remote_name> in current directory)",
    )

    rt = sub.add_parser("roundtrip", help="Upload + download + MD5 comparison")
    rt.add_argument("local_path", help="Local file to test")
    rt.add_argument("remote_dir", help="Remote directory, e.g. /fs")

    args = parser.parse_args()

    if args.cmd == "status":
        asyncio.run(run_status(args.host, args.port, args.binary_timeout))
    elif args.cmd == "list":
        asyncio.run(
            run_list(
                args.host,
                args.port,
                args.path,
                text_mode=args.text,
                raw_json=args.raw_json,
            )
        )
    elif args.cmd == "upload":
        _run_async(
            run_upload(
                args.host,
                args.port,
                args.local_path,
                args.remote_dir,
                binary_timeout=args.binary_timeout,
                skip_binary_check=args.skip_binary_check,
            )
        )
    elif args.cmd == "download":
        _run_async(
            run_download(
                args.host,
                args.port,
                args.remote_dir,
                args.remote_name,
                args.local_path,
                binary_timeout=args.binary_timeout,
                skip_binary_check=args.skip_binary_check,
            )
        )
    elif args.cmd == "roundtrip":
        _run_async(
            run_roundtrip(
                args.host,
                args.port,
                args.local_path,
                args.remote_dir,
                binary_timeout=args.binary_timeout,
                skip_binary_check=args.skip_binary_check,
            )
        )


if __name__ == "__main__":
    main()
