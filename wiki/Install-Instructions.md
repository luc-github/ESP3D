For people not willing to read check this great video from Chris Riley :
[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/pJGBRriNc9I/0.jpg)](https://www.youtube.com/watch?v=pJGBRriNc9I)

## Download and prepare the Arduino IDE: ##

`1.1`  Install Arduino IDE version 1.X from https://www.arduino.cc/en/Main/Software

`1.2`  Install the Arduino IDE

`2.`  Open the Arduino IDE and click File, Preferences   
`2.1`   In the "Additional Boards Manager URL field:  Paste: http://arduino.esp8266.com/stable/package_esp8266com_index.json   
`2.2`   Click OK  
`2.3`   Click Tools -> Boards -> Board Manager   
`2.4`   Scroll to near the bottom, and find "esp8266 by ESP8266 Community) and click on the row   
`2.5`   On the "Select Version" dropdown, select latest version and click Install   
`2.6`   Wait for the ESP8266 support to be installed   

`3.`   Download and install the SPIFFS Uploader tool    
<B>EDIT:This part is no more necessary since FW 0.9.99 which contains self uploader </B>  
`3.1`   Go to https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst#uploading-files-to-file-system   
`3.2`   Download the ESP8266FS tool from the page above   
`3.3`  Open a file manager to your Arduino sketchbook directory. If you don't know where that is, click File, Preferences in the Arduino IDE and look at the field: Sketchbook Location   
`3.4`   Inside your sketchbook folder, create a new directory called 'tools'   
`3.5`   Extract the content of ESP8266FS-x.x.x.zip into Tools (So it ends up with something like /home/user/Documented/sketchbook/tools/ESP8266FS/tool/esp8266fs.jar    
`3.6`   Restart the Arduino IDE

## Download and install the code ##

`4.`   Download the latest release of this project:    
https://github.com/luc-github/ESP3D/releases/latest  
`4.1`   Extract it to your sketchbook or other location     
`4.2`   Open the Arduino IDE and open the ESP8266 subdirectory -> esp8266.ino (or esp3d.ino for latest versions)

`5.`   Configure your Board
_NB:  Read [this article for NB notes](https://github.com/luc-github/ESP8266/wiki/Flash-Size) on selecting the correct Board settings._
To recap:  
`5.1`   Make sure you have the clock speed set to 160Mhz  
`5.2`   Make sure you have the correct Flash size selected (More details [here](https://github.com/luc-github/ESP8266/wiki/Flash-Size#figuring-out-the-flash-size))   

_Next:  Configure your ESP8266 for upload (USB to serial plugged in, GPIO0 and GPIO15 pulled low, RST pulled high)_

`6.`  Upload the sketch:  Click the Upload button in Arduino IDE (Or press Ctrl+U) 

_Reboot the ESP8266 into run mode ((USB to serial removed , GPIO0 pulled high,  GPIO15 pulled low, RST pulled high)_

`7.`  Fire up a device and scan for WIFI access points

`7.1`  Find the AP called 'ESP8266' 

`7.2`  Connect to the AP using the default password of '12345678'

`7.3`  Upload index.html.gz file to the SPIFFS filesystem using web page uploader

## Initial Configuration ##

`9.`  Open device web page on the AP connected device 

`9.1`  Accept Captive portal redirect or

`9.2`  Open a web browser and navigate to http://192.168.0.1

`10.`  Login in using admin/admin and configure the device to your choosing   
`10.1`  I recommend changing to Station mode and connecting to your home/office Wifi instead of staying in AP mode   
`10.2`  You may want to change the Baud rate   
`10.3`  You can change to DHCP,  or at the very least setup a Static IP you are familiar with.

## Wire up and use ##

`11.`  Connect to your printer's serial port    

**Other NB things to keep in mind:**

* After applying power the ESP8266 takes approx 10 seconds before it will send "M117 <ip address>" on the serial port.  If your printer is connected to the ESP8266, and has an LCD connected, the M117 command is "Print this message to the LCD" - i.e after a successful boot it will print the IP address to the printer's LCD


* If you mess up a configuration you can pull down GPIO2 during reset/powerup to wipe the settings stored in EEPROM.

# Still having issue ?
If behavior is not consistent, you may need to erase the full flash, for that use the esptool present in your ESP core instalation in tools directory with option `--chip auto erase_flash`   
So in my case on git version of ESP32 under windows :    
`C:\Users\user\Documents\Arduino\hardware\espressif\esp32\tools\esptool>esptool.exe --chip auto erase_flash`

esptool can also be found here : https://github.com/espressif/esptool 
