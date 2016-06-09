# ESP3D
Firmware for ESP8266 used with 3D printer using [arduino core version](https://github.com/esp8266/Arduino)   
This firmware allows not only to have a cheap bridge between Wifi and serial, but also to have a web UI to configure wifi, to monitor 3D printer and even control it, and to make things easy,
UI is fully customizable without reflashing FW.
Firmware should work with any 3D printer firmware (repetier/marlin/etc..) if serial connection has correct setup.
I currently use it with my personnal flavor of [repetier for Due based boards](https://github.com/luc-github/Repetier-Firmware-0.92).
Please use ESP with at least 1M flash, for ESP with 512K there is limited version [here](https://github.com/luc-github/ESP3D/tree/ESP-512K-64KSPIFFS)

<u>Stable version:</u>    
Arduino ide 1.6.5 with stable [2.0.0](http://arduino.esp8266.com/versions/2.0.0/package_esp8266com_index.json) from ESP8266, please use https://github.com/luc-github/ESP3D/releases/tag/v0.5.1    
Arduino ide 1.6.8 with stable [2.2.0](http://arduino.esp8266.com/versions/2.2.0/package_esp8266com_index.json) from ESP8266, please use https://github.com/luc-github/ESP3D/releases/tag/v0.6.2    

<u>Development version:</u>    
Arduino ide 1.6.9 with git from ESP8266 : [![Build Status](https://travis-ci.org/luc-github/ESP3D.svg?branch=master)](https://travis-ci.org/luc-github/ESP3D)    

[All releases](https://github.com/luc-github/ESP3D/wiki)

:question:Any question ?[![Join the chat at https://gitter.im/luc-github/ESP3D](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP3D?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)   
:exclamation:Any issue ? check [FAQ](https://github.com/luc-github/ESP3D/issues?utf8=%E2%9C%93&q=label%3AFAQ+) or [submit ticket](https://github.com/luc-github/ESP3D/issues)    


:+1:Thanks
* to @disneysw for bringing this module idea
* to @lkarlslund for suggestion about independant reset using GPIO2
* to all contributors (treepleks, j0hnlittle, openhardwarecoza, TRoager, all feedbacks owners and donations)

Every support is welcome: [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Y8FFE7NA4LJWQ)    
Especially if need to buy new modules for testing.

##Features
* Serial/Wifi bridge using configurable port 8888, here to enable/disable [TCP_IP_DATA_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings, here to enable/disable [RECOVERY_FEATURE](https://github.com/luc-github/ESP8266/blob/master/esp8266/config.h)   
* Wifi configuration by web browser (Station or Access point)
* Authentication for sensitive pages, here to enable/disable [AUTHENTICATION_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Update firmware by web browser, here to enable/disable [WEB_UPDATE_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Control ESP module using commands on serial or data port, here to enable/disable [SERIAL_COMMAND_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* UI fully constomizable without reflashing FW using html templates, [keywords](https://raw.githubusercontent.com/luc-github/ESP3D/master/docs/keywords.txt) and html files/images
* Captive portal in Access point mode which redirect all unknow call to main page, here to enable/disable [CAPTIVE_PORTAL_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h) 
* mDNS which allows to key the name defined in web browser and connect only with bonjour installed on computer, here to enable/disable [MDNS_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* SSDP, this feature is a discovery protocol, supported on Windows out of the box, here to enable/disable [SSDP_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Printer monitoring / control (temperatures/speed/jog/list SDCard content/launch,pause or stop a print/etc...), here to enable/disable [MONITORING_FEATURE/INFO_MSG_FEATURE/ERROR_MSG_FEATURE/STATUS_MSG_FEATURE](https://github.com/luc-github/ESP3D/blob/master/esp3d/config.h)
* Fail safe mode (Access point)is enabled if cannot connect to defined station at boot.

##Web configuration      
*Wifi Mode : Access point / Client station  
*IP Generation: DHCP/Static IP      
*IP/MASK/GATEWAY for static data    
*Baud Rate for serial (supported : 9600, 19200, 38400, 57600, 115200, 230400, 250000)    
*web port and data port      

    
##Default Configuration      
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
Baud rate: 9600   
Web port:80   
Data port: 8888     
Web Page refresh: 3 secondes    
User: admin     
Password: admin

These are the pages defined using template:    
Home page :     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page1.png><br>
System Configuration Page:     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page2.png><br>     
Access Point Configuration Page:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page3.png><br>     
Client Configuration Page:     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page4.png><br>     
Printer Status Page for 64K SPIFFS, due to limited space available no fancy:     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page5-2.png><br>    
Printer Status Page for more than 64K SPIFFS, fancy one:     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/page5.png><br>     
Extra Settings Page, for web UI and for printer:     
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page6.png><br>     
Change password Page:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page7.png><br>     
Login Page:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/Page8.png><br>     
the template files are stored on SPIFFS:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/UI/files.png><br>
and uploaded using [pluggin IDE](http://esp8266.github.io/Arduino/versions/2.1.0/doc/filesystem.html#uploading-files-to-file-system)    
Any files on SPIFFS can be called on web interface without having the path hard coded, this give more flexibility, favicon.ico is a good example of it.         
So UI is kind of separated from FW which allow easier modifications. For this a light file manager is available in extra settings page, it allows to upload/download/delete files. 
Because SPIFFS is flat filesystem, no directory management is necessary, so it is very simple.

Additionally 404.tpl (the page not found) and restart.tpl(restart page when applying changes) are not mandatory, a fail safe version is embeded if they are not present.     

##Direct commands:    

    -restart module from host/printer: [ESP888]RESTART      
    -Get IP (only printer see answer): [ESP111]M117     
    -reset EEPROM and restart: [ESP444]RESET    
    -display EEPROM content: [ESP444]CONFIG    
    -go to safe mode without restart: [ESP444]SAFEMODE    
    -SSID: [ESP100]<SSID>    
    -Password: [ESP101]<Password>   
    -Station mode: [ESP103]STA   
    -AP mode: [ESP103]AP   
    -IP Static: [ESP104]STATIC    
    -IP DHCP: [ESP104]DHCP    
 
##Installation
* For stable:
Please use [Arduino IDE 1.6.5](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager use 2.0.0 stable version by adding in your preferences http://arduino.esp8266.com/version/2.0.0/package_esp8266com_index.json
with https://github.com/luc-github/ESP3D/releases/tag/v0.5.1
or
use [Arduino IDE 1.6.8](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager use 2.2.0 stable version by adding in your preferences http://arduino.esp8266.com/stable/package_esp8266com_index.json
with https://github.com/luc-github/ESP3D/releases/tag/v0.6.2   

* For development:
Please use [Arduino IDE 1.6.8](http://arduino.cc/en/Main/Software) and [git version of esp8266 module](http://esp8266.github.io/Arduino/versions/2.2.0/doc/installing.html#using-git-version)

* To flash the module :   
For better performance select CPU Frequency to be 160MHz instead of default 80MHz   
Use IDE to upload directly  (latest version of board manager module generate one binary)     
* To flash the html files present in data directory you need to use another tool, installation and usage is explained [here](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system)    
Once flashed you also can use the web updater to flash new FW in System Configuration Page or go to settings to change html files 

<H3>:warning:Do not flash Printer fw with ESP connected - it bring troubles, at least on DaVinci</H3>

##Hardware connection

* Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
* Use GPIO0 to ground to be in update mode   

For ESP01:   
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/HW/Wires.png><br>

For ESP12E:   
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/HW/WiresESP12E.png><br>

For Davinci Board:   
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/Davinci/davinci.png><br>

For RADDS Board:   
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/RADDS/RADDS.png><br>

##Contribution/customization
To modifying and Testing tpl files a local tool has been created by [j0hnlittle](https://github.com/j0hnlittle) to avoid to upload everytime your tpl files just to see the results of your modifications. It is a python script (2.7+) located in tools directory, launch it: python server.py, then open browser: http://localhost:8080   
It will display the web ui and allow some navigation   

To style the code before pushing PR please use [astyle --style=otbs *.h *.cpp *.ino](http://astyle.sourceforge.net/)

Feedback/suggestion/discussions are always welcome
 

##Result of ESP12E on Davinci    
I use a proto board to connect ESP12E socket, one micro switch for recovery, one jumper for normal usage/ flash, I did not put hardware switch.    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/Davinci/board.jpg><br>  
Connected to Davinci:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/Davinci/boardconnected.jpg><br>  
The back cover:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/Davinci/backside.jpg><br>  
The screen when connected to AP:    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/Davinci/screen.jpg><br>   
 
##Result of ESP12E on Due/RADDS 
 the rendering on screen when connection to AP is done:   
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/master/images/RADDS/screen.jpg><br> 


##TODO   
-- Close open topics    
-- Do testing (a lot)    
-- UI Improvement    
-- Printer EEPROM management
