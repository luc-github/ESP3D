To connect the ESP3D to the MKS GEN v1.2 (but the v1.3 and above 1.4 is the most used today)

I have used and ESP12E with the standard schematics, with one important difference, the two resistor connected to the RX pin are substituted by a 1N4148 diode, like in the Adafruit Huzzah board.
![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/MKS-1.2/wires.png)

ESP12E is connected to the AUX1 

ESP12E RX is connected to the pin NEAR GND of the upper row (Marked TXD on pinout.)  
ESP12E TX is connected to the adiacent pin at the end of the upper row (Marked RXD on pinout.)

![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/MKS-1.2/board.png) 
