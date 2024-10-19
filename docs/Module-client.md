# Module client

## Summary

A module client is a class that represent a client, it handle commands from outside to the ESP3D, and handle commands from ESP3D to outside.

WebDav, FTP ,SSDP , MDNS are not module client, they are modules that handle in/out data differently.

The main clients are : Serial, USB Serial, Telnet, WebSocket are kind of full duplex, the http client is a half duplex client for everything but ESP3D commands, in that case it is a full duplex client.

## General description of data flow

The module client fill a buffer char by char until char `\n` or `r` is found or buffer is full,  the data come from outside, serial port, usb port, telnet, websocket.
The catch of the commands is done by specific library, and the library inform the module client that a command is received, the module client will then dispatch the command to the ESP3D.

To do that the module client will add some context data to the command, the context data is the client type, the authentication level, the origin of the command, the target of the command, the size of the data, and the data itself. It is called a message.

there 3 cases: 

* If a command is not recognized as ESP3D command  and origin is not the printer/cnc the message will be transfered to the communication module client with the printer/cnc.

* If the command is recognized as ESP3D command what ever the origin, the message will be transfered to the ESP3D, and the message will change the origin as the target to answer the command.

* If a command is not recognized as ESP3D command and origin is the printer/cnc the message will be dispatched all clients.

The messages are stored in a FIFO which can be accessed from different thread on the ESP32 when it is done by differents functions on ESP8266 sequentially.

Note: Each message is a unique object, it is not possible to send the same message twice, in case of multiple client the message will be duplicated.

Note 2: message are dynamicaly allocated in memory, which means that when dispatch is done or failed the message must be freed to avoid any memory leak.

## Serial

Serial is a full duplex client, it is the default output client for printer/cnc. It use the native Serial library of the ESP32 and ESP8266.
### Output -> ESP3D
The Serial API allow to hook a function to be called when some data is received.
 `Serials[_serialIndex]->onReceive(receiveSerialCb);`

 When called the function read the data char by char until char `\n` or `r` is found or buffer is full, if it is the case the data is packaged as message and dispatched to the FIFO list by the flush function.

 The FIFO list is accessed from different thread/function on the ESP32,  it is done by differents functions sequentially on ESP8266 .

 The handle function is the one that will dispatch the message to the defined target.

### ESP3D -> Output

When dispatched the message is packaged and sent to the targeted output client. It is delivered to client module by the function `dispatch(ESP3DMessage *message)`.

The message data will be delivered to the client module by the function `writeBytes(const uint8_t *buffer, size_t size)` and using the available function `availableForWrite()` to know how many data can be sent will dispatch the data to Serial.

## USB Serial

USB Serial is a full duplex client, it is optional output client for ESP32 S2/S3 only. It use the esp32-usb-serial library.
The Library has 2 intialization functions: `usb_serial_init()` and `usb_serial_create_task()` which handle usb events.
if no need to handle usb events just call `usb_serial_init()` and do not call `usb_serial_create_task()`.
Additionaly USB Serial client has a dedicated task that check if there is any supported USB Serial device connected, it try all drivers one by one until it find one that match the supported ones.
If device is found it willhook a usb event function and a onreceive function to handle data.