# Exclusive Printer-Link Access

`PrinterLinkService` lets a protocol temporarily reserve the connection to the
printer. This is useful for binary or otherwise stateful protocols that cannot
share the byte stream with terminal commands or automatic polling.

## API

Capture the link with an owner name and an optional receive callback:

```cpp
printer_link_service.acquire("binary-transfer", receiveData, context);
```

While captured:

- all printer RX is delivered only to `receiveData`;
- a null callback deliberately discards printer RX;
- normal commands, including WebUI polling, are rejected before serial TX;
- other clients cannot capture the link;
- the WebUI receives `printerLink:captured:binary-transfer`.

Release it using the same owner name:

```cpp
printer_link_service.release("binary-transfer");
```

Only the current owner can release the link. Release restores normal terminal
RX and printer TX and emits `printerLink:released`. A newly connected WebUI is
sent the current state immediately, so it can disable its terminal input and
polling controls even when capture began before the page connected.

Every successful `acquire()` must have a matching `release()` on success,
cancellation, timeout, and error paths. The service does not transmit protocol
bytes itself; the owner continues to use the selected serial service directly.
