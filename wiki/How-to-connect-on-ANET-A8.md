# Connecting Anet A8 to ESP Boards

## Anet boards up to v1.5

### Step 1

You will also have to unsolder the resistors R52 and R53 – they are zero ohm resistors, and serve no other purpose than connecting the atmega chip directly to the onboard USB to UART converter (the CH340 chip). Do it VERY careful – you don’t want to damage your board. If you don’t feel confident – don’t do it.

![R52 and R53 position](http://lokspace.eu/wp-content/uploads/2017/01/image08-300x300.jpg)

### Step 2

Now prepare the printer’s motherboard. It requires a simple modification, that does not interfere with it’s operation afterwards – just solder 3 pin x 2 row male header on J8, and add 2 jumpers (or jumper wires) as shown on the picture:

![Header and jumper position](http://lokspace.eu/wp-content/uploads/2017/01/image05-300x300.jpg)

### Step 3

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

## For connecting version 1.7 Anet boards

Unlike older boards this board does not require you to remove any resistors.  
You will have to solder two wires from number 9 and number 10 its recommender to connect these to pin 1 and 2 of J3 connector.  
![ANET A8 v1.7 board](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Anet/board.jpg)
