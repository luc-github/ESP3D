# ESP8266
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
<S>--Use GPIO2 connected to a printer pin to interrupt the bridge loop if need change configuration (may be not necessary if no performance impact of allowing ESP screening commands - TBT - TBC)   </S> no need anymore
--TODO: Full wiring with drawing     
TBD         

##Development   
Currently using [Arduino IDE 1.6.3](http://arduino.cc/en/Main/Software)  with the stand alone  esp8266 module 0.0.3 (https://github.com/sandeepmistry/esp8266-Arduino)    
Full IDE from https://github.com/esp8266/Arduino can be used   
  
Additionnaly:   
--Use minimal css from http://getbootstrap.com/examples/theme/ if connected to internet, this is to get better UI and rendering according display device (link can be changed in FW), if not available ,CSS is ignored displaying basic HTML   
--Some Javascript is used to enhance web UI - very limited but I expected so error depending browser so will try to limit usage (test done using chrome/IE looks Ok but no time for safari/firefox/opera/etc...)    

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
IP: 192.168.0.1   
Mask: 255.255.255.0   
GW:192.168.0.1    
<S>Bridge Mode   </S> no need anymore
Baud rate: 9600 

From web: 
port 80
--Need picture and flow     
...    
TBD


##Commands from/to serial:    
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
-- show available AP in web page and allow to select 
-- show connected clients if in AP 
-- need to display wifi information in front end
-- need to display MAC address in front end/ configuration to help if MAC filtering on router is necessary

more to come    
 
