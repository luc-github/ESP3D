<span align="left"><img src="https://github.com/luc-github/ESP3D/blob/2.1/images/ESP3D.png" width="200px"/></span><span align="left">Firmware for ESP8266/ESP8285  and ESP32 used with 3D printer</span>  
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-26-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->
[<img src="https://img.shields.io/liberapay/patrons/ESP3D.svg?logo=liberapay">](https://liberapay.com/ESP3D)

[Latest stable release ![Release Version](https://img.shields.io/github/release/luc-github/ESP3D.svg?style=plastic) ![Release Date](https://img.shields.io/github/release-date/luc-github/ESP3D.svg?style=plastic)](https://github.com/luc-github/ESP3D/releases/latest/) [![github-ci](https://github.com/luc-github/ESP3D/workflows/build-ci-2.0/badge.svg)](https://github.com/luc-github/ESP3D/actions/workflows/build-ci-2.0.yml) [![Release Version](https://img.shields.io/github/v/release/luc-github/ESP3D-WEBUI?color=green&label=WebUI&style=plastic)](https://github.com/luc-github/ESP3D-WEBUI/tree/2.1)      
please use Arduino ide 1.8.9+ with [![Release Version](https://img.shields.io/badge/ESP32-1.0.4-green?style=plastic)](https://github.com/espressif/arduino-esp32/releases/tag/1.0.4) or [![Release Version](https://img.shields.io/badge/ESP8266-2.5.2-green?style=plastic)](https://github.com/esp8266/Arduino/releases/tag/2.5.2)

[Latest development version ![Development Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic) ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D/3.0?style=plastic)](https://github.com/luc-github/ESP3D/tree/3.0) [![github-ci](https://github.com/luc-github/ESP3D/workflows/build-ci/badge.svg)](https://github.com/luc-github/ESP3D/actions/workflows/build-ci.yml) [![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=WebUI)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0)   
please use Arduino ide 1.8.9+ with [![Release Version](https://img.shields.io/badge/ESP32-2.0.8-yellow?style=plastic&logo=github)](https://github.com/espressif/arduino-esp32) or [![Release Version](https://img.shields.io/badge/ESP8266-3.1.1-yellow?style=plastic&logo=github)](https://github.com/esp8266/Arduino/) [![Project Page ESP3D 3.0](https://img.shields.io/badge/Project%20page-ESP3D%203.0-blue)](https://github.com/users/luc-github/projects/1/views/1)

[All releases](https://github.com/luc-github/ESP3D/releases)

> [!WARNING]
>### Disclaimer
> The software is provided 'as is,' without any warranty of any kind, expressed or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.
>It is essential that you carefully read and understand this disclaimer before using this software and its components. If you do not agree with any part of this disclaimer, please refrain from using the software.  

This firmware allows not only to have a cheap bridge between Wifi and serial, but also to have a web UI to configure wifi, to monitor 3D printer and even control it, and to make things easy,
UI is fully customizable without reflashing FW.

Firmware should work with any 3D printer firmware (repetier/marlin/smoothieware using GCODE) if serial connection has a correct setup.
I currently use it with my personnal flavor of [repetier for Due based boards](https://github.com/luc-github/Repetier-Firmware-0.92).

The web interface files are present in data directory but UI has it's own repository [ESP3D-WEBUI](https://github.com/luc-github/ESP3D-WEBUI).
* Be aware  ESP3D-WEBUI is for firmware 0.9.99 minimum - previous released version use tpl files which are no more used.
* Note for ESP8266 1MB flash : FW is now too big will all features you need to chose strip the FW and select only some features, also WebUI is now also too big for full multilanguage support to fit the 128K SPIFFS so please use pack with limited language (en +another) https://github.com/luc-github/ESP3D-WEBUI/tree/2.1/languages

This branch does not take any new features, only bug fix, for new feature please use https://github.com/luc-github/ESP3D/tree/3.0, thank you.    
    

## Sponsors 
[<img width="200px" src="https://raw.githubusercontent.com/luc-github/ESP3D-WEBUI/2.1/images/sponsors-supporters/MKS/mksmakerbase.jpg" title="MKS Makerbase">](https://github.com/makerbase-mks)&nbsp;&nbsp;

## Supporters

## Become a sponsor or a supporter
 * A sponsor is a recurent donator    
If your tier is `10 US$/month` or more, to thank you for your support, your logo / avatar will be added to the readme page with eventually with a link to your site.    
 * A supporter is per time donator 
 If your donation is over `120 US$` per year, to thank you for your support, your logo / avatar will be added to the readme page with eventually with a link to your site.  

 Every support is welcome, indeed helping users / developing new features need time and devices, donations contribute a lot to make things happen, thank you.

* liberapay <a href="https://liberapay.com/ESP3D/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a> 
* Paypal [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FQL59C749A78L)
* ko-fi [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G0C0QT7)   

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
Check [Documentation](http://esp3d.io/esp3d/v2.x/documentation/commands/index.html)

## Installation instructions
See [Documentation](http://esp3d.io/esp3d/v2.x/installation/index.html)

## Installation
Feedback on 2.0 was : ESP3D being a library is not really useful and make setup more complex, so now we are back to simple application.   

1. If you haven't already set up Arduino IDE for ESPs then do so for [ESP8266](https://github.com/esp8266/Arduino) or [ESP32 core version](https://github.com/espressif/arduino-esp32).
2. Download the [latest release](https://github.com/luc-github/ESP3D/releases/) and manually copy the libraries present in the `ESP3D-x.y.z/libraries` directory into your `Arduino/libraries` directory.  (*no need if using platformIO*).  *These versions are verified to work with ESP3D, any others (newer version) may cause untested behavior.*
- Use webserver support (recommended as stable), asyncwebserver support is no longer stable on ESP3D 
- * arduinoWebSockets from @Links2004
 
Generic ones:      
Specific for ESP32    
* ESP32SSDP
If you want OLED support:  
* oled-ssd1306 from @squix78    

If you want DHT11/22 support:  
* DHT_sensor_library_for_ESPx from @beegee-tokyo   
3. Compile project esp3d.ino according target: ESP8266 board or ESP32 board, please review config.h to enable disable a feature, by default athentication is disabled and most others are enabled.   
* for ESP8266 set CPU freq to 160MHz for better   
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


 
## Need more information about supported boards or wiring ?
Check [Hardware support](http://esp3d.io/esp3d/v2.x/hardware/index.html) and [Wiring support](http://esp3d.io/esp3d/v2.x/installation/wiring/index.html)

## :question:Any question ?   
Check [Documentation](http://esp3d.io/esp3d/v2.x/index.html) or Join the chat at [![Discord server](https://img.shields.io/discord/752822148795596940?color=blue&label=discord&logo=discord)](https://discord.gg/Z4ujTwE)  

The reference site: http://esp3d.io/

## :exclamation:Any issue/feedback ?    
Check [FAQ](https://github.com/luc-github/ESP3D/discussions?discussions_q=category%3AF.A.Q) 
If you still have issue: [submit ticket](https://github.com/luc-github/ESP3D/issues)    
If it is not an issue join discussion [here](https://github.com/luc-github/ESP3D/discussions)

    

## ESP3D is used by :
* Opensource version is used by various boards like : https://www.aliexpress.com/wholesale?SearchText=esp3d&opensearch=true&switch_new_app=y
and https://youtu.be/XoWW0aU6DGE?t=76  

This is for information only - I am not linked to these products, it is just a search result of a query using ESP3D as keyword. So I am not responsible of any usage of them.


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

## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://ponc.tech"><img src="https://avatars.githubusercontent.com/u/1230627?v=4?s=100" width="100px;" alt="Harald Wagener"/><br /><sub><b>Harald Wagener</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=oliof" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/A7F"><img src="https://avatars.githubusercontent.com/u/17254471?v=4?s=100" width="100px;" alt="Luke"/><br /><sub><b>Luke</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=A7F" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/atsju"><img src="https://avatars.githubusercontent.com/u/4628382?v=4?s=100" width="100px;" alt="Julien Staub"/><br /><sub><b>Julien Staub</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=atsju" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://kingbain.com"><img src="https://avatars.githubusercontent.com/u/367922?v=4?s=100" width="100px;" alt="John Bain"/><br /><sub><b>John Bain</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=KingBain" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Xstasy"><img src="https://avatars.githubusercontent.com/u/283593?v=4?s=100" width="100px;" alt="Xstasy"/><br /><sub><b>Xstasy</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=Xstasy" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://www.upwork.com/freelancers/~016e4c7416f7d925c8"><img src="https://avatars.githubusercontent.com/u/171243?v=4?s=100" width="100px;" alt="Семён Марьясин"/><br /><sub><b>Семён Марьясин</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=MarSoft" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://encrypted.pcode.nl/"><img src="https://avatars.githubusercontent.com/u/929583?v=4?s=100" width="100px;" alt="Pascal de Bruijn"/><br /><sub><b>Pascal de Bruijn</b></sub></a><br /><a href="#ideas-pmjdebruijn" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/luc-github/ESP3D/commits?author=pmjdebruijn" title="Code">💻</a></td>
    </tr>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/jschwalbe"><img src="https://avatars.githubusercontent.com/u/8005921?v=4?s=100" width="100px;" alt="jschwalbe"/><br /><sub><b>jschwalbe</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=jschwalbe" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://padcom13.blogspot.com"><img src="https://avatars.githubusercontent.com/u/553994?v=4?s=100" width="100px;" alt="Matthias Hryniszak"/><br /><sub><b>Matthias Hryniszak</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=padcom" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/treepleks"><img src="https://avatars.githubusercontent.com/u/16371923?v=4?s=100" width="100px;" alt="T. Reepleks"/><br /><sub><b>T. Reepleks</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=treepleks" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://twitter.com/AustinStAubin"><img src="https://avatars.githubusercontent.com/u/969780?v=4?s=100" width="100px;" alt="Austin St. Aubin"/><br /><sub><b>Austin St. Aubin</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=AustinSaintAubin" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://patrickelectric.work"><img src="https://avatars.githubusercontent.com/u/1215497?v=4?s=100" width="100px;" alt="Patrick José Pereira"/><br /><sub><b>Patrick José Pereira</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=patrickelectric" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/j0hnlittle"><img src="https://avatars.githubusercontent.com/u/16280939?v=4?s=100" width="100px;" alt="John Little"/><br /><sub><b>John Little</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=j0hnlittle" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/d-a-v"><img src="https://avatars.githubusercontent.com/u/4800356?v=4?s=100" width="100px;" alt="david gauchard"/><br /><sub><b>david gauchard</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=d-a-v" title="Code">💻</a> <a href="#ideas-d-a-v" title="Ideas, Planning, & Feedback">🤔</a></td>
    </tr>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://www.mydiy.dev"><img src="https://avatars.githubusercontent.com/u/23615562?v=4?s=100" width="100px;" alt="coliss86"/><br /><sub><b>coliss86</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=coliss86" title="Documentation">📖</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/fape"><img src="https://avatars.githubusercontent.com/u/1052464?v=4?s=100" width="100px;" alt="Farkas Péter"/><br /><sub><b>Farkas Péter</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=fape" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://www.panucatt.com"><img src="https://avatars.githubusercontent.com/u/1669825?v=4?s=100" width="100px;" alt="Panucatt"/><br /><sub><b>Panucatt</b></sub></a><br /><a href="#ideas-royco" title="Ideas, Planning, & Feedback">🤔</a> <a href="#promotion-royco" title="Promotion">📣</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3Aroyco" title="Bug reports">🐛</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://discord.gg/yNwksQvZmQ"><img src="https://avatars.githubusercontent.com/u/12979070?v=4?s=100" width="100px;" alt="makerbase"/><br /><sub><b>makerbase</b></sub></a><br /><a href="#platform-makerbase-mks" title="Packaging/porting to new platform">📦</a> <a href="#financial-makerbase-mks" title="Financial">💵</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3Amakerbase-mks" title="Bug reports">🐛</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/liqijian101"><img src="https://avatars.githubusercontent.com/u/16173343?v=4?s=100" width="100px;" alt="liqijian101"/><br /><sub><b>liqijian101</b></sub></a><br /><a href="#ideas-liqijian101" title="Ideas, Planning, & Feedback">🤔</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3Aliqijian101" title="Bug reports">🐛</a> <a href="#platform-liqijian101" title="Packaging/porting to new platform">📦</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/disneysw"><img src="https://avatars.githubusercontent.com/u/10690321?v=4?s=100" width="100px;" alt="disneysw"/><br /><sub><b>disneysw</b></sub></a><br /><a href="#ideas-disneysw" title="Ideas, Planning, & Feedback">🤔</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://www.fysetc.com"><img src="https://avatars.githubusercontent.com/u/36067086?v=4?s=100" width="100px;" alt="FYSETC.COM"/><br /><sub><b>FYSETC.COM</b></sub></a><br /><a href="#financial-FYSETC" title="Financial">💵</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3AFYSETC" title="Bug reports">🐛</a> <a href="#platform-FYSETC" title="Packaging/porting to new platform">📦</a></td>
    </tr>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="http://www.bigtree-tech.com"><img src="https://avatars.githubusercontent.com/u/38851044?v=4?s=100" width="100px;" alt="BIGTREETECH"/><br /><sub><b>BIGTREETECH</b></sub></a><br /><a href="#platform-bigtreetech" title="Packaging/porting to new platform">📦</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/dbuezas"><img src="https://avatars.githubusercontent.com/u/777196?v=4?s=100" width="100px;" alt="David Buezas"/><br /><sub><b>David Buezas</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=dbuezas" title="Code">💻</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3Adbuezas" title="Bug reports">🐛</a></td>
      <td align="center" valign="top" width="14.28%"><a href="http://engineer2designer.blogspot.com"><img src="https://avatars.githubusercontent.com/u/25747949?v=4?s=100" width="100px;" alt="E2D"/><br /><sub><b>E2D</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=Engineer2Designer" title="Code">💻</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3AEngineer2Designer" title="Bug reports">🐛</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/rondlh"><img src="https://avatars.githubusercontent.com/u/77279634?v=4?s=100" width="100px;" alt="rondlh"/><br /><sub><b>rondlh</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=rondlh" title="Code">💻</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3Arondlh" title="Bug reports">🐛</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Levak"><img src="https://avatars.githubusercontent.com/u/2292040?v=4?s=100" width="100px;" alt="Levak"/><br /><sub><b>Levak</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/commits?author=Levak" title="Code">💻</a> <a href="https://github.com/luc-github/ESP3D/issues?q=author%3ALevak" title="Bug reports">🐛</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
