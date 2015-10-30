# ESP8266

[![Join the chat at https://gitter.im/luc-github/ESP8266](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP8266?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
FW for ESP8266 used with 3D printer

##Description      
Thanks to @disneysw for bringing this module idea    
Thanks to @lkarlslund for suggestion about independant reset using GPIO2   

Have a bridge configurable by web (implemented) and optionally by printer (not yet implemented)  
Have a front end to know what is the wifi status (implemented) or know what is the print status (not yet implemented) - this part can be optional and removed by compilation directive if no need    

Should be compatible with reprap printer (Marlin FW/Repetier FW)  as soon as you can make both serial to communicate. 

current release is v0.4-beta.1: https://github.com/luc-github/ESP8266/releases/tag/v0.4-beta.1, need to use at least staging version 1.6.5-1160-gef26c5f

master may not work unless you use the latest code of https://github.com/esp8266/arduino due to delay between PR integration in staging/stable versions

If you use an ESP with 512K flash like ESP01 please go here : https://github.com/luc-github/ESP8266/tree/ESP-512K-64KSPIFFS, it is dedicated to low memory device.      
If you use an ESP with more than 512K flsah please use master.      

##Hardware connection       
--Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
--Use GPIO0 to ground to be in update mode   
--Use a switch to reset/disable module    
For ESP01:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Wires.png><br>   

For ESP12E:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/WiresESP12E.png><br>
<br>
For Davinci Board:<BR>
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/davinci.png><br> 

##Development   
Currently using [Arduino IDE 1.6.5](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager added from [github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)    
using relased version (http://arduino.esp8266.com/package_esp8266com_index.json)     
and staging version (http://arduino.esp8266.com/staging/package_esp8266com_index.json)     
  
Additionnaly:   
--Use minimal css from http://getbootstrap.com/examples/theme/  

##Flash the Module    
*Tools:      
--Use IDE to upload directly  (latest version of board manager module generate one binary)     
-- to flash the htm files present in data directory you need to use another tool, installation and usage is explained here:  http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html#file-system   


*Connection
--Connect GPIO0 to ground to be in update mode

<H3>Do not flash Printer fw with ESP connected - it bring troubles, at least on DaVinci     </H3>

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

These are the pages defined using template:    
Home page :     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page1.png><br>
System Configuration Page:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page2.png><br>     
Access Point Configuration Page:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page3.png><br>     
Client Configuration Page:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page4.png><br>     
Printer Status Page for 64K SPIFFS, due to limited space available no fancy:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page5-2.png><br>    
Printer Status Page for more than 64K SPIFFS, fancy one:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/page5.png><br>     
Extra Settings Page, for web UI and for printer:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page6.png><br>     
the template files are stored on SPIFFS:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/files.png><br>
and uploaded using [IDE](http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html#file-system)    
The list of keywords can be find here : https://github.com/luc-github/ESP8266/blob/master/keywords.txt     
Any files on SPIFFS can be called on web interface without having the path hard coded  - this give more flexibility,  favicon.ico is a good example of it.         
So UI is kind of separated from FW which allow easier modifications. For this a light file manager is available in extra settings page, it allows to upload/download/delete files. as SPIFFS is flat filesystem no directory management is necessary so it is very simple.

Additionally 404.tpl (the page not found) and restart.tpl(restart page when applying changes) are not mandatory, a fail safe version is embeded if they are not present.     

Currently, I tested on ESP01 using 64K SPIFFS ( please use data directory content accordingly due to space limitation) and NodeMCU 1.0 1M SPIFFS.     

##Protocol for discovery   
*mDNS : on Station mode only with bonjour installed on computer (done)  
*SSDP : on Station and AP mode (done)    
*Captive portal : on AP mode only (not yet functionnal)    

##Commands/msg from/to serial(not fully implemented):    
*from module to printer by serial communication   
    -M117 [Message], Error/status message from module (done)     
    -Send Wifi settings [AP/STATION,SSID,DHC/STATIC,IP,MASK,GW,STATUS,MAC ADDRESSS, BAUD?], ]Module configuration without password    
        
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
    -IP Static: [ESP103]STATIC    
    -IP DHCP: [ESP103]DHCP    
 
 
##Front End (implemented)
--Display printer status (done)   
--Display temperatures (done)    
--Display positions/flow/speed (done)    
--Display print progress if any (done)   
--List SDCard Content (done)   
--Launch a Print (done)    
--Stop/Pause a Print (done)   
--Emergency Stop (done)   
--Jog control / custom commands (done)     
 
##TODO   
-- Close open topics    
-- Do testing (a lot)    
-- UI Improvement    
--<s>Printer EEPROM management</s>    (Canceled/Postponed for next stage)

##Result of ESP12E on Davinci    
I use a proto board to connect ESP12E socket, one micro switch for recovery, one jumper for normal usage/ flash, I did not put hardware switch.    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Davinci/board.jpg><br>  
Connected to Davinci:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Davinci/boardconnected.jpg><br>  
The back cover:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Davinci/backside.jpg><br>  
The screen when connected to AP:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Davinci/screen.jpg><br>  
The settings:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Davinci/Capture.PNG><br>  
 
##Result of ESP12E on Due/RADDS
 Use Serial1 for communications
 <img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/RADDS/RADDS.png><br> 
 the rendering on screen when connection to AP is done:   
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/RADDS/screen.jpg><br> 
 The settings:     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/RADDS/Capture.PNG><br>  
