# ESP3D[![Code Climate](https://codeclimate.com/github/luc-github/ESP3D/badges/gpa.svg)](https://codeclimate.com/github/luc-github/ESP3D)

Firmware for ESP8266/ESP8285  and ESP32 used with 3D printer using [ESP8266 core version](https://github.com/esp8266/Arduino)  and [ESP32 core version](https://github.com/espressif/arduino-esp32)   

This firmware allows not only to have a cheap bridge between Wifi and serial, but also to have a web UI to configure wifi, to monitor 3D printer and even control it, and to make things easy,
UI is fully customizable without reflashing FW.

Firmware should work with any 3D printer firmware (repetier/marlin/smoothieware using GCODE) if serial connection has a correct setup.
I currently use it with my personnal flavor of [repetier for Due based boards](https://github.com/luc-github/Repetier-Firmware-0.92).

The web interface files are present in data directory but UI has it's own repository [ESP3D-WEBUI](https://github.com/luc-github/ESP3D-WEBUI).
* be aware  ESP3D-WEBUI is for firmware 0.9.99 minimum - previous released version use tpl files which are no more used.

<u>Stable version (ESP8266 only):</u>    
Arduino ide 1.6.5 with stable [2.0.0](http://arduino.esp8266.com/versions/2.0.0/package_esp8266com_index.json) from ESP8266, please use https://github.com/luc-github/ESP3D/releases/tag/v0.5.1    
Arduino ide 1.6.8 with stable [2.2.0](http://arduino.esp8266.com/versions/2.2.0/package_esp8266com_index.json) from ESP8266, please use https://github.com/luc-github/ESP3D/releases/tag/v0.6.2    
Arduino ide 1.8.5 with stable [2.4.0](http://arduino.esp8266.com/versions/2.4.0/package_esp8266com_index.json) from ESP8266, please use https://github.com/luc-github/ESP3D/releases/tag/V0.9.99    

<u>RC version for 1.0(master branch)</u>    
Arduino ide 1.8.4 with git version from ESP8266 or ESP32 : [![Build Status](https://travis-ci.org/luc-github/ESP3D.svg?branch=master)](https://travis-ci.org/luc-github/ESP3D)  
 
<u>[Development version for 2.0 (asyncwebserver branch)](https://github.com/luc-github/ESP3D/tree/asyncwebserver) & [ESP-WEBUI (asyncUI branch)](https://github.com/luc-github/ESP3D-WEBUI/tree/asyncUI):</u>    
Arduino ide 1.8.4 with git version from ESP8266 or ESP32 : [![Build Status](https://travis-ci.org/luc-github/ESP3D.svg?branch=asyncwebserver)](https://travis-ci.org/luc-github/ESP3D)   
for 100% support of ESP32

[All releases](https://github.com/luc-github/ESP3D/wiki)

:+1:Thanks
* to @disneysw for bringing this module idea
* to @lkarlslund for suggestion about independent reset using GPIO2
* to Roy Cortes from http://www.panucatt.com for supporting and pushing me implementing great features
* to all contributors, feedbacks owners and donations.

## Donate
Every support is welcome: [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Y8FFE7NA4LJWQ)    
Especially if need to buy new modules for testing.

## Features
* Serial/Wifi bridge using configurable port 8888, here to enable/disable [TCP_IP_DATA_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings, here to enable/disable [RECOVERY_FEATURE](https://github.com/luc-github/ESP8266/blob/master/esp8266/config.h)   
* Complete configuration by web browser (Station or Access point) or by Serial commands
* Authentication for sensitive pages, here to enable/disable [AUTHENTICATION_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Update firmware by web browser, here to enable/disable [WEB_UPDATE_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Control ESP module using commands on serial or data port, here to enable/disable [SERIAL_COMMAND_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* UI fully constomizable without reflashing FW using html templates, [keywords](https://raw.githubusercontent.com/luc-github/ESP3D/master/docs/keywords.txt) and html files/images
* Captive portal in Access point mode which redirect all unknow call to main page, here to enable/disable [CAPTIVE_PORTAL_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h) 
* mDNS which allows to key the name defined in web browser and connect only with bonjour installed on computer, here to enable/disable [MDNS_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* SSDP, this feature is a discovery protocol, supported on Windows out of the box, here to enable/disable [SSDP_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Printer monitoring / control (temperatures/speed/jog/list SDCard content/launch,pause or stop a print/etc...), here to enable/disable [MONITORING_FEATURE/INFO_MSG_FEATURE/ERROR_MSG_FEATURE/STATUS_MSG_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Fail safe mode (Access point)is enabled if cannot connect to defined station at boot.
* The web ui add even more feature : https://github.com/luc-github/ESP3D-WEBUI/blob/master/README.md#features  


## Web configuration      
*Wifi Mode : Access point / Client station  
*IP Generation: DHCP/Static IP      
*IP/MASK/GATEWAY for static data    
*Baud Rate for serial (supported : 9600, 19200, 38400, 57600, 115200, 230400, 250000)    
*web port and data port      

    
## Default Configuration      
Default Settings:    
AP:ESP8266    
PW:12345678   
Authentification: WPA     
Mode: g (n is not supported by AP, just by STA)    
channel: 11    
AP: visible    
Sleep Mode: Modem    
IP Mode: Static IP    
IP: 192.168.0.1   
Mask: 255.255.255.0   
GW:192.168.0.1    
Baud rate: 115200   
Web port:80   
Data port: 8888     
Web Page refresh: 3 secondes    
User: admin   
Password: admin   
User:user   
Password: user   



## Direct commands:    
Check wiki : https://github.com/luc-github/ESP3D/wiki/Direct-ESP3D-commands

## Installation
1. Please follow installation of the ESP core you want to use : [ESP8266 core version](https://github.com/esp8266/Arduino)  or [ESP32 core version](https://github.com/espressif/arduino-esp32)   
2.  Add missing libraries if you target ESP32 present in libraries directory
* DNSServer (from https://github.com/bbx10/DNSServer_tng)
* WebServer (from https://github.com/bbx10/WebServer_tng)
* NetBIOS and SSDP are currently disabled for ESP32 as not yet supported
3. Compile project (ESP3D.ino) according target: ESP8266 board or ESP32 board, please review config.h to enable disable a feature, by default athenticatio is disabled and all others are enabled.   
* for ESP8266 set CPU freq to 160MHz for better (https://github.com/luc-github/ESP3D/wiki/Install-Instructions)
4. Upload the data content on ESP3D file system
* Using SPIFFS uploader, this plugin and install instructions is available on each ESP core - please refere to it
* Using embedded uploader (you may need to format SPIFFS using : [ESP710]FORMAT on ESP8266 first)    
if embedded uploader does not show up you can force it ti display using : http://your_IP_address?forcefallback=yes    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/docs/embedded.png><br>

## Update
* Generate a binary using the export binary menu from Arduino IDE and upload it using ESP-WEBUI or embedded interface  

<H3>:warning:Do not flash your Printer fw with ESP connected - it bring troubles, at least on DaVinci</H3>

## Contribution/customization
* To style the code before pushing PR please use [astyle --style=otbs *.h *.cpp *.ino](http://astyle.sourceforge.net/)   
* The embedded page is created using nodejs then gulp to generate a compressed html page (tool.html.gz), all necessary modules can be installed using the install.bat file content, then it is included using bin2c (https://sourceforge.net/projects/bin2c/) to generate the  h file used to create the file nofile.h, update the array and size according new out.h.   
* The current UI is located [here](https://github.com/luc-github/ESP3D-WEBUI)
* An optional UI is under development using old repetier UI - check [UI\repetier\testui.htm] (https://github.com/luc-github/ESP3D/blob/master/UI/repetier/testui.htm) file   

Feedback/suggestion/discussions are always welcome   
 
## Need more information about supported boards or wiring ?
[Check the wiki](https://github.com/luc-github/ESP3D/wiki)

## :question:Any question ?   
Check [Wiki](https://github.com/luc-github/ESP3D/wiki/Install-Instructions) or [![Join the chat at https://gitter.im/luc-github/ESP3D](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP3D?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)   

## :exclamation:Any issue/feedback ?    
Check [Wiki](https://github.com/luc-github/ESP3D/wiki/Install-Instructions) and [FAQ](https://github.com/luc-github/ESP3D/issues?utf8=%E2%9C%93&q=label%3AFAQ+) or [submit ticket](https://github.com/luc-github/ESP3D/issues)    

## ESP3D is used by :
* Custom version is used on azteeg mini wifi : http://www.panucatt.com/azteeg_X5_mini_reprap_3d_printer_controller_p/ax5mini.htm
* More to come...

## TODO   
-- Close open topics    
-- Do testing (a lot)    
-- UI Improvement    
-- ESP3D V2
