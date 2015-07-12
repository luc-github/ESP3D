# ESP8266

[![Join the chat at https://gitter.im/luc-github/ESP8266](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/luc-github/ESP8266?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
FW for ESP8266 used with 3D printer

##Description      
Thanks to @disneysw for bringing this module idea    
Thanks to @lkarlslund for suggestion about independant reset using GPIO2   

Have a bridge configurable by web (implemented) and optionally by printer (not yet implemented)  
Have a front end to know what is the wifi status (implemented) or know what is the print status (not yet implemented) - this part can be optional and removed by compilation directive if no need    

Should be compatible with reprap printer (Marlin FW/Repetier FW)       

##Hardware connection       
--Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW - I did not found information on this. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
--Use GPIO0 to ground to be in update mode   
--Use a switch to reset/disable module    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Wires.png><br>   
       

##Development   
Currently using [Arduino IDE 1.6.4](http://arduino.cc/en/Main/Software)  with the esp8266 module from board manager added from [github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)    
using relased version (http://arduino.esp8266.com/package_esp8266com_index.json)     
and staging version (http://arduino.esp8266.com/staging/package_esp8266com_index.json)     
  
Additionnaly:   
--Use minimal css from http://getbootstrap.com/examples/theme/ if connected to internet, this is to get better UI and rendering according display device (link can be changed in FW), if not available ,CSS is ignored displaying basic HTML   

##Flash the Module    
*Tools:      
--Use IDE to upload directly  (latest version of board manager module generate one binary)    

*Connection
--Connect GPIO0 to ground to be in update mode

##Web configuration      
*Wifi Mode : Access point / Client station  
*IP Generation: DHCP/Static IP      
*IP/MASK/GATEWAY for static data    
*Baud Rate for serial (supported : 9600, 19200, 38400, 57600, 115200, 230400)    
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

These are the UI when css is in cache or internet is available:    
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page1.png><br> 
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page2.png><br>     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page3.png><br>     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/Page4.png><br>     
<img src=https://raw.githubusercontent.com/luc-github/ESP8266/master/page5.png><br>     


##Commands from/to serial(not yet implemented):    
*from module to printer   [Need Printer FW support and can be disabled in ESP FW]    
    -M800 S1 , restart module done need a wifi/activity restart      
    -M801 [Message], Error message from module      
    -M802 [Message], Status message from module        
    -M804 [AP/STATION,SSID,DHC/STATIC,IP,MASK,GW,STATUS,MAC ADDRESSS, BAUD?], ]Module configuration without password    
        
*from host to printer   (not yet implemented) [Need Printer FW support] on port 8888    
    -M803 [IP, AP,SSID, Password....], ]Module configuration settings to be used  by module    
    -M805 query to get M804 informations    
          


*from printer to module   [Need Printer FW support and can be disabled in ESP FW] (not yet implemented)  
    -request configuration/status (generate the M804 as answer)   
    -set AP/STATION,SSID,PASSSWORD,DHC/STATIC,IP,MASK,GW,BAUD from serial    
 
##Front End (basic implemented)  [need Printer FW support it] or just display module status 
--Display printer status   
--Display temperatures   (done)    
--Display positions/flow/speed   (done)    
--Display print progress if any   
--List SDCard Content   
--Launch/Stop/(Pause?) a Print   
 
##TODO    
-- Define Front End Functions  
-- do testing   
-- do a complete drawing for connections    
-- add some Javascript to enhance web UI     


more to come    
 
