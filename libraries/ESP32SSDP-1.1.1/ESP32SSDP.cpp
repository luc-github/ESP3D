/*
ESP32 Simple Service Discovery
Copyright (c) 2015 Hristo Gochkov

Original (Arduino) version by Filippo Sallemi, July 23, 2014.
Can be found at: https://github.com/nomadnt/uSSDP

License (MIT license):
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*/
#ifdef ARDUINO_ARCH_ESP32
#include <functional>
#include "ESP32SSDP.h"
#include "WiFiUdp.h"
#include <lwip/ip_addr.h>

//#define DEBUG_SSDP  Serial

#define SSDP_INTERVAL     1200
#define SSDP_PORT         1900
#define SSDP_METHOD_SIZE  10
#define SSDP_URI_SIZE     2
#define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 2
static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);
#define SSDP_UUID_ROOT "38323636-4558-4dda-9188-cda0e6"


static const char _ssdp_response_template[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "EXT:\r\n";

static const char _ssdp_notify_template[] PROGMEM =
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "NTS: ssdp:alive\r\n";

static const char _ssdp_packet_template[] PROGMEM =
    "%s" // _ssdp_response_template / _ssdp_notify_template
    "CACHE-CONTROL: max-age=%u\r\n" // _interval
    "SERVER: %s UPNP/1.1 %s/%s\r\n" // _servername, _modelName, _modelNumber
    "USN: uuid:%s%s\r\n" // _uuid, _usn_suffix
    "%s: %s\r\n"  // "NT" or "ST", _deviceType
    "LOCATION: http://%u.%u.%u.%u:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
    "\r\n";

static const char _ssdp_schema_template[] PROGMEM =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/xml\r\n"
    "Connection: close\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n"
    "<?xml version=\"1.0\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
    "<major>1</major>"
    "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
    "<device>"
    "<deviceType>%s</deviceType>"
    "<friendlyName>%s</friendlyName>"
    "<presentationURL>%s</presentationURL>"
    "<serialNumber>%s</serialNumber>"
    "<modelName>%s</modelName>"
    "<modelDescription>%s</modelDescription>"
    "<modelNumber>%s</modelNumber>"
    "<modelURL>%s</modelURL>"
    "<manufacturer>%s</manufacturer>"
    "<manufacturerURL>%s</manufacturerURL>"
    "<UDN>uuid:%s</UDN>"
    "<serviceList>%s</serviceList>"
    "<iconList>%s</iconList>"
    "</device>"
    "</root>\r\n"
    "\r\n";

struct SSDPTimer {
    ETSTimer timer;
};

SSDPClass::SSDPClass() :
    _replySlots{NULL},
    _respondToAddr{0,0,0,0}
{
    _server = nullptr;
    _timer = nullptr;
    _port = 80;
    _ttl = SSDP_MULTICAST_TTL;
    _interval = SSDP_INTERVAL;
    _respondToPort = 0;
    _pending = false;
    _stmatch = false;
    _delay=0;
    _process_time = 0;
    _notify_time = 0;
    _uuid[0] = '\0';
    _usn_suffix[0] = '\0';
    _respondType[0] = '\0';
    _modelNumber[0] = '\0';
    sprintf(_deviceType, "urn:schemas-upnp-org:device:Basic:1");
    _friendlyName[0] = '\0';
    _presentationURL[0] = '\0';
    _serialNumber[0] = '\0';
    _modelName[0] = '\0';
    _modelURL[0] = '\0';
    _manufacturer[0] = '\0';
    _manufacturerURL[0] = '\0';
    _servername = "Arduino/1.0";
    sprintf(_schemaURL, "ssdp/schema.xml");
    _schema = nullptr;
}

SSDPClass::~SSDPClass()
{
    end();
}

void SSDPClass::end()
{
    if (_schema) {
        free(_schema);
        _schema = nullptr;
    }
    if(!_server) {
        return;
    }
#ifdef DEBUG_SSDP
    DEBUG_SSDP.printf_P(PSTR("SSDP end ... "));
#endif
    // undo all initializations done in begin(), in reverse order
    _stopTimer();
    _server->stop();
    delete (_server);
    _server = 0;
}

IPAddress SSDPClass::localIP()
{
    tcpip_adapter_ip_info_t ip;
    if (WiFi.getMode() == WIFI_STA) {
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip)) {
            return IPAddress();
        }
    } else if (WiFi.getMode() == WIFI_OFF) {
        if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip)) {
            return IPAddress();
        }
    }
    return IPAddress(ip.ip.addr);
}

