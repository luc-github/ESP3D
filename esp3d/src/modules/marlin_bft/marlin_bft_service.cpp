/*
  marlin_bft_service.cpp - Marlin Binary File Transfer client

  Copyright (c) 2026 ESP3D contributors

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#include "marlin_bft_service.h"

#if defined(MARLIN_BINARY_FILE_TRANSFER_FEATURE)

#include "../../core/esp3d_commands.h"
#include "../printer_link/printer_link_service.h"
#include "../serial/serial_service.h"
#if defined(USB_SERIAL_FEATURE)
#include "../usb-serial/usb_serial_service.h"
#endif

MarlinBftService marlin_bft_service;

namespace {
constexpr uint8_t PROTOCOL_CONTROL = 0;
constexpr uint8_t PROTOCOL_FILE_TRANSFER = 1;
constexpr uint8_t CONTROL_SYNC = 1;
constexpr uint8_t CONTROL_CLOSE = 2;
constexpr uint8_t FILE_QUERY = 0;
constexpr uint8_t FILE_OPEN = 1;
constexpr uint8_t FILE_CLOSE = 2;
constexpr uint8_t FILE_WRITE = 3;
constexpr uint8_t FILE_ABORT = 4;
constexpr uint16_t HEADER_TOKEN = 0xB5AD;
constexpr char PRINTER_LINK_OWNER[] = "binary-transfer";
}

bool MarlinBftService::ownsPrinterLink() const { return _linkCaptured; }

bool MarlinBftService::start(const char *source, const char *destination,
                             const char *compression, bool dummy) {
  if (ownsPrinterLink()) {
    return false;
  }
  if (!source || !destination || !source[0] || !destination[0]) {
    _error = "Source and destination are required";
    _state = State::Failed;
    return false;
  }

  String sourcePath(source), destinationPath(destination);
  sourcePath.trim();
  destinationPath.trim();
  if (!sourcePath.length() || !destinationPath.length() ||
      destinationPath.indexOf("..") >= 0 || destinationPath.length() > 240 ||
      destinationPath.indexOf('\n') >= 0 || destinationPath.indexOf('\r') >= 0) {
    _error = "Invalid destination path";
    _state = State::Failed;
    return false;
  }

  String mode = compression ? compression : "auto";
  mode.toLowerCase();
  if (mode == "auto")
    _compressionMode = CompressionMode::Auto;
  else if (mode == "none")
    _compressionMode = CompressionMode::None;
  else if (mode == "heatshrink")
    _compressionMode = CompressionMode::Heatshrink;
  else {
    _error = "Invalid compression mode";
    _state = State::Failed;
    return false;
  }

  _sourceFs = ESP_GBFS::getFSType(sourcePath.c_str());
  if (_sourceFs != FS_FLASH && _sourceFs != FS_SD) {
    _error = "Source must begin with /FS or /SD";
    _state = State::Failed;
    return false;
  }
  if (!ESP_GBFS::accessFS(_sourceFs)) {
    _error = "Source filesystem is busy or unavailable";
    _state = State::Failed;
    return false;
  }
  _sourceFile = ESP_GBFS::open(sourcePath.c_str(), ESP_FILE_READ);
  if (!_sourceFile || _sourceFile.isDirectory()) {
    releaseSource();
    _error = "Cannot open source file";
    _state = State::Failed;
    return false;
  }

  _sourcePath = sourcePath;
  _destination = destinationPath;
  _sourceSize = _sourceFile.size();
  _sourceBytesRead = 0;
  _wireBytesSent = 0;
  _totalRetries = 0;
  _inputSize = _inputOffset = 0;
  _payloadSize = 0;
  _sourceEof = false;
  _encoderFinished = false;
  _dummy = dummy;
  _finalResult = FinalResult::Completed;
  _binaryMode = false;
  _cancelRequested = false;
  _capabilitySeen = _capabilitySupported = _asciiOkReceived = false;
  _enteredReceived = _syncReceived = _resendRequested = false;
  _streamErrorReceived = _pftErrorReceived = false;
  _ackReceived = _pftReceived = _pftSuccess = false;
  _heatshrinkOffered = _compressionEnabled = false;
  _windowBits = _lookaheadBits = 0;
  _sync = _packetSync = _packetRetries = 0;
  _serverPayloadSize = _packetSize = 0;
  _lineSize = 0;
  _error = "";
  _startedAt = millis();

  if (!printer_link_service.acquire(PRINTER_LINK_OWNER, receivePrinterData,
                                    this)) {
    releaseSource();
    _error = "Printer link is already in use";
    _state = State::Failed;
    return false;
  }
  _linkCaptured = true;
  _state = State::CheckingCapabilities;

  static const uint8_t capabilityCommand[] = "M115\n";
  if (!writePrinter(capabilityCommand, sizeof(capabilityCommand) - 1)) {
    fail("Cannot send M115");
    return false;
  }
  _deadline = millis() + RESPONSE_TIMEOUT_MS;
  return true;
}

void MarlinBftService::cancel() {
  if (ownsPrinterLink()) _cancelRequested = true;
}

void MarlinBftService::receivePrinterData(const uint8_t *data, size_t size,
                                          void *context) {
  if (context)
    static_cast<MarlinBftService *>(context)->consumeRx(data, size);
}

void MarlinBftService::consumeRx(const uint8_t *data, size_t size) {
  if (!_linkCaptured) return;
  for (size_t i = 0; i < size; ++i) {
    const char c = static_cast<char>(data[i]);
    if (c == '\n' || c == '\r') {
      if (_lineSize) {
        _line[_lineSize] = '\0';
        processLine(_line);
        _lineSize = 0;
      }
    }
    else if (_lineSize + 1 < sizeof(_line)) {
      _line[_lineSize++] = c;
    }
    else {
      _lineSize = 0;
    }
  }
}

void MarlinBftService::processLine(const char *line) {
  if (!line || !line[0]) return;

  if (_state == State::CheckingCapabilities) {
    const char *capability = strstr(line, "Cap:BINARY_FILE_TRANSFER:");
    if (capability) {
      _capabilitySeen = true;
      capability += strlen("Cap:BINARY_FILE_TRANSFER:");
      _capabilitySupported = capability[0] == '1';
    }
    if (!strncmp(line, "ok", 2)) _asciiOkReceived = true;
    return;
  }

  if (_state == State::EnteringBinaryMode &&
      strstr(line, "Switching to Binary Protocol")) {
    _enteredReceived = true;
    return;
  }

  if (_state == State::Syncing && line[0] == 's' && line[1] == 's') {
    unsigned sync = 0, payload = 0;
    if (sscanf(line + 2, "%u,%u", &sync, &payload) == 2 && payload > 0) {
      _sync = static_cast<uint8_t>(sync);
      _serverPayloadSize = min(payload, static_cast<unsigned>(MAX_PAYLOAD_SIZE));
      _syncReceived = true;
    }
    return;
  }

  if (line[0] == 'o' && line[1] == 'k') {
    const unsigned received = strtoul(line + 2, nullptr, 10);
    if (static_cast<uint8_t>(received) == _packetSync) _ackReceived = true;
    return;
  }
  if (line[0] == 'r' && line[1] == 's') {
    const unsigned requested = strtoul(line + 2, nullptr, 10);
    if (static_cast<uint8_t>(requested) == _packetSync)
      _resendRequested = true;
    return;
  }
  if (!strncmp(line, "fe", 2)) {
    _streamErrorReceived = true;
    return;
  }
  if (!strncmp(line, "PFT:", 4)) {
    _pftReceived = true;
    _pftSuccess = strstr(line, "PFT:success") == line;
    if (_state == State::Querying) {
      _pftSuccess = strstr(line, "PFT:version:") == line;
      const char *capability = strstr(line, ":compression:heatshrink,");
      if (capability) {
        unsigned window = 0, lookahead = 0;
        if (sscanf(capability, ":compression:heatshrink,%u,%u", &window,
                   &lookahead) == 2) {
          _heatshrinkOffered = true;
          _windowBits = static_cast<uint8_t>(window);
          _lookaheadBits = static_cast<uint8_t>(lookahead);
        }
      }
    }
    else if (_state == State::Writing) {
      _pftErrorReceived = true;
    }
  }
}

void MarlinBftService::handle() {
  if (!ownsPrinterLink()) return;

  if (_streamErrorReceived) {
    _streamErrorReceived = false;
    fail("Marlin reported a binary stream error");
    return;
  }
  if (_pftErrorReceived) {
    _pftErrorReceived = false;
    fail("Marlin reported a file write error");
    return;
  }

  if (_state == State::CheckingCapabilities && _asciiOkReceived) {
    if (!_capabilitySeen || !_capabilitySupported) {
      fail("Marlin does not advertise BINARY_FILE_TRANSFER");
      return;
    }
    _asciiOkReceived = false;
    _state = State::EnteringBinaryMode;
    static const uint8_t enterCommand[] = "M28 B1\n";
    if (!writePrinter(enterCommand, sizeof(enterCommand) - 1)) {
      fail("Cannot send M28 B1");
      return;
    }
    _deadline = millis() + RESPONSE_TIMEOUT_MS;
    return;
  }
  if (_state == State::EnteringBinaryMode && _enteredReceived) {
    _enteredReceived = false;
    _binaryMode = true;
    _state = State::Syncing;
    sendPacket(PROTOCOL_CONTROL, CONTROL_SYNC, nullptr, 0);
    return;
  }
  if (_state == State::Syncing && _syncReceived) {
    _syncReceived = false;
    _state = State::Querying;
    sendPacket(PROTOCOL_FILE_TRANSFER, FILE_QUERY, nullptr, 0);
    return;
  }
  if (_resendRequested) {
    _resendRequested = false;
    if (!resendPacket()) fail("Marlin requested too many retries");
    return;
  }

  if (_cancelRequested && _state != State::Aborting &&
      _state != State::ClosingConnection) {
    sendAbort(FinalResult::Cancelled, "Transfer cancelled");
    return;
  }

  processResponses();
  if (!ownsPrinterLink()) return;

  if (static_cast<int32_t>(millis() - _deadline) >= 0) {
    if (_state == State::CheckingCapabilities) {
      fail("Marlin did not answer the BINARY_FILE_TRANSFER capability query");
    }
    else if (_state == State::EnteringBinaryMode) {
      fail("Marlin did not enter Binary File Transfer mode");
    }
    else if (_ackReceived && (_state == State::Querying ||
                              _state == State::Opening ||
                              _state == State::ClosingFile ||
                              _state == State::Aborting)) {
      fail("Marlin operation response timeout");
    }
    else if (!resendPacket()) {
      fail("Printer response timeout");
    }
  }
}

void MarlinBftService::processResponses() {
  if (_state == State::Syncing) return;
  if (!_ackReceived) return;

  if (_state == State::Writing) {
    _sync++;
    _ackReceived = false;
    sendNextPayload();
    return;
  }
  if (_state == State::ClosingConnection) {
    _sync++;
    finish(_finalResult);
    return;
  }
  if (!_pftReceived) return;

  _sync++;
  if (!_pftSuccess) {
    fail("Marlin rejected the file-transfer request");
    return;
  }

  if (_state == State::Querying) {
    if (!prepareCompression()) return;
    const size_t nameSize = _destination.length() + 1;
    if (nameSize + 2 > _serverPayloadSize) {
      fail("Destination name exceeds Marlin payload size");
      return;
    }
    _payload[0] = _dummy ? 1 : 0;
    _payload[1] = _compressionEnabled ? 1 : 0;
    memcpy(&_payload[2], _destination.c_str(), nameSize);
    _state = State::Opening;
    sendPacket(PROTOCOL_FILE_TRANSFER, FILE_OPEN, _payload, nameSize + 2);
  }
  else if (_state == State::Opening) {
    sendNextPayload();
  }
  else if (_state == State::ClosingFile) {
    sendConnectionClose();
  }
  else if (_state == State::Aborting) {
    sendConnectionClose();
  }
}

bool MarlinBftService::prepareCompression() {
  _compressionEnabled = _compressionMode != CompressionMode::None &&
                        _heatshrinkOffered;
  if (_compressionMode == CompressionMode::Heatshrink &&
      !_heatshrinkOffered) {
    fail("Marlin does not offer heatshrink compression");
    return false;
  }
  if (_compressionEnabled) {
    if (_windowBits < 4 || _windowBits > 15 || _lookaheadBits < 3 ||
        _lookaheadBits >= _windowBits) {
      fail("Marlin announced invalid heatshrink parameters");
      return false;
    }
    _encoder = heatshrink_encoder_alloc(_windowBits, _lookaheadBits);
    if (!_encoder) {
      fail("Cannot allocate heatshrink encoder");
      return false;
    }
    heatshrink_encoder_reset(_encoder);
  }
  return true;
}

bool MarlinBftService::prepareNextPayload() {
  _payloadSize = 0;
  if (!_compressionEnabled) {
    _payloadSize = _sourceFile.read(_payload, _serverPayloadSize);
    _sourceBytesRead += _payloadSize;
    return _payloadSize != 0;
  }

  size_t guard = 0;
  while (_payloadSize < _serverPayloadSize && guard++ < 4096) {
    size_t produced = 0;
    const HSE_poll_res pollResult = heatshrink_encoder_poll(
        _encoder, &_payload[_payloadSize], _serverPayloadSize - _payloadSize,
        &produced);
    _payloadSize += produced;
    if (pollResult == HSER_POLL_MORE || _payloadSize == _serverPayloadSize)
      continue;

    if (_encoderFinished) break;

    if (_inputOffset < _inputSize) {
      size_t consumed = 0;
      const HSE_sink_res sinkResult = heatshrink_encoder_sink(
          _encoder, &_input[_inputOffset], _inputSize - _inputOffset,
          &consumed);
      if (sinkResult < 0) {
        fail("heatshrink rejected input");
        return false;
      }
      _inputOffset += consumed;
      continue;
    }

    if (!_sourceEof) {
      _inputSize = _sourceFile.read(_input, sizeof(_input));
      _inputOffset = 0;
      _sourceBytesRead += _inputSize;
      _sourceEof = _inputSize == 0;
      if (!_sourceEof) continue;
    }

    const HSE_finish_res finishResult = heatshrink_encoder_finish(_encoder);
    if (finishResult < 0) {
      fail("heatshrink finalization failed");
      return false;
    }
    _encoderFinished = finishResult == HSER_FINISH_DONE;
    if (_encoderFinished) break;
  }
  if (guard >= 4096) {
    fail("heatshrink encoder made no progress");
    return false;
  }
  return _payloadSize != 0;
}

bool MarlinBftService::sendNextPayload() {
  if (!prepareNextPayload()) {
    if (_state == State::Failed || _state == State::Aborting) return false;
    _state = State::ClosingFile;
    return sendPacket(PROTOCOL_FILE_TRANSFER, FILE_CLOSE, nullptr, 0);
  }
  _state = State::Writing;
  return sendPacket(PROTOCOL_FILE_TRANSFER, FILE_WRITE, _payload, _payloadSize);
}

uint16_t MarlinBftService::checksum(uint16_t value, uint8_t byte) {
  const uint16_t low = ((value & 0xFF) + byte) % 255;
  return ((((value >> 8) + low) % 255) << 8) | low;
}

bool MarlinBftService::sendPacket(uint8_t protocol, uint8_t type,
                                  const uint8_t *payload,
                                  size_t payloadSize) {
  if (payloadSize > MAX_PAYLOAD_SIZE) {
    fail("Binary packet is too large");
    return false;
  }
  _packet[0] = HEADER_TOKEN & 0xFF;
  _packet[1] = HEADER_TOKEN >> 8;
  _packet[2] = _sync;
  _packet[3] = (protocol << 4) | (type & 0x0F);
  _packet[4] = payloadSize & 0xFF;
  _packet[5] = (payloadSize >> 8) & 0xFF;

  uint16_t sum = 0;
  for (size_t i = 2; i < 6; ++i) sum = checksum(sum, _packet[i]);
  const uint16_t headerSum = sum;
  _packet[6] = headerSum & 0xFF;
  _packet[7] = headerSum >> 8;
  sum = checksum(sum, _packet[6]);
  sum = checksum(sum, _packet[7]);
  if (payloadSize) {
    memcpy(&_packet[8], payload, payloadSize);
    for (size_t i = 0; i < payloadSize; ++i)
      sum = checksum(sum, payload[i]);
  }
  if (payloadSize) {
    _packet[8 + payloadSize] = sum & 0xFF;
    _packet[9 + payloadSize] = sum >> 8;
  }
  _packetSize = payloadSize ? payloadSize + 10 : 8;
  _packetSync = _sync;
  _packetRetries = 0;
  _ackReceived = _pftReceived = _pftSuccess = false;
  if (!writePrinter(_packet, _packetSize)) {
    fail("Cannot write binary packet");
    return false;
  }
  _wireBytesSent += _packetSize;
  _deadline = millis() + RESPONSE_TIMEOUT_MS;
  return true;
}

bool MarlinBftService::resendPacket() {
  if (!_packetSize || _packetRetries >= MAX_RETRIES) return false;
  ++_packetRetries;
  ++_totalRetries;
  _ackReceived = false;
  if (!writePrinter(_packet, _packetSize)) return false;
  _wireBytesSent += _packetSize;
  _deadline = millis() + RESPONSE_TIMEOUT_MS;
  return true;
}

void MarlinBftService::sendAbort(FinalResult result, const char *reason) {
  _finalResult = result;
  if (reason) _error = reason;
  _cancelRequested = false;
  if (!_binaryMode) {
    finish(result);
    return;
  }
  _state = State::Aborting;
  sendPacket(PROTOCOL_FILE_TRANSFER, FILE_ABORT, nullptr, 0);
}

void MarlinBftService::sendConnectionClose() {
  _state = State::ClosingConnection;
  sendPacket(PROTOCOL_CONTROL, CONTROL_CLOSE, nullptr, 0);
}

void MarlinBftService::fail(const char *reason) {
  if (reason) _error = reason;
  if (_binaryMode && _state != State::Aborting &&
      _state != State::ClosingConnection) {
    sendAbort(FinalResult::Failed, reason);
  }
  else {
    finish(FinalResult::Failed);
  }
}

void MarlinBftService::finish(FinalResult result) {
  _binaryMode = false;
  _cancelRequested = false;
  if (_encoder) {
    heatshrink_encoder_free(_encoder);
    _encoder = nullptr;
  }
  releaseSource();
  if (result == FinalResult::Completed) {
    _error = "";
    _state = State::Completed;
  }
  else if (result == FinalResult::Cancelled) {
    _state = State::Cancelled;
  }
  else {
    _state = State::Failed;
  }
  if (_linkCaptured) {
    printer_link_service.release(PRINTER_LINK_OWNER);
    _linkCaptured = false;
  }
}

void MarlinBftService::releaseSource() {
  if (_sourceFile) _sourceFile.close();
  if (_sourceFs != FS_UNKNOWN) ESP_GBFS::releaseFS(_sourceFs);
  _sourceFs = FS_UNKNOWN;
}

bool MarlinBftService::writePrinter(const uint8_t *data, size_t size) {
  const ESP3DClientType output = esp3d_commands.getOutputClient();
#if defined(USB_SERIAL_FEATURE)
  if (output == ESP3DClientType::usb_serial)
    return esp3d_usb_serial_service.writeBytes(data, size) == size;
#endif
  if (output == ESP3DClientType::serial)
    return esp3d_serial_service.writeBytes(data, size) == size;
  return false;
}

const char *MarlinBftService::stateName(State state) {
  switch (state) {
    case State::Idle: return "idle";
    case State::CheckingCapabilities: return "checking_capabilities";
    case State::EnteringBinaryMode: return "entering";
    case State::Syncing: return "syncing";
    case State::Querying: return "querying";
    case State::Opening: return "opening";
    case State::Writing: return "writing";
    case State::ClosingFile: return "closing_file";
    case State::Aborting: return "aborting";
    case State::ClosingConnection: return "closing_connection";
    case State::Completed: return "completed";
    case State::Cancelled: return "cancelled";
    case State::Failed: return "failed";
  }
  return "unknown";
}

String MarlinBftService::jsonEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    if (c == '\\' || c == '"') escaped += '\\';
    if (c == '\n') escaped += "\\n";
    else if (c == '\r') escaped += "\\r";
    else escaped += c;
  }
  return escaped;
}

String MarlinBftService::statusJson() const {
  const bool active = ownsPrinterLink();
  const unsigned progress = _sourceSize
      ? static_cast<unsigned>((100ULL * _sourceBytesRead) / _sourceSize)
      : (active ? 0 : 100);
  String json;
  json.reserve(384);
  json = "{\"status\":\"";
  json += stateName(_state);
  json += "\",\"active\":";
  json += active ? "true" : "false";
  json += ",\"source\":\"" + jsonEscape(_sourcePath);
  json += "\",\"destination\":\"" + jsonEscape(_destination);
  json += "\",\"source_size\":" + String(_sourceSize);
  json += ",\"source_bytes_read\":" + String(_sourceBytesRead);
  json += ",\"wire_bytes_sent\":" + String(_wireBytesSent);
  json += ",\"progress\":" + String(progress);
  json += ",\"compression\":\"";
  json += _compressionEnabled ? "heatshrink" : "none";
  json += "\",\"window_bits\":" + String(_windowBits);
  json += ",\"lookahead_bits\":" + String(_lookaheadBits);
  json += ",\"retries\":" + String(_totalRetries);
  const uint32_t elapsed = _startedAt ? millis() - _startedAt : 0;
  json += ",\"elapsed_ms\":" + String(elapsed);
  json += ",\"error\":\"" + jsonEscape(_error) + "\"}";
  return json;
}

#endif  // MARLIN_BINARY_FILE_TRANSFER_FEATURE
