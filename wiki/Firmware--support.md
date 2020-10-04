## References

FW on Board |  GCODE 
------------ | -------------  
Repetier | https://github.com/repetier/Repetier-Firmware/blob/master/src/ArduinoDUE/Repetier/Repetier.ino#L39-L151 
Repetier for Davinci | https://github.com/luc-github/Repetier-Firmware-0.92/blob/master/src/ArduinoDUE/Repetier/Repetier.ino#L39-L144 
Marlin | http://marlinfw.org/meta/gcode/
Marlinkimbra |https://github.com/MagoKimbra/MarlinKimbra/blob/V4_2_9/Documentation/GCodes.md 
Smoothieware | http://smoothieware.org/supported-g-codes 
GRBL | https://github.com/gnea/grbl/wiki/Grbl-v1.1-Commands 
Reprap | https://duet3d.dozuki.com/Wiki/Gcode   

***

## Temperature query

FW on Board |  GCODE | Answer | Note | Supported ? 
------------ | ------------- | ------------- | ------------- | ------------- 
Repetier |  M105 | T:24.59 / 0 B:29.17 / 0 B@:0 @:0 T0:24.59 / 0 @0:0 T1:25.68 / 0 @1:0   | T and T0 are E0, T1 is E1, B is bed | Yes 
Marlin | M105 | ok T:25.8 /0.0 B:26.1 /0.0 T0:25.8 /0.0 T1:25.5 /0.0 @:0 B@:0 @0:0 @1:0 |T and T0 are E0, T1 is E1, B is bed | Yes
MarlinKimbra | M105 | ok T:26.4 /0.0 B:26.3 /0.0 T0:26.4 /0.0 T1:26.3 /0.0 @:0 B@:0 @0:0 @1:0 |T and T0 are E0, T1 is E1, B is bed | Yes
Smoothieware | M105 | ok T:25.6 /0.0 @0 T1:24.5 /0.0 @0 B:25.7 /0.0 @0 | T is E0, T1 is E1, B is bed | Yes
GRBL| N/A | | | N/A 
Reprap | M105 | T:26.5 /0.0 B:24.8 /0.0 | | TBA 

***
## Position query

FW on Board |  GCODE | Answer | Note | Supported ? 
------------ | ------------- | ------------- | ------------- | ------------- 
Repetier |  M114| X:0.00 Y:0.00 Z:0.000 E:0.0000 | | Yes
Marlin | M114| X:0.00 Y:0.00 Z:0.00 E:0.00 Count X: 0 Y:0 Z:0 | | Yes
MarlinKimbra | M114| X:0.00 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0 | | Yes
Smoothieware | M114| ok C: X:0.0000 Y:0.0000 Z:0.0000 E:0.000 |  | Yes
GRBL| ?| &lt;Idle&#124;MPos:10.000,0.000,0.000&#124;FS:0,0 &#124;Ov:71,100,147&gt; |  | Yes
Reprap | M114 | X:0.000 Y:0.000 Z:0.000 E0:0.0 E1:0.0 E2:0.0  Count 0 0 0 User 0.0 0.0 0.0 | | TBA

***

## SD Card file list

FW on Board |  GCODE | Answer | Note | Supported ? 
------------ | ------------- | ------------- | ------------- | ------------- 
Repetier |  M20 | Begin file list<br>sample1.g 599<br>MYFOLDER/<br>End file list<br> | filename and size, folder name end with / | Yes
Marlin | M20 | Begin file list<br>`CURA~1.GCO` <br>`/MYFOLDER/CURA~1.GCO` <br>End file list | only filename, folder name start with / | Yes
MarlinKimbra | M20 | Begin file list<br>cura.gcode<br>MYFOLDER/<br>MYFOLDER/cura.gcode<br>End file list | only filename, folder name end with / | Yes
Smoothieware | M20 | Begin file list<br>myfolder/<br>config.txt<br>End file list | only filename, folder name end with / | Yes
GRBL| N/A|  |  | N/A
Reprap | M20 S2 P/ | {"dir":"/","files":["*System Volume Information","*sys","AxholderV5.gcode","*folder1","*New folder","New Text Document.txt","test.g","test - Copy.g","*folder1 - Copy","license.txt"],"err":0} | folder start with *, JSON format | TBA 

***