void SSDPClass::setUUID(const char *uuid, bool rootonly)
{
    //no sanity check is done - TBD
    if (rootonly) {
        uint32_t chipId = ((uint16_t) (ESP.getEfuseMac() >> 32));
        sprintf(_uuid, "%s%02x%02x%02x",
                uuid,
                (uint16_t) ((chipId >> 16) & 0xff),
                (uint16_t) ((chipId >>  8) & 0xff),
                (uint16_t)   chipId        & 0xff  );
    } else {
        strlcpy(_uuid, uuid,sizeof(_uuid));
    }
}

bool SSDPClass::begin()
{
    _pending = false;
    _stmatch = false;
    end();
    if (strlen(_uuid) == 0) {
        setUUID(SSDP_UUID_ROOT);
    }
#ifdef DEBUG_SSDP
    DEBUG_SSDP.printf("SSDP UUID: %s\n", (char *)_uuid);
#endif
    assert(nullptr == _server);
    _server = new WiFiUDP;
    if (!(_server->beginMulticast(IPAddress(SSDP_MULTICAST_ADDR), SSDP_PORT))) {
#ifdef DEBUG_SSDP
        DEBUG_SSDP.println("Error begin");
#endif
        return false;
    }

    _startTimer();

    return true;
}

void SSDPClass::_send(ssdp_method_t method)
{
    char buffer[1460];
    IPAddress ip = localIP();

    char valueBuffer[strlen_P(_ssdp_notify_template)+1];
    strcpy_P(valueBuffer, (method == NONE)?_ssdp_response_template:_ssdp_notify_template);

    int len = snprintf_P(buffer, sizeof(buffer),
                         _ssdp_packet_template,
                         valueBuffer,
                         _interval,
                         _servername.c_str(),
                         _modelName, _modelNumber,
                         _uuid, _usn_suffix,
                         (method == NONE)?"ST":"NT",
                         _respondType,
                         ip[0], ip[1], ip[2], ip[3], _port, _schemaURL
                        );
    if(len < 0) {
        return;
    }
    IPAddress remoteAddr;
    uint16_t remotePort;
    if(method == NONE) {
        remoteAddr = _respondToAddr;
        remotePort = _respondToPort;
#ifdef DEBUG_SSDP
        DEBUG_SSDP.print("Sending Response to ");
#endif
    } else {
        remoteAddr = IPAddress(SSDP_MULTICAST_ADDR);
        remotePort = SSDP_PORT;
#ifdef DEBUG_SSDP
        DEBUG_SSDP.print("Sending Notify to ");
#endif
    }
#ifdef DEBUG_SSDP
    DEBUG_SSDP.print(remoteAddr);
    DEBUG_SSDP.print(":");
    DEBUG_SSDP.println(remotePort);
#endif
    _server->beginPacket(remoteAddr, remotePort);
    _server->println(buffer);
#ifdef DEBUG_SSDP
    DEBUG_SSDP.println("*************************TX*************************");
    DEBUG_SSDP.println(buffer);
    DEBUG_SSDP.println("****************************************************");
#endif
    _server->endPacket();
}

const char * SSDPClass::schema()
{
    uint len = strlen(_ssdp_schema_template)
               + 21 //(IP = 15) + 1 (:) + 5 (port)
               + SSDP_DEVICE_TYPE_SIZE
               + SSDP_FRIENDLY_NAME_SIZE
               + SSDP_SCHEMA_URL_SIZE
               + SSDP_SERIAL_NUMBER_SIZE
               + SSDP_MODEL_NAME_SIZE
               + _modelDescription.length()
               + SSDP_MODEL_VERSION_SIZE
               + SSDP_MODEL_URL_SIZE
               + SSDP_MANUFACTURER_SIZE
               + SSDP_MANUFACTURER_URL_SIZE
               + SSDP_UUID_SIZE
               + _services.length()
               + _icons.length();
    if (_schema) {
        free (_schema);
        _schema = nullptr;
    }
    _schema = (char *)malloc(len+1);
    if (_schema) {
        IPAddress ip = localIP();
        sprintf(_schema, _ssdp_schema_template,
                ip[0], ip[1], ip[2], ip[3], _port,
                _deviceType,
                _friendlyName,
                _presentationURL,
                _serialNumber,
                _modelName,
                _modelDescription.c_str(),
                _modelNumber,
                _modelURL,
                _manufacturer,
                _manufacturerURL,
                _uuid,
                _services.c_str(),
                _icons.c_str()
               );
    } else {
#ifdef DEBUG_SSDP
        DEBUG_SSDP.println("not enough memory for schema");
#endif
    }
    return _schema;
}

