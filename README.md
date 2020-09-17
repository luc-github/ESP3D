<span align="left"><img src="https://github.com/luc-github/ESP3D/blob/2.1/images/ESP3D.png" width="200px"/></span><span align="left">Firmware for ESP8266/ESP8285  and ESP32 used with 3D printer</span>

[Latest stable release ![Release Version](https://img.shields.io/github/release/luc-github/ESP3D.svg?style=plastic) ![Release Date](https://img.shields.io/github/release-date/luc-github/ESP3D.svg?style=plastic)](https://github.com/luc-github/ESP3D/releases/latest/) [![Travis (.org) branch](https://img.shields.io/travis/luc-github/ESP3D/2.1?style=plastic)](https://travis-ci.org/luc-github/ESP3D) [![Release Version](https://img.shields.io/github/v/release/luc-github/ESP3D-WEBUI?color=green&include_prereleases&label=WebUI&style=plastic)](https://github.com/luc-github/ESP3D-WEBUI/tree/2.1)      
please use Arduino ide 1.8.9+ with [![Release Version](https://img.shields.io/badge/ESP32-1.0.4-green?style=plastic)](https://github.com/espressif/arduino-esp32/releases/tag/1.0.4) or [![Release Version](https://img.shields.io/badge/ESP8266-2.5.2-green?style=plastic)](https://github.com/esp8266/Arduino/releases/tag/2.5.2)

[Latest development version ![Development Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic) ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D/3.0?style=plastic)](https://github.com/luc-github/ESP3D/tree/3.0) [![Travis (.org) branch](https://img.shields.io/travis/luc-github/ESP3D/3.0?style=plastic)](https://travis-ci.org/luc-github/ESP3D) [![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=WebUI)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0)   
please use Arduino ide 1.8.9+ with [![Release Version](https://img.shields.io/badge/ESP32-git-yellow?style=plastic&logo=github)](https://github.com/espressif/arduino-esp32) or [![Release Version](https://img.shields.io/badge/ESP8266-git-yellow?style=plastic&logo=github)](https://github.com/esp8266/Arduino/)

[All releases](https://github.com/luc-github/ESP3D/releases)

This firmware allows not only to have a cheap bridge between Wifi and serial, but also to have a web UI to configure wifi, to monitor 3D printer and even control it, and to make things easy,
UI is fully customizable without reflashing FW.

Firmware should work with any 3D printer firmware (repetier/marlin/smoothieware using GCODE) if serial connection has a correct setup.
I currently use it with my personnal flavor of [repetier for Due based boards](https://github.com/luc-github/Repetier-Firmware-0.92).

The web interface files are present in data directory but UI has it's own repository [ESP3D-WEBUI](https://github.com/luc-github/ESP3D-WEBUI).
* Be aware  ESP3D-WEBUI is for firmware 0.9.99 minimum - previous released version use tpl files which are no more used.
* Note for ESP8266 1MB flash : FW is now too big will all features you need to chose strip the FW and select only some features, also WebUI is now also too big for full multilanguage support to fit the 128K SPIFFS so please use pack with limited language (en +another) https://github.com/luc-github/ESP3D-WEBUI/tree/2.1/languages
    

## Sponsors 
[<img width="200px" src="https://raw.githubusercontent.com/luc-github/ESP3D/3.0/images/sponsors-supporters/FYSETC/LOGO.png">](https://www.fysetc.com/)  

## Supporters
[<img src="https://raw.githubusercontent.com/luc-github/ESP3D/3.0/images/sponsors-supporters/PanucattDevices/Panucatt.jpg" />](http://www.panucatt.com/)


## Become a sponsor or a supporter
 * A sponsor is a recurent donator [<img src="https://raw.githubusercontent.com/luc-github/ESP3D/3.0/images/sponsors-supporters/sponsor.PNG" />](https://github.com/sponsors/luc-github)   
 The github sponsors are automaticaly displayed by github, if your tier is `A big hug` or more, to thank you for your support, your logo / avatar will be also added to the readme page with eventually with a link to your site.    

 * A supporter is per time donator [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Y8FFE7NA4LJWQ)   
 If your donation is over a per year sponsor tier `A big hug`, to thank you for your support, your logo / avatar will be added to the readme page with eventually with a link to your site.    

 Every support is welcome, giving support/ developing new features need time and devices, donations contribute a lot to make things happen, thank you.

## Features
* Serial/Wifi bridge using configurable port 8888
* Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do a hard recovery for settings
* Complete configuration by web browser (Station or Access point) or by Serial/telnet commands
* Authentication (optional) for better security
* Update firmware by web browser
* Captive portal in Access point mode which redirect all unknow call to main page
* mDNS which allows to key the name defined in web browser and connect only with bonjour installed on computer
* SSDP, this feature is a discovery protocol, supported on Windows out of the box
* Fail safe mode (Access point)is enabled if cannot connect to defined station at boot.
* Embedded FS uploader and updater.
* OLED screen support  
* Notifications using Line / Pushover / email   
* The web ui add even more feature : https://github.com/luc-github/ESP3D-WEBUI/blob/2.1/README.md#features  

## Default Configuration      
Default Settings:    
* Access Point: ESP3D    
* PW:12345678   
* Authentification: WPA     
* IP: 192.168.0.1   
* Baud rate: 115200   
* Web port:80 
* Data port: 8888     
if Authentication is enabled :
* User: admin   
* Password: admin   
* User:user   
* Password: user   



## Direct commands:    
Check wiki : https://github.com/luc-github/ESP3D/wiki/Direct-ESP3D-commands

## Installation
Feedback on 2.0 was : ESP3D being a library is not really useful and make setup more complex, so now we are back to simple application.   

1. Please follow installation of the ESP core you want to use : [ESP8266 core version](https://github.com/esp8266/Arduino)  or [ESP32 core version](https://github.com/espressif/arduino-esp32)   
2.  Add manually libraries present in libraries directory to your arduino libraries directory (no need if you use platformIO) -these versions are verified to work with ESP3D, any others (newer version) may cause untested behavior.  

Use webserver support (recommended as stable), asyncwebserver support is no more stable on ESP3D 
* arduinoWebSockets from @Links2004  
 
Generic ones:      
Specific for ESP32    
* ESP32SSDP
If you want OLED support:  
* oled-ssd1306 from @squix78    

If you want DHT11/22 support:  
* DHT_sensor_library_for_ESPx from @beegee-tokyo   
3. Compile project esp3d.ino according target: ESP8266 board or ESP32 board, please review config.h to enable disable a feature, by default athentication is disabled and most others are enabled.   
* for ESP8266 set CPU freq to 160MHz for better (https://github.com/luc-github/ESP3D/wiki/Install-Instructions)
4. Upload the data content on ESP3D file system
* Using SPIFFS uploader, this plugin and install instructions is available on each ESP core - please refere to it
or
* Using embedded uploader (you may need to format SPIFFS using : [ESP710]FORMAT on ESP8266 first)    
if embedded uploader does not show up you can force it ti display using : http://your_IP_address?forcefallback=yes    
<img src=https://raw.githubusercontent.com/luc-github/ESP3D/2.1/images/docs/embedded.png><br>


## Update
* Generate a binary using the export binary menu from Arduino IDE and upload it using ESP-WEBUI or embedded interface  

<H3>:warning:Do not flash your Printer fw with ESP connected on Serial - it bring troubles, at least on DaVinci, but no issue if you update using web UI</H3>

## Contribution/customization
* To style the code before pushing PR please use [astyle --style=otbs *.h *.cpp *.ino](http://astyle.sourceforge.net/)   
* The embedded page is created using nodejs then gulp to generate a compressed html page (tool.html.gz), all necessary modules will be installed using the build.bat, you also need bin2c tool (https://sourceforge.net/projects/bin2c/) to generate the h file from the binary,  installation and build is done using the build.bat.   
* The corresponding UI is located [here](https://github.com/luc-github/ESP3D-WEBUI/tree/2.1)
* An optional UI was development using old repetier UI - check [UI\repetier\testui.htm] (https://github.com/luc-github/ESP3D/blob/master/UI/repetier/testui.htm) file   

 
## Need more information about supported boards or wiring ?
[Check the wiki](https://github.com/luc-github/ESP3D/wiki)

## :question:Any question ?   
Check [Wiki](https://github.com/luc-github/ESP3D/wiki/Install-Instructions) or Join the chat at [Discord server](https://discord.gg/Z4ujTwE)   

## :exclamation:Any issue/feedback ?    
Check [Wiki](https://github.com/luc-github/ESP3D/wiki/Install-Instructions) and [FAQ](https://github.com/luc-github/ESP3D/issues?utf8=%E2%9C%93&q=label%3AFAQ+) or [submit ticket](https://github.com/luc-github/ESP3D/issues)    

## ESP3D is used by :
* Opensource version is used by various boards like : https://www.aliexpress.com/wholesale?SearchText=esp3d&opensearch=true&switch_new_app=y
and https://youtu.be/XoWW0aU6DGE?t=76  


* Custom version (not this repository) is used on azteeg mini wifi : http://www.panucatt.com/azteeg_X5_mini_reprap_3d_printer_controller_p/ax5mini.htm    
and Wifi boards for 3D printers : https://www.panucatt.com/ProductDetails.asp?ProductCode=WB8266,   
<h2>:warning: Do not use this repository for boards using custom firmware, you will lose several features.</h2> 

* More to come...   

## :+1:Thanks
* to @disneysw for bringing this module idea
* to @lkarlslund for suggestion about independent reset using GPIO2
* to Roy Cortes from http://www.panucatt.com for supporting and pushing me implementing great features
* to all contributors, feedbacks owners and donations.


If you use ESP3D on your product, drop me a message so I can link your product page here.   



