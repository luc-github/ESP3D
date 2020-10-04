The ESP8266 comes in various models:

configure your Arduino IDE -> Tools -> Boards as:

# Known working configs

### The latest ESP01 and ESP12Es come with 4Mb of  flash:  For those 
* Board: Generic ESP8266 Module
* Flash Mode: DIO
* Flash Frequency: 40Mhz
* Flash Size:  4M (3M SPIFFS)
* Debug Port: Disabled
* Debug Level: None
* Reset Method: CK
* Upload Speed:  115200

### Some of the older devices come with 1M flash
* Board: Generic ESP8266 Module
* Flash Mode: DIO
* Flash Frequency: 40Mhz
* Flash Size:  1M (128K SPIFFS)
* Debug Port: Disabled
* Debug Level: None
* Reset Method: CK
* Upload Speed:  115200

### Though now no longer supported, it is possible to run the firmware on devices like the ESP07 with 512K of flash:
* Board: Generic ESP8266 Module
* Flash Mode: DIO
* Flash Frequency: 40Mhz
* Flash Size:  512k (128K SPIFFS)
* Debug Port: Disabled
* Debug Level: None
* Reset Method: CK
* Upload Speed:  115200

# Figuring out the Flash Size
If you are unsure how much flash memory your particular module has. you can figure it out from the Arduino IDE:

1. Open the Arduino IDE
2.  Click File, Examples, ESP8266, CheckFlashConfig
3.  Upload the sketch to the ESP8266
4.  View the Serial Monitor (115200 baud)
5.  This compares what you have in Tools -> Board -> Flash Size to what is actually on the board...

For example
`Flash real id:   001340C8`

`Flash real size: 524288`



`Flash ide  size: 524288`

`Flash ide speed: 40000000`

`Flash ide mode:  DIO`

`Flash Chip configuration ok.`

(NB:  If you dont get a 'Flash Chip configuration ok.' uploading will appear to work succesfully but the chip will crash on startup and never show an access point / serial output)