void SSDPClass::schema(WiFiClient client)
{
    client.print(schema());
}

void SSDPClass::_update()
{
    int nbBytes  =0;
    char * packetBuffer = nullptr;

    if(!_pending && _server) {
        ssdp_method_t method = NONE;
        nbBytes= _server->parsePacket();
        typedef enum {METHOD, URI, PROTO, KEY, VALUE, ABORT} states;
        states state = METHOD;
        typedef enum {STRIP, START, SKIP, MAN, ST, MX} headers;
        headers header = STRIP;

        uint8_t cursor = 0;
        uint8_t cr = 0;

        char buffer[SSDP_BUFFER_SIZE] = {0};
        packetBuffer = new char[nbBytes +1];
        int message_size=_server->read(packetBuffer,nbBytes);
        int process_pos = 0;
        packetBuffer[message_size]='\0';
        _respondToAddr = _server->remoteIP();
        _respondToPort = _server->remotePort();
#ifdef DEBUG_SSDP
        if (message_size) {
            DEBUG_SSDP.println("*************************RX*************************");
            DEBUG_SSDP.print(_server->remoteIP());
            DEBUG_SSDP.print(":");
            DEBUG_SSDP.println(_server->remotePort());
            DEBUG_SSDP.println(packetBuffer);
            DEBUG_SSDP.println("****************************************************");
        }
#endif
        while(process_pos < message_size) {

            char c = packetBuffer[process_pos];
            process_pos++;
            (c == '\r' || c == '\n') ? cr++ : cr = 0;
            switch(state) {
            case METHOD:
                if(c == ' ') {
                    if(strcmp(buffer, "M-SEARCH") == 0) {
                        method = SEARCH;
                    }

                    if(method == NONE) {
                        state = ABORT;
                    } else {
                        state = URI;
                    }
                    cursor = 0;

                } else if(cursor < SSDP_METHOD_SIZE - 1) {
                    buffer[cursor++] = c;
                    buffer[cursor] = '\0';
                }
                break;
            case URI:
                if(c == ' ') {
                    if(strcmp(buffer, "*")) {
                        state = ABORT;
                    } else {
                        state = PROTO;
                    }
                    cursor = 0;
                } else if(cursor < SSDP_URI_SIZE - 1) {
                    buffer[cursor++] = c;
                    buffer[cursor] = '\0';
                }
                break;
            case PROTO:
                if(cr == 2) {
                    state = KEY;
                    cursor = 0;
                }
                break;
            case KEY:
                // end of HTTP request parsing. If we find a match start reply delay.
                if(cr == 4) {
                    if (_stmatch) {
                        _pending = true;
                        _process_time = millis();
                    }
                } else if(c == ':') {
                    cursor = 0;
                    state = VALUE;
                } else if(c != '\r' && c != '\n' && c != ' ' && cursor < SSDP_BUFFER_SIZE - 1) {
                    buffer[cursor++] = c;
                    buffer[cursor] = '\0';
                }
                break;
            case VALUE:
                if(cr == 2) {
                    switch(header) {
                    case START:
                    case STRIP:
                    case SKIP:
                        break;
                    case MAN:
#ifdef DEBUG_SSDP
                        DEBUG_SSDP.printf("MAN: %s\n", (char *)buffer);
#endif
                        break;
                    case ST:
                        // save the search term for the reply and clear usn suffix.
                        strlcpy(_respondType, buffer, sizeof(_respondType));
                        _usn_suffix[0] = '\0';
#ifdef DEBUG_SSDP
                        DEBUG_SSDP.printf("ST: '%s'\n",buffer);
#endif
                        // if looking for all or root reply with upnp:rootdevice
                        if(strcmp(buffer, "ssdp:all")==0 || strcmp(buffer, "upnp:rootdevice")==0) {
                            _stmatch = true;
                            // set USN suffix
                            strlcpy(_usn_suffix, "::upnp:rootdevice", sizeof(_usn_suffix));
#ifdef DEBUG_SSDP
                            DEBUG_SSDP.println("the search type matches all and root");
#endif
                            state = KEY;
                        } else
                            // if the search type matches our type, we should respond instead of ABORT
                            if(strcasecmp(buffer, _deviceType) == 0) {
                                _stmatch = true;
                                // set USN suffix to the device type
                                strlcpy(_usn_suffix, "::", sizeof(_usn_suffix));
                                strlcat(_usn_suffix, _deviceType, sizeof(_usn_suffix));
#ifdef DEBUG_SSDP
                                DEBUG_SSDP.println("the search type matches our type");
#endif
                                state = KEY;
                            } else {
                                state = ABORT;
#ifdef DEBUG_SSDP
                                DEBUG_SSDP.println("REJECT. The search type does not match our type");
#endif
                            }
                        break;
                    case MX:
                        // delay in ms from 0 to MX*1000 where MX is in seconds with limits.
                        _delay = (short)random(0, atoi(buffer) * 1000L);
                        if (_delay > SSDP_MAX_DELAY) {
                            _delay = SSDP_MAX_DELAY;
                        }
                        break;
                    }

                    if(state != ABORT) {
                        state = KEY;
                        header = STRIP;
                        cursor = 0;
                    }
                } else if(c != '\r' && c != '\n') {
                    if(header == STRIP) {
                        if(c == ' ') {
                            break;
                        } else {
                            header = START;
                        }
                    }
                    if(header == START) {
                        if(strncmp(buffer, "MA", 2) == 0) {
                            header = MAN;
                        } else if(strcmp(buffer, "ST") == 0) {
                            header = ST;
                        } else if(strcmp(buffer, "MX") == 0) {
                            header = MX;
                        } else {
                            header = SKIP;
                        }
                    }

                    if(cursor < SSDP_BUFFER_SIZE - 1) {
                        buffer[cursor++] = c;
                        buffer[cursor] = '\0';
                    }
                }
                break;
            case ABORT:
                _pending = false;
                _delay = 0;
                break;
            }
        }
    }
    if(packetBuffer) {
        delete packetBuffer;
    }
    // save reply in reply queue if one is pending
    if(_pending) {
        int i;
        // Many UPNP hosts send out mulitple M-SEARCH packets at the same time to mitigate
        // packet loss. Just reply to one for a given host:port.
        for (i = 0; i < SSDP_MAX_REPLY_SLOTS; i++) {
            if (_replySlots[i]) {
                if (_replySlots[i]->_respondToPort == _respondToPort &&
                        _replySlots[i]->_respondToAddr == _respondToAddr
                   ) {
                    // keep original delay
                    _delay = _replySlots[i]->_delay;
                    _process_time = _replySlots[i]->_process_time;
#ifdef DEBUG_SSDP
                    DEBUG_SSDP.printf("Remove dupe SSDP reply in slot %i.\n", i);
#endif
                    delete _replySlots[i];
                    _replySlots[i] = 0;
                }
            }
        }
        // save packet to available reply queue slot
        for (i = 0; i < SSDP_MAX_REPLY_SLOTS; i++) {
            if (!_replySlots[i]) {
#ifdef DEBUG_SSDP
                DEBUG_SSDP.printf("Saving deferred SSDP reply to queue slot %i.\n", i);
#endif
                _replySlots[i] = new ssdp_reply_slot_item_t;
                _replySlots[i]->_process_time = _process_time;
                _replySlots[i]->_delay = _delay;
                _replySlots[i]->_respondToAddr = _respondToAddr;
                _replySlots[i]->_respondToPort = _respondToPort;
                strlcpy(_replySlots[i]->_respondType, _respondType, sizeof(_replySlots[i]->_respondType));
                strlcpy(_replySlots[i]->_usn_suffix, _usn_suffix, sizeof(_replySlots[i]->_usn_suffix));
                break;
            }
        }
#ifdef DEBUG_SSDP
        if (i == SSDP_MAX_REPLY_SLOTS) {
            DEBUG_SSDP.println("SSDP reply queue is full dropping packet.");
        }
#endif
        _pending = false;
        _delay = 0;
    }
    // send any packets that are pending and overdue.
    unsigned long t = millis();
    bool sent = false;
    for (int i = 0; i < SSDP_MAX_REPLY_SLOTS; i++) {
        if (_replySlots[i]) {
            // millis delay with overflow protection.
            if (t - _replySlots[i]->_process_time > _replySlots[i]->_delay) {
                // reply ready. restore and send.
                _respondToAddr = _replySlots[i]->_respondToAddr;
                _respondToPort = _replySlots[i]->_respondToPort;
                strlcpy(_respondType, _replySlots[i]->_respondType, sizeof(_respondType));
                strlcpy(_usn_suffix, _replySlots[i]->_usn_suffix, sizeof(_usn_suffix));
#ifdef DEBUG_SSDP
                DEBUG_SSDP.println("Send None");
#endif
                _send(NONE);
                sent = true;
                delete _replySlots[i];
                _replySlots[i] = 0;
            }
        }
    }
#ifdef DEBUG_SSDP
    uint8_t rcount = 0;
    DEBUG_SSDP.print("SSDP reply queue status: [");
    for (int i = 0; i < SSDP_MAX_REPLY_SLOTS; i++) {
        DEBUG_SSDP.print(_replySlots[i] ? "X" : "-" );
    }
    DEBUG_SSDP.println("]");
#endif
    if(_notify_time == 0 || (millis() - _notify_time) > (_interval * 1000L)) {
        _notify_time = millis();
        // send notify with our root device type
        strlcpy(_respondType, "upnp:rootdevice", sizeof(_respondType));
        strlcpy(_usn_suffix, "::upnp:rootdevice", sizeof(_usn_suffix));
#ifdef DEBUG_SSDP
        DEBUG_SSDP.println("Send Notify");
#endif
        _send(NOTIFY);
        sent = true;
    }
    if (!sent) {
#ifdef DEBUG_SSDP
        DEBUG_SSDP.println("Do not sent");
#endif
    } else {
        _server->flush();
    }
}

