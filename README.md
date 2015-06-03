# ESP8266

[![Join the chat at https://gitter.im/luc-github/ESP8266](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP8266?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
FW for ESP8266 used with 3D printer

##Description      
Thanks to @disneysw for bringing this module idea and basic code    
Thanks to @lkarlslund for suggestion about independant reset using GPIO2   

Have a bridge configurable by web and optionally by printer   
Have a front end to know what is the wifi status or know what is the print status  - this part can be optional and removed by compilation directive if no need    
...     
TBD     

##Hardware connection       
--Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW - I did not found information on this. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
--Use GPIO0 to ground to be in update mode   
--Use a switch to reset/disable module    
--TODO: Full wiring with drawing     
TBD         

##Development   
Currently using [Arduino IDE 1.6.4](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager added from [github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)
  
Additionnaly:   
--Use minimal css from http://getbootstrap.com/examples/theme/ if connected to internet, this is to get better UI and rendering according display device (link can be changed in FW), if not available ,CSS is ignored displaying basic HTML   

##Flash the Module    
*Tools:      
--Use IDE to upload directly    
--Use esp flasher: https://github.com/nodemcu/nodemcu-flasher  for 0x00000 and 0X00004 binaries   

*Connection
--Connect GPIO0 to ground to be in update mode 

##Wifi connection      
*Wifi Mode : Access point / Client station    (not sure it is useful to handle AP/STA in same time as make configuration more complex, but use AP/STA only to help the configuration mode = be able to scan other AP/ test connection when in AP mode )   
*IP Generation: DHCP/Static IP      
*IP/MASK/GATEWAY for static data    
<S>*Usage: Bridge/Front End  </S> no need anymore     
*Baud Rate for serial (supported : 9600, 19200, 38400, 57600, 115200, 230400)    
...       
TBD   
    
##Configuration      
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

From web: 
port 80
--Need picture and flow    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page1.png><br> 
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page2.png><br>     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page3.png><br>     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page4.png>     


...    
TBD


##Commands from/to serial(not yet implemented):    
*from module to printer   [Need Printer FW support and can be disabled in ESP FW]    
    -M800 S1 , restart module done need a wifi/activity restart      
    -M801 [Message], Error message from module      
    -M802 [Message], Status message from module        
    -M804 [AP/STATION,SSID,DHC/STATIC,IP,MASK,GW,STATUS,MAC ADDRESSS, BAUD?], ]Module configuration without password    
    ...    
    TBD    
        
*from host to printer    [Need Printer FW support] on port 8888    
    -M803 [IP, AP,SSID, Password....], ]Module configuration settings to be used  by module    
    -M805 query to get M804 informations    
    ...      
    TBD           


*from printer to module   [Need Printer FW support and can be disabled in ESP FW]   
    -request configuration/status (generate the M804 as answer)   
    -set AP/STATION,SSID,PASSSWORD,DHC/STATIC,IP,MASK,GW,BAUD from serial    
    ...    
    TBD    
 
##Front End [need Printer FW support it] or just display module status
--Display printer status   
--Display temperatures   
--Display print progress if any   
--List SDCard Content   
--Launch/Stop/(Pause?) a Print
 
##TODO    
-- Define Front End Functions  
-- do the bridge serial/TCPIP
-- do coding   
-- do testing   
-- do a complete drawing for connections    
-- add some Javascript to enhance web UI 
-- allow to change ports from Web UI, currently fixed to 80 and 8888


more to come    
 
