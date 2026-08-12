# Marlin Binary File Transfer

Enable `MARLIN_BINARY_FILE_TRANSFER_FEATURE` to copy a file from ESP3D flash
or its local SD card to storage managed by Marlin. Marlin must advertise
`Cap:BINARY_FILE_TRANSFER:1` in its `M115` response.

ESP3D negotiates the packet size and compression parameters. With
`compression=auto`, heatshrink is selected when Marlin offers it; otherwise
the file is sent uncompressed. While a transfer is active, ESP3D reserves the
printer connection and rejects other commands.

## HTTP API

Start a transfer with an authenticated POST request:

```text
POST /printer-sd-transfer?action=start&source=/FS/model.gcode&destination=/model.gcode&compression=auto
```

Use `/FS/...` for ESP flash and `/SD/...` for the ESP's local SD card. Optional
parameters are `compression=auto|none|heatshrink` and `dummy=true|false`.

Poll progress with:

```text
GET /printer-sd-transfer?action=status
```

The JSON response includes `status`, `active`, source and wire byte counts,
`progress`, negotiated compression parameters, retries, elapsed time, and an
error message. Terminal states are `completed`, `cancelled`, and `failed`.

Cancel an active transfer with:

```text
POST /printer-sd-transfer?action=cancel
```

The WebUI should poll status until `active` becomes false, then refresh the
Marlin SD listing after a successful transfer.