void SSDPClass::setSchemaURL(const char *url)
{
    strlcpy(_schemaURL, url, sizeof(_schemaURL));
}

void SSDPClass::setHTTPPort(uint16_t port)
{
    _port = port;
}

void SSDPClass::setDeviceType(const char *deviceType)
{
    strlcpy(_deviceType, deviceType, sizeof(_deviceType));
}

void SSDPClass::setName(const char *name)
{
    strlcpy(_friendlyName, name, sizeof(_friendlyName));
}

void SSDPClass::setURL(const char *url)
{
    strlcpy(_presentationURL, url, sizeof(_presentationURL));
}

void SSDPClass::setSerialNumber(const char *serialNumber)
{
    strlcpy(_serialNumber, serialNumber, sizeof(_serialNumber));
}

void SSDPClass::setSerialNumber(const uint32_t serialNumber)
{
    snprintf(_serialNumber, sizeof(uint32_t)*2+1, "%08X", serialNumber);
}

void SSDPClass::setModelName(const char *name)
{
    strlcpy(_modelName, name, sizeof(_modelName));
}

void SSDPClass::setModelDescription(const char *desc)
{
    _modelDescription = desc;
}
void SSDPClass::setServerName(const char *name)
{
    _servername = name;
}

void SSDPClass::setModelNumber(const char *num)
{
    strlcpy(_modelNumber, num, sizeof(_modelNumber));
}

