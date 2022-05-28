# Hardware connection

If your motherboard doesn't support wifi, this repo will help you add an ESP32 board with an ESP8266 MCU. The ESP32 board supports wifi and will connect to your printer using a serial UART port thus acting as a wifi to UART bridge. The board needs to be programmed and connected properly to your printer to work. The printer motherboard will also need to be reprogrammed to enable the UART port that will be used with ESP32 board.

Connection between ESP and printer board needs 4 wires:

- ESP Tx needs to connect to Rx on MCU of printer board.
- ESP Rx needs to connect to Tx on MCU of printer board.  
- You also need to power supply ESP with with GND and 3V3 or 5V.

ESP8266 MCU supports only 3V3. Power supply it with 5V will fry it but note that some ESP32 boards have onboard regulators any might support 5V.

Although not 100% respecting datasheet of ESP8266 [the I/O seem to be 5V tolerant](https://ba0sh1.com/2016/08/03/is-esp8266-io-really-5v-tolerant/) so you can have your ESP Rx pin connected to a 5V supplied MCU. To be on the safe side having an 1-10k serial resistor between MCU Tx and ESP Rx will limit the current drawn by ESP. The extra clean way would be to have a good bidirectional level shiffter but this requires additional components and some electronics skills to choose correct parts.

## Connection diagrams for some printers and ESP boards

- [Printer motherboards](#Printer-motherboards)
  - [Anet A8 boards](#Anet-A8-boards)
  - [Anycubic i3 mega - Trigorilla 8bit board](#Anycubic-i3-mega---Trigorilla-8bit-board)
  - [AZSMZ LCD board](#AZSMZ-LCD-board)
  - [AZSMZ-mini board](#AZSMZ-mini-board)
  - [Azteeg X5 mini board](#Azteeg-X5-mini-board)
  - [BIQU KFB2.0 board](#BIQU-KFB2.0-board)
  - [Creality CR10 Ender 3 board](#Creality-CR10-Ender-3-board)
  - [Creality Ender 4 board](#Creality-Ender-4-board)
  - [Davinci 1.0/2.0 board](#Davinci-1.0/2.0-board)
  - [Davinci 1.0A board](#Davinci-1.0A-board)
  - [MKS boards](#MKS-boards)
  - [MKS Smoothieware board](#MKS-Smoothieware-board)
  - [RADDS board](#RADDS-board)
  - [RAMPS 1.4/Re-ARM board](#RAMPS-1.4/Re-ARM-board)
  - [Smoothieboard board](#Smoothieboard-board)
  - [Weedo Tina2 board](#Weedo-Tina2-board)
  - [For printer boards not listed here](#For-printer-boards-not-listed-here)
- [ESP32 board](#ESP32-board)
  - [ESP-01](#ESP-01)
  - [ESP-01 serial wifi module](#ESP-01-serial-wifi-module)
  - [ESP-12E/F](#ESP-12E/F)
  - [ESP 12F serial wifi module](#ESP-12F-serial-wifi-module)
  - [ESP32-Cam](#ESP32-Cam)
  - [NodeMCU V2/V3](#NodeMCU-V2/V3)
  - [Sonoff](#Sonoff)
  - [Wemos D1 mini](#Wemos-D1-mini)


## Printer motherboards

### Anet A8 boards

#### Anet boards up to v1.5

#### Step 1

You will also have to unsolder the resistors R52 and R53 – they are zero ohm resistors, and serve no other purpose than connecting the atmega chip directly to the onboard USB to UART converter (the CH340 chip). Do it VERY careful – you don’t want to damage your board. If you don’t feel confident – don’t do it.

![R52 and R53 position](http://lokspace.eu/wp-content/uploads/2017/01/image08-300x300.jpg)

#### Step 2

Now prepare the printer’s motherboard. It requires a simple modification, that does not interfere with it’s operation afterwards – just solder 3 pin x 2 row male header on J8, and add 2 jumpers (or jumper wires) as shown on the picture:

![Header and jumper position](http://lokspace.eu/wp-content/uploads/2017/01/image05-300x300.jpg)

#### Step 3

Connect the ESP to J3 repsecting pinout

![J3 connector pinout](http://lokspace.eu/wp-content/uploads/2017/01/image00-232x300.jpg)

|ESP|J3|
|:---:|:---:|
|Tx|Rx|
|Rx|Tx|
|GND|GND|
|VCC|3.3V|
|CH_PD|3.3V|

For more Info check <http://lokspace.eu/anet-a8-wifi-mod/>

#### For connecting version 1.7 Anet boards

Unlike older boards this board does not require you to remove any resistors.  
You will have to solder two wires from number 9 and number 10 its recommender to connect these to pin 1 and 2 of J3 connector.  
![ANET A8 v1.7 board](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Anet/board.jpg)

---

### Anycubic i3 mega - Trigorilla 8bit board

To connect the ESP12e to the UART0. (Credits:<https://www.lesimprimantes3d.fr/forum/profile/197-murdock/>).  
(Green = RX, Blue = TX)  
5V (buck to 3.3v if directly connect to ESP - most development ESP boards already have this voltage limited built-in - but check!) and GND can be taken from the AUX3 exposed connector.  
UART0 is normally used by USB port so don't use both together - so this hack piggybacks on that same port at UART level.  

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Trigorilla/board.jpg)

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Trigorilla/nodemcu.jpg)

---

### AZSMZ LCD board

![AZSMZ-12864-LCD board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/AZSMZ-mini/AZSMZ-12864-LCD.jpg)

---

### AZSMZ-mini board

![AZSMZ-mini board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/AZSMZ-mini/AZSMZ-mini.jpg)

If you don't have the soldering skills to grab the connectors from the unpopulated ethernet connection, you can also get 3.3v and GND from the ISP header (bottom left on the diagram above).

---

### Azteeg X5 mini board

![Azteeg X5 mini board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/AzteegX5-mini/azteeg.PNG)

---

### BIQU KFB2.0 board

all in one Ramps1.4/Mega2560 R3 controller based

![BIQU KFB2.0 board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/BIQU-KFB2.0/board.jpg)

---

### Creality CR10 Ender 3 board

For the Sanguino based CR-10 and Ender printers you will need to solder to any of the via circled (can also be done in the backside of board), or to the legs of the Arduino or ftdi. Connect TX from the board to RX of Wemos D1 mini and RX from board to TX of Wemos D1 mini. 5v and GND are located in the six pin header next to the LCD connector.

![CR10 board connection diagram using vias](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/CR10/board.jpg)

Since soldering might be difficult because the solder points are so close to each other, another option is to scrape off the insulation from the traces on the backside and solder there. Be extra careful not to scrape the surrounding ground plane. You need suitable fine scraping tools for this. The picture below shows an Ender-2 PCB.

![CR10 board connection diagram using copper trace](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/CR10/traces.jpg)

---

### Creality Ender 4 board

You will need to solder to small circle, or to the legs of the ATmega2560 (RXD0 pin 2, TXD0 pin 3)

![Ender 4 board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/ender4/board.jpg)

---

### Davinci 1.0/2.0 board

![Davinci board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/davinci.png)
![Davinci board connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/board.jpg)
![Davinci board placement](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/boardconnected.jpg)
![Davinci board placement](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/backside.jpg)
![Davinci screen with IP address](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/screen.jpg)

---

### Davinci 1.0A board

![NodeMCU V3 wiring](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/davinciA-1.jpg)
![Wiring on Davinci board](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/davinciA-4.jpg)
![Zoom on wiring on Davinci board](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/davinciA-2.jpg)

Alternate Module placement for increased WiFi range (outside metal chassis, antenna has vertical polarization)

![Davinci A3 external module placement](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Davinci/davinciA-3.jpg)

---

### MKS boards

To connect the ESP3D to the MKS GEN v1.2 (but the v1.3 and above 1.4 is the most used today).

I have used an ESP12E with the standard schematics, with one important difference, the two resistor connected to the RX pin are substituted by a 1N4148 diode, like in the Adafruit Huzzah board.
![ESP12E wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/MKS-1.2/wires.png)

ESP12E is connected to the AUX1

ESP12E RX is connected to the pin NEAR GND of the upper row (Marked TXD on pinout.)  
ESP12E TX is connected to the adiacent pin at the end of the upper row (Marked RXD on pinout.)

![MKS wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/MKS-1.2/board.png)

---

### MKS Smoothieware board

![MKS Smoothieware wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/MKS-SMOOTHIEWARE/MKS-smoothie.png)

---

### RADDS board

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/RADDS/RADDS.png)
![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/RADDS/screen.jpg)

---

### RAMPS 1.4/Re-ARM board

Ramps 1.4 can be used on Arduino Mega (repetier/marlin) and Re-ARM for ramps boards (smoothieware/marlin)
![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/RAMPS1.4/RAMPS.PNG)

Alternative pins if use Re-ARM (J4/UART port)

![image](https://i.ibb.co/cDMKGbK/Screenshot-20190803-022151.png)

---

### Smoothieboard board

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Smoothieware/smoothieboard-wiring.png)

---

### Weedo Tina2 board

This printer is also brand labelled as **Monoprice MP cadet 3D printer**

![Weedo Tina2 connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/TINA2/weedo_tina2.jpg)

In marlin this connection is **serial port 3**.

Note the Mega2560 is 5V powered and ESP is 3V3 powered.  

---

### For printer boards not listed here

Vast majority of printers have an USB port that is converted to UART before going to MCU. Many printers also have additional (unused) UART port you can use. When possible, always use the additional port for connecting ESP to printer board. When no additional UART port is available you might use the Tx and Rx lines between USB/UART converter and MCU but it's recommended to cut (in a reversible way) the line to USB/UART converter to avoid conflicts.

If the board is ATmega based the simplest way to find a usable UART port for the ESP is to open ATmega datasheet.

## ESP32 board

### ESP-01

- Use GPIO2 to ground to reset all settings in hard way - 2-6 sec after boot / not before!! Set GPIO2 to ground before boot change boot mode and go to special boot that do not reach FW. Currently boot take 10 sec - giving 8 seconds to connect GPIO2 to GND and do an hard recovery for settings   
- Use GPIO0 to ground to be in update mode 

![wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/HW/Wires.png)

---

### ESP-01 serial wifi module

![module picture](https://www.keyestudio.com/u_file/1901/products/11/ff587ce89a.jpg)

more info about the Breakout PCB: <https://www.keyestudio.com/keyestudio-esp-01s-wifi-to-serial-shield-module-for-arduino-esp8266-wifi-p0499-p0499.html>

---

### ESP-12E/F

ESP need 3.3v, it is not 5v tolerant, if printer board use more than 3.3V like 5V on ramps.  
![wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/HW/WiresESP12E.png)

you can also use Logic LevelConverter Bi-Directional

![picture](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/HW/logic.PNG)

In order to flash some ESP12E/F boards via their UART interface, the following pins need to be connected:

- VCC to GPIO2
- GND to GPIO0

This has been tested with ESP-12-E boards labeled "ESP8266 For ESP3D FYSETC.COM"

---

### ESP 12F serial wifi module

We can flash our loved ESP3D to cheap ESP-12F based serial wifi module (eg [from aliexpress](https://www.aliexpress.com/item/ESP8266-ESP-12F-Serial-WIFI-Wireless-Transceiver-Module-For-Arduino-ESP-12F-Adapter-Expansion-Board-For/32804504326.html) ). It contains built in 2-way levelshifter/bi-directional logic level converter. So we can power and use via 5V uart from the 3d printers' motherboard.

- We need to manualy ground the ```IO0``` while powering up to start in flash mode while powering up (there is no switch for that, neither for reset)
  - ![wiring](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/ESP/ESP12.png)
- I used FTDI adapter as usb2serial
- We have to see in console/serial monitor boot mode is (**1**,7).
  - baudrate: 74880
  - ```rst cause:2, boot mode:(3,7)```
- Then flash like other esp based board for esp3d
  - [check flash size](https://github.com/luc-github/ESP3D/wiki/Flash-Size). Mine has 4M
  - [Install](https://github.com/luc-github/ESP3D/wiki/Install-Instructions)

---

### ESP32-Cam

Once the board is programmed, the wiring to the printer board should be like this:
![ESP32 CAM connection diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/ESP/ESPcam32.png)

Note: this is not the only way to connect the board.

- other values for the resistors could be used (always in the ratio 1:2)
- could use a logic level coverter 5V - 3.3V (see [D1-mini wiki page](https://github.com/luc-github/ESP3D/wiki/D1-mini) )

---

### NodeMCU V2/V3

![NodeMCU wiring diagram](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/NodeMCU/NodeMCU.PNG)

---

### Sonoff

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Sonoff/Sonoff.png)

Relay is connected by GPIO12, it can be handled using ESP201 command:

    *Get/Set pin value
    [ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]
    if no V<value> get P<pin> value
    if V<value> 0/1 set INPUT_PULLUP value, but for GPIO16 INPUT_PULLDOWN_16
    GPIO1 and GPIO3 cannot be used as they are used for serial
    if PULLUP=YES set input pull up, if not set input
    if RAW=YES do not set pinmode just read value

So `[ESP201]P12 V0` should be off and `[ESP201]P12 V1` should be on

---

### Wemos D1 mini

![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/D1_mini/FB_IMG_1510306696875.jpg)
![image](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/D1_mini/20171111_215253.jpg)
