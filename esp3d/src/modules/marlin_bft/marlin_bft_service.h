/*
  marlin_bft_service.h - Marlin Binary File Transfer client

  Copyright (c) 2026 ESP3D contributors

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "../../include/esp3d_config.h"

#if defined(MARLIN_BINARY_FILE_TRANSFER_FEATURE)

#include <Arduino.h>
extern "C" {
#include <heatshrink_encoder.h>
}

#include "../filesystem/esp_globalFS.h"

class MarlinBftService final {
 public:
  enum class State : uint8_t {
    Idle,
    CheckingCapabilities,
    EnteringBinaryMode,
    Syncing,
    Querying,
    Opening,
    Writing,
    ClosingFile,
    Aborting,
    ClosingConnection,
    Completed,
    Cancelled,
    Failed
  };

  bool start(const char *source, const char *destination,
             const char *compression = "auto", bool dummy = false);
  void cancel();
  void handle();
  bool ownsPrinterLink() const;
  String statusJson() const;

 private:
  enum class CompressionMode : uint8_t { Auto, None, Heatshrink };
  enum class FinalResult : uint8_t { Completed, Cancelled, Failed };

  static constexpr size_t MAX_PAYLOAD_SIZE = 512;
  static constexpr size_t MAX_PACKET_SIZE = MAX_PAYLOAD_SIZE + 10;
  static constexpr uint32_t RESPONSE_TIMEOUT_MS = 2500;
  static constexpr uint8_t MAX_RETRIES = 5;

  bool writePrinter(const uint8_t *data, size_t size);
  static void receivePrinterData(const uint8_t *data, size_t size,
                                 void *context);
  void consumeRx(const uint8_t *data, size_t size);
  bool sendPacket(uint8_t protocol, uint8_t type, const uint8_t *payload,
                  size_t payloadSize);
  bool resendPacket();
  void processLine(const char *line);
  void processResponses();
  bool prepareCompression();
  bool prepareNextPayload();
  bool sendNextPayload();
  void sendAbort(FinalResult result, const char *reason);
  void sendConnectionClose();
  void finish(FinalResult result);
  void fail(const char *reason);
  void releaseSource();
  static uint16_t checksum(uint16_t value, uint8_t byte);
  static const char *stateName(State state);
  static String jsonEscape(const String &value);

  State _state = State::Idle;
  FinalResult _finalResult = FinalResult::Completed;
  CompressionMode _compressionMode = CompressionMode::Auto;
  ESP_GBFile _sourceFile;
  uint8_t _sourceFs = FS_UNKNOWN;
  String _sourcePath;
  String _destination;
  String _error;
  bool _dummy = false;
  bool _linkCaptured = false;
  bool _binaryMode = false;
  bool _cancelRequested = false;
  bool _capabilitySeen = false;
  bool _capabilitySupported = false;
  bool _asciiOkReceived = false;
  bool _enteredReceived = false;
  bool _syncReceived = false;
  bool _resendRequested = false;
  bool _streamErrorReceived = false;
  bool _pftErrorReceived = false;
  bool _ackReceived = false;
  bool _pftReceived = false;
  bool _pftSuccess = false;
  bool _heatshrinkOffered = false;
  bool _compressionEnabled = false;
  bool _sourceEof = false;
  bool _encoderFinished = false;
  uint8_t _windowBits = 0;
  uint8_t _lookaheadBits = 0;
  uint8_t _sync = 0;
  uint8_t _packetSync = 0;
  uint8_t _packetRetries = 0;
  uint32_t _totalRetries = 0;
  size_t _serverPayloadSize = 0;
  size_t _packetSize = 0;
  size_t _payloadSize = 0;
  size_t _inputSize = 0;
  size_t _inputOffset = 0;
  size_t _sourceSize = 0;
  size_t _sourceBytesRead = 0;
  size_t _wireBytesSent = 0;
  uint32_t _deadline = 0;
  uint32_t _startedAt = 0;
  uint8_t _packet[MAX_PACKET_SIZE] = {};
  uint8_t _payload[MAX_PAYLOAD_SIZE] = {};
  uint8_t _input[512] = {};
  char _line[256] = {};
  size_t _lineSize = 0;
  heatshrink_encoder *_encoder = nullptr;
};

extern MarlinBftService marlin_bft_service;

#endif  // MARLIN_BINARY_FILE_TRANSFER_FEATURE
