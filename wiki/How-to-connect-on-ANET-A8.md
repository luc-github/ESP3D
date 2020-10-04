# Connecting Anet A8 to ESP Boards
## This is for versions ≤1.5
## Preparation.
### Step 1
You will also have to unsolder the resistors R52 and R53 – they are zero ohm resistors, and serve no other purpose than connecting the atmega chip directly to the onboard USB to UART converter (the CH340 chip). Do it VERY careful – you don’t want to damage your board. If you don’t feel confident – don’t do it.

![](http://lokspace.eu/wp-content/uploads/2017/01/image08-300x300.jpg)
### Step 2 
Now prepare the printer’s motherboard. It requires a simple modification, that does not interfere with it’s operation afterwards – just solder 3 pin x 2 row male header on J8, and add 2 jumpers (or jumper wires) as shown on the picture:

![](http://lokspace.eu/wp-content/uploads/2017/01/image05-300x300.jpg)
###  Step 3
Connect the ESP to J3 Pinout

![](http://lokspace.eu/wp-content/uploads/2017/01/image00-232x300.jpg)
`<table>
  <tr>
    <th>ESP</th>
    <th>J3</th>
   
  </tr>
  <tr>
    <td>TX</td>
    <td>RX</td>
   
  </tr>
  <tr>
    <td>RX</td>
    <td>TX</td>
   
  </tr>
  <tr>
    <td>GND</td>
    <td>GND</td>
    
  </tr>
  <tr>
    <td>VCC</td>
    <td>3.3V</td>

  </tr>
  <tr>
    <td>CH_PD</td>
    <td>3.3V</td>

  </tr>


  </tr>
</table>
For more Info check http://lokspace.eu/anet-a8-wifi-mod/

# For connecting version 1.7 Anet boards
Unlike older boards this board does not require you to remove any resistors. 
You will have to solder two wires from number 9 and number 10 its recommender to connect these to pin 1 and 2 of J3 connector. ![board](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Anet/board.jpg)
 