void SSDPClass::setModelURL(const char *url)
{
    strlcpy(_modelURL, url, sizeof(_modelURL));
}

void SSDPClass::setManufacturer(const char *name)
{
    strlcpy(_manufacturer, name, sizeof(_manufacturer));
}

void SSDPClass::setManufacturerURL(const char *url)
{
    strlcpy(_manufacturerURL, url, sizeof(_manufacturerURL));
}

void SSDPClass::setTTL(const uint8_t ttl)
{
    _ttl = ttl;
}

void SSDPClass::setInterval(uint32_t interval)
{
    _interval = interval;
}

void SSDPClass::_onTimerStatic(SSDPClass* self)
{
#ifdef DEBUG_SSDP
    DEBUG_SSDP.println("Update");
#endif
    self->_update();
}

void SSDPClass::_startTimer()
{
    _stopTimer();
    _timer= new SSDPTimer();
    ETSTimer* tm = &(_timer->timer);
    const int interval = 1000;
    ets_timer_disarm(tm);
    ets_timer_setfn(tm, reinterpret_cast<ETSTimerFunc*>(&SSDPClass::_onTimerStatic), reinterpret_cast<void*>(this));
    ets_timer_arm(tm, interval, 1 /* repeat */);
}

void SSDPClass::_stopTimer()
{
    if(!_timer) {
        return;
    }
    ETSTimer* tm = &(_timer->timer);
    ets_timer_disarm(tm);
    delete _timer;
    _timer = nullptr;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SSDP)
SSDPClass SSDP;
#endif

#endif
