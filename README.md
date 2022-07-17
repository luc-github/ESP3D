# ESP3D 3.0 alpha stage

<img src="https://github.com/luc-github/ESP3D/blob/3.0/images/Screen/logo2.png">
<H3>Do not use it for production, in alpha stage for 3D printer/Sand-Table and CNC - breaking changes can happen anytime - just be aware of that, but feel free to test it and feedback</H3>

To compile ESP3D you need to edit the configuration.h according your needs, you can also generate it, using the configurator which simplify a lot this step: https://luc-github.github.io/

ESP3D V3 use ESP3D-WebUI 3.0, but it is built according your system and your system firmware: https://github.com/luc-github/ESP3D-WEBUI/3.0/dist/, so you need to use the right one, the [configurator](https://luc-github.github.io/) may also help you to download the correct version for your needs.

[Latest development version ![Development Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic) ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/luc-github/ESP3D/3.0?style=plastic)](https://github.com/luc-github/ESP3D/tree/3.0) [![github-ci](https://github.com/luc-github/ESP3D/workflows/build-ci/badge.svg)](https://github.com/luc-github/ESP3D/actions/workflows/build-ci.yml) [![Development  Version](https://img.shields.io/badge/Devt-v3.0-yellow?style=plastic&label=WebUI)](https://github.com/luc-github/ESP3D-WEBUI/tree/3.0)

please use Arduino ide 1.8.9+ with [![Release Version](https://img.shields.io/badge/ESP32-git-yellow?style=plastic&logo=github)](https://github.com/espressif/arduino-esp32) or [![Release Version](https://img.shields.io/badge/ESP8266-git-yellow?style=plastic&logo=github)](https://github.com/esp8266/Arduino/)  
or PlatformIO

Because ESP3D 3.0 is actually a set of several projects: [ESP3D](https://github.com/luc-github/ESP3D), [ESP3D-Configurator](https://github.com/luc-github/ESP3D-Configurator), [ESP3D-WEBUI](https://github.com/luc-github/ESP3D-WEBUI), [ESP3DLib](https://github.com/luc-github/ESP3DLib), [ESP32SSDP](https://github.com/luc-github/ESP32SSDP), [svg-icons](https://github.com/luc-github/svg-icons)

I did a project board to put all majors tasks/issues in one places : [ESP3D Project](https://github.com/users/luc-github/projects/1/views/1)

# TODO List to move to beta

- [Finish web UI Major features (3D printing and GRBL)](https://github.com/luc-github/ESP3D-WEBUI/issues/94#issuecomment-660600551)
- Do user documentation: Readme / features list description / wiki/ Features videos etc...
- Test current features heavily
- Collect some feedback

# TODO List in beta

- Test heavily (still)
- [Implement minor missing features not present in alpha](https://github.com/luc-github/ESP3D-WEBUI/issues?q=is%3Aissue+is%3Aopen+label%3A3.0)
- Translate UI and build language packs
- Adjust features according feedback
- Refactoring code when necessary
- TBD

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
