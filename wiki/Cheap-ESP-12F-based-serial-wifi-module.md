We can flash our loved ESP3D to cheap ESP-12F based serial wifi module (eg [from aliexpress](https://www.aliexpress.com/item/ESP8266-ESP-12F-Serial-WIFI-Wireless-Transceiver-Module-For-Arduino-ESP-12F-Adapter-Expansion-Board-For/32804504326.html) ). It contains built in 2way levelshifter/bi-directional logic level converter. So we can power and use via 5V uart from the 3d printers' motherboard.

* We need to manualy ground the ```IO0``` while powering up to start in flash mode while powering up (there is no switch for that, neither for reset)
  * ![wiring](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/ESP/ESP12.png)
* I used FTDI adapter as usb2serial
* We have to see in console/serial monitor boot mode is (*1*,7).
  * baudrate: 74880
  * ``` rst cause:2, boot mode:(3,7)```
* Then flash like other esp based board for esp3d
  * [check flash size](https://github.com/luc-github/ESP3D/wiki/Flash-Size). Mine has 4M
  * [Install](https://github.com/luc-github/ESP3D/wiki/Install-Instructions)


