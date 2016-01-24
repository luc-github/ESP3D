# ESP8266

[![Join the chat at https://gitter.im/luc-github/ESP8266](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP8266?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
FW for ESP8266 used with 3D printer     

Arduino ide 1.6.5 with latest master from ESP8266 : [![Build Status](https://travis-ci.org/luc-github/ESP8266.svg?branch=master)](https://travis-ci.org/luc-github/ESP8266)    

##Description      
Thanks to @luc-github for bringing this idea to life on Repetier.

Have a bridge configurable by web and optionally by printer (not yet implemented)  
Have a front end to know what is the wifi status or know what is the print status (not yet implemented) - this part can be optional and removed by compilation directive if no need

Only aim is to make this compatible to with Smoothieware, as far as possible.
Many things in the Repetier version do not work yet with Smoothieware (marked as non-smooth below).

For master to work, you have to use the 2.0.0 verion of https://github.com/esp8266/arduino (https://github.com/esp8266/Arduino#using-git-version-) and work under Windows (the 64 bits Linux version does not compile the firmware correctly).

If you use an ESP with 512K flash like ESP01 please go here : https://github.com/treepleks/ESP8266/tree/ESP-512K-64KSPIFFS, it is dedicated to low memory device.      
If you use an ESP with more than 512K flash, please use master.      

##Hardware connection       
--Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
--Use GPIO0 to ground to be in update mode   
--Use a switch to reset/disable module    
For ESP01:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Wires.png><br>   

For ESP12E:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/WiresESP12E.png><br>
<br>

##Development   
Currently using [Arduino IDE 1.6.5](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager added from [github.com/esp8266/Arduino](https://github.com/esp8266/Arduino) using the 2.0.0 released version (http://arduino.esp8266.com/package_esp8266com_index.json).
  
Additionnaly:
--Use minimal css from http://getbootstrap.com/examples/theme/  
--Reuses the Smoothieware SVG JogRose and JogBars (edited to use CSS animations).

##Flash the Module    
*Tools:      
--Use IDE to upload directly  (latest version of board manager module generate one binary)     
-- to flash the html files present in data directory you need to use another tool. Installation and usage is explained here:  http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html#file-system   
Once flashed you also can use the web updater to flash new FW in System Configuration Page

*Connection
--Connect GPIO0 to ground to be in update mode

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
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page1.png><br>
System Configuration Page:     
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page2.png><br>     
Access Point Configuration Page:    
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page3.png><br>     
Client Configuration Page:     
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page4.png><br>     
Printer Status Page for 64K SPIFFS, due to limited space available no fancy:     
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page5-2.png><br>    
Printer Status Page for more than 64K SPIFFS, fancy one:     
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/page5.png><br>     
Extra Settings Page, for web UI and for printer:     
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page6.png><br>     
Change password Page:    
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page7.png><br>     
Login Page:    
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/Page8.png><br>     
the template files are stored on SPIFFS:    
<img src=https://raw.githubusercontent.com/treepleks/ESP8266/master/files.png><br>
and uploaded using [IDE](http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html#file-system)    
The list of keywords can be find here : https://github.com/treepleks/ESP8266/blob/master/keywords.txt     
Any files on SPIFFS can be called on web interface without having the path hard coded  - this give more flexibility,  favicon.ico is a good example of it.
So UI is kind of separated from FW which allow easier modifications. For this a light file manager is available in the  "Extra Settings" page. It allows to upload/download/delete files.

The 404.tpl (page not found) and restart.tpl (restart when applying changes) pages are not mandatory, a fail safe version is embedded.     

Currently, tested on a new generation (black) 1MB ESP01 using 128K SPIFFS that can be directly soldered on the LCD panel of the AZSMZ Smoothieboard clone.

##Protocol for discovery   
*mDNS : on Station mode only with bonjour installed on computer.
*SSDP : on Station and AP mode.
*Captive portal : on AP mode only.

##Basic Authentification   
Can be disabled  in FW
default user: admin
default password: admin 

#OTA support
Currently only web update is supported not telnet one

##Commands/msg from/to serial(not fully implemented):    
*from module to printer by serial communication   
    -M117 [Message], Error/status message from module (done)     
    -Send Wifi settings [AP/STATION,SSID,DHC/STATIC,IP,MASK,GW,STATUS,MAC ADDRESSS, BAUD?], Module configuration without password    
        
*from host to printer on port 8888  (implemented) 
    - bridge from TCP/IP to Serial and vice-versa (done)   
          
*from printer/host to module  (not fully implemented)  
    -request configuration/status      
    -set AP/STATION,SSID,PASSSWORD,DHC/STATIC,IP,MASK,GW,BAUD from serial 
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
 
##Front End (implemented)
--Display printer status
--Display temperatures
--Display positions/flow/speed (non-smooth)
--Display print progress if any (non-smooth)
--List SDCard Content (non-smooth)
--Launch a Print (non-smooth)
--Stop/Pause a Print
--Emergency Stop
--Jog control / custom commands
 
##TODO   
-- Close open topics
-- Do testing (a lot)
-- UI Improvement
-- Printer config management (not done)

##Donation:
You can support the author of the original Repetier-targeted firmware: [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Y8FFE7NA4LJWQ).
