# Where to connect ESP on Anycubic i3 mega - Trigorilla 8bit board   
To connect the ESP12e to the UART0. (Credits:https://www.lesimprimantes3d.fr/forum/profile/197-murdock/).    
(Green = RX, Blue = TX)    
5V (buck to 3.3v if directly connect to ESP - most development ESP boards already have this voltage limited built-in - but check!) and GND can be taken from the AUX3 exposed connector.    
UART0 is normally used by USB port so don't use both together - so this hack piggybacks on that same port at UART level.    

![](https://user-images.githubusercontent.com/46300801/50600783-8c295f80-0eb2-11e9-8ee2-0536256125e9.JPG)

![](https://user-images.githubusercontent.com/46300801/50600853-bbd86780-0eb2-11e9-93eb-5680ab05b3c8.JPG)