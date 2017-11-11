# ESP3D - Async WebServer Version - This is pre-alpha and not ready for production    
It use the great https://github.com/me-no-dev/ESPAsyncWebServer   

Build status : [![Build Status](https://travis-ci.org/luc-github/ESP3D.svg?branch=master)](https://travis-ci.org/luc-github/ESP3D)    


This is the base of ESP3D V2 - several change willl come so no backward compatibility is possible with 1.0
It support [ESP8266](https://github.com/esp8266/Arduino) and [ESP32](https://github.com/espressif/arduino-esp32). MCU   

Be aware ESP32 core is still missing several libraries (NetBios, SSDP).    
   
Mandatories libraries are present in libraries directory    
* for ESP8266:   
    - ESPAsyncWebServer (from https://github.com/me-no-dev/ESPAsyncWebServer )   
    - ESPAsyncTCP (from https://github.com/me-no-dev/ESPAsyncTCP)   
    - use lwIP version Prebuild Source GCC as some libraries are not yet compatible lwIP V2   
    
* for ESP32:    
    - ESPAsyncWebServer  (from https://github.com/me-no-dev/ESPAsyncWebServer )   
    - AsyncTCP (from https://github.com/me-no-dev/AsyncTCP)   
    - DNSServer (from https://github.com/bbx10/DNSServer_tng)   
    
#What UI to use ?
* index.html.gz is present in data directory based on https://github.com/luc-github/ESP3D-WEBUI
* new repo will be setup to support new features for 2.0
    
#What is currently working?    
* UI (index.html.gz) / printer control / SPIFFS / Web Update
* Embedded uploader / updater   
* TCP/IP Serial bridge
* Serial commands (docs/Commands.txt)
* Captive portal (not tested yet)   

#What is not yet working?   
* Authentication is not active yet as it will be rewriten using events   
* TCP/IP Debuging is disabled as only Sync currently     

#What is planed to be added?     
* Oled screen support   
* Nextion screen support   
* WebSockets    
* Full Serial printer output in live    


Every support is welcome: [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Y8FFE7NA4LJWQ)    
Especially if need to buy new modules for testing.   
