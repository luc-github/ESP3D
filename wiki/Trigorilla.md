# Where to connect ESP on Anycubic i3 mega - Trigorilla 8bit board   
To connect the ESP12e to the UART0. (Credits:https://www.lesimprimantes3d.fr/forum/profile/197-murdock/).    
(Green = RX, Blue = TX)    
5V (buck to 3.3v if directly connect to ESP - most development ESP boards already have this voltage limited built-in - but check!) and GND can be taken from the AUX3 exposed connector.    
UART0 is normally used by USB port so don't use both together - so this hack piggybacks on that same port at UART level.    

![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Trigorilla/board.jpg)

![](https://raw.githubusercontent.com/wiki/luc-github/ESP3D/images/Trigorilla/nodemcu.jpg)
