# ESP3D 3.0 ![ESP3D](https://img.shields.io/badge/dynamic/json?label=ESP3D&query=$.version&url=https://raw.githubusercontent.com/luc-github/ESP3D/refs/heads/3.0/info.json)
<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-2-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->
<img src="https://github.com/luc-github/ESP3D/blob/3.0/images/Screen/logo2.png">
Firmware for ESP8266/ESP8285 and ESP32 (original, pico, S2, S3, C3, C6) used with 3D printer/Sand-Table and CNC 

To compile ESP3D you need to edit the configuration.h according your needs.   
You can also generate it, using [![Development  Version](https://img.shields.io/badge/ESP3D-Configurator-red?style=for-the-badge&logo=preact)](https://luc-github.github.io/) which simplify a lot this step.   
or click here: https://luc-github.github.io/      

ESP3D 3.0 use [![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=ESP3D-WEBUI)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0), but it is built according your system and your system firmware:[![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=WebUI%20Packages)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0/dist/). 

so you need to use the right one, the [ESP3D-Configurator](https://luc-github.github.io/) may also help you to download the correct version for your needs.

![Development](https://img.shields.io/badge/dynamic/json?label=Development&query=$.devt&color=green&style=plastic&url=https://raw.githubusercontent.com/luc-github/ESP3D/refs/heads/3.0/info.json)
[![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=ESP3D-WEBUI)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0)
![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D/3.0?style=plastic)
[![github-ci](https://github.com/luc-github/ESP3D/workflows/build-ci/badge.svg)](https://github.com/luc-github/ESP3D/actions/workflows/build-ci.yml)


Please go to [esp3d.io](http://esp3d.io/esp3d/v3.x/index.html) for documentation and installation

> [!WARNING]
>### Disclaimer
> The software is provided 'as is,' without any warranty of any kind, expressed or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages, or other liability, whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the software or the use or other dealings in the software.
>It is essential that you carefully read and understand this disclaimer before using this software and its components. If you do not agree with any part of this disclaimer, please refrain from using the software.  

# SSDP Discovery tools
   * Windows : 
       - Just go to Network page
   * OSX : TBD
   * Linux :
       - On Debian-style distros, install `gupnp-tools` and run `gssdp-discover -i <devicename> --timeout=3`
   * Android :
       - SSDP/UPnP Scanner : https://play.google.com/store/apps/details?id=com.vgc.ssdpscan
       

## Sponsors 
<div align="center">
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/diamond-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/diamond-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-0.svg?v=1" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/platinum-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/platinum-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/gold-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/gold-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   <div style="display:flex; flex-wrap:wrap; gap:20px; justify-content:center; margin-bottom:20px">
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-0.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-0.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-1.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-1.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-2.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-2.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-3.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-3.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-4.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-4.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-5.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-5.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-6.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-6.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-7.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-7.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-8.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-8.svg" style="max-width:400px; width:auto; height:auto"></a>
       <a href="https://luc-github.github.io/sponsors/esp3d/silver-9.html" target="_blank" rel="noopener noreferrer"><img src="https://luc-github.github.io/sponsors/esp3d/silver-9.svg" style="max-width:400px; width:auto; height:auto"></a>
   </div>
   Support ESP3D Development - <a href="https://esp3d.io/sponsoring" target="_blank" rel="noopener noreferrer">Become a Sponsor</a>
</div>


# Chat

Please use discord : [![discord](https://img.shields.io/discord/752822148795596940?color=blue&label=discord&logo=discord)](https://discord.gg/Z4ujTwE)

# Credits: embedded code / inspiration code

- FTP server come from Jean-Michel Gallego https://github.com/gallegojm/Arduino-Ftp-Server
- WebDav server come from David Gauchard https://github.com/d-a-v/ESPWebDAV
- Mks support come from https://github.com/makerbase-mks/MKS-WIFI
- Line support come from https://github.com/TridentTD/TridentTD_LineNotify and https://notify-bot.line.me/doc/en/ and https://pushover.net/api
- Pushover support come from https://github.com/ArduinoHannover/Pushover
- Email support come from https://github.com/CosmicBoris/ESP8266SMTP and https://www.electronicshub.org/send-an-email-using-esp8266/
- Telegram support come from https://medium.com/@xabaras/sending-a-message-to-a-telegram-channel-the-easy-way-eb0a0b32968
- HomeAssistant support come from https://developers.home-assistant.io/docs/api/rest/

# Credits: libraries

- Websockets come from Markus Sattler https://github.com/Links2004/arduinoWebSockets
- BMx280MI come from Gregor Christandl https://bitbucket.org/christandlg/bmx280mi
- DHT sensor come from beegee_tokyo http://desire.giesecke.tk/index.php/2018/01/30/esp32-dht11/
- ESP32SSDP come from luc-github https://github.com/luc-github/ESP32SSDP
- ESP8266-Arduino-Lua come from François Dugast but esp8266/esp32 compatible version is here https://github.com/luc-github/ESP8266-Arduino-Lua as PR is not merged
- ESP8266 and ESP32 Oled Driver for SSD1306 display come from Daniel Eichhorn, Fabrice Weinberg https://github.com/ThingPulse/esp8266-oled-ssd1306
- LittleFS_esp32 come from lorol https://github.com/lorol/LITTLEFS
- lv_arduino come from Pavel Brychta <pablo@xpablo.cz> https://littlevgl.com
- SdFat come from Bill Greiman https://github.com/greiman/SdFat
- TFT_eSPI come from Bodmer https://github.com/Bodmer/TFT_eSPI
- ESP8266 core come from https://github.com/esp8266/Arduino
- ESP32 core come from https://github.com/espressif/arduino-esp32


## Contributors ✨

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):
<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tbody>
    <tr>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/BerranRemzi"><img src="https://avatars.githubusercontent.com/u/11856339?v=4?s=100" width="100px;" alt="Berran Remzi"/><br /><sub><b>Berran Remzi</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/issues?q=author%3ABerranRemzi" title="Bug reports">🐛</a> <a href="https://github.com/luc-github/ESP3D/commits?author=BerranRemzi" title="Code">💻</a></td>
      <td align="center" valign="top" width="14.28%"><a href="https://github.com/Binch3000"><img src="https://avatars.githubusercontent.com/u/168651837?v=4?s=100" width="100px;" alt="Binch3000"/><br /><sub><b>Binch3000</b></sub></a><br /><a href="https://github.com/luc-github/ESP3D/issues?q=author%3ABinch3000" title="Bug reports">🐛</a></td>
    </tr>
  </tbody>
</table>

<!-- markdownlint-restore -->
<!-- prettier-ignore-end -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

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
