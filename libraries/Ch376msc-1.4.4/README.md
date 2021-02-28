# Arduino library for CH376 / CH375 file manager control chip
Supports read/write files to USB flash drive or SD card (CH375 only support USB flash drive).
>Why use this chip if there is already a library to handle the SD card and it is easier to just hook up the SD card(with resistors or SD card module) to Arduino?
>The SD library is widely used and is reliable, the only problem is the Arduino does't have to much memory and with the SD lib the MCU has to cope with the FAT file system,
>and we're just talking about SD card management, the USB storage drive handling is a more complicated and memory consuming procedure and you need a USB-HOST chip.
>The CH376 chip easily can write and read files even if they are on SD card or on Usb thumb-drive(CH375 only support USB thumb-drive). The chip supports FAT12, FAT16 and FAT32 file systems, meaning the chip does the hard work, 
>the MCU does not have to deal with the FAT file system, it only sends instructions to the chip on the communication bus you like (SPI, UART (HW serial, SW serial)), and the magic happens in the chip.
>The chip can do more, e.g to handle HID devices(usb keyboard, mouse, joystick ...) but this feature is not yet used in the library, maybe in the future.

Click [here](https://github.com/djuseeq/Ch376msc#test) to see the difference between libraries about memory usage.


## Getting Started

Configure the jumpers on the module depending on which communication protocol you are using(see [API reference](https://github.com/djuseeq/Ch376msc#api-reference))
![Alt text](extras/JumperSelect.png?raw=true "Setting")

 ### PCB modding for SD card
 > If you planning to use the chip for SD card also and you have a pcb like on the photo above, then some soldering skill is required.
 > First of all with a DMM check the pins of the chip(26,25,23 and 7) are they floating or connected to GND/VCC.
 > On mine pcb the chip pin 23 (SD_CS) is connected to ground, like you can [see here](extras/schematic.png), 
 > pins or the chip have incorrect marking(looks like CH375 which one doesn't support SD card) . [Link](https://www.mpja.com/download/31813MPSch.pdf) for the module's schematic diagram. 
 > I used soldering iron and tweezer to lift up the pin from the pcb(be careful, you can easily break the chip's leg).
 > Follow [this schema](extras/modPcb.png) to make the proper connection between the chip and SD card socket.
 > I used a [SD card adapter](extras/sdAdapter.jpg) and for sake of stability, use the capacitors+1R resistor on Vcc line.
 > The SD card operate from 3.3V and this board already have a 3.3V voltage regulator so that is fine.
 > Here are some photos from the ugly modding ;) [Photo1](extras/board1.jpg) [Photo2](extras/board2.jpg).

## Versions
v1.4.4 Sep 29, 2020
  - error handling improvement
  - new function, getChipVer()
  - bug fix, issue #34 Variable Naming conflicts with core ESP32 Variables
  
v1.4.3 Feb 06, 2020
  - bug fix issue #22 unknown partition
  - new functions as requested in #21 , #23
  - reorganizing the library
  - added function-like macros to easy configure the SPI clock rate(see in examples/lcd_menu)
  
v1.4.2 Jan 07, 2020
 > - support SD card manage(API ref. - setSource(),if the SD card socket is not available on the module,
 > then modification on the module is required, please read [Pcb modding for sd card](https://github.com/djuseeq/Ch376msc#pcb-modding-for-sd-card) section)
 > - a new example of using an SD card
 > - the checkDrive function name was misleading, renamed to checkIntMessage
 > - improvements, bug fixes
 > - unnecessary examples removed
  
v1.4.1 Dec 22, 2019 
  - supports more architectures(see Tested boards table below) - issue #11
  - constructor update (BUSY pin is not longer used)
  - improved logic to the mount/unmount flash drive
  - directory support ( cd(); function )
  - use advanced file listing with (*) wildcard character(API reference, listDir() function)

v1.4.0 Sep 26, 2019 
  - new functions
     - getTotalSectors() - returns a unsigned long number, total sectors on the drive
     - getFreeSectors() - returns a unsigned long number, free sectors on the drive
     - getFileSystem() - returns a byte number, 0x01-FAT12, 0x02-FAT16, 0x03-FAT32
  - updated example files with a new functions
  - new example file, searching for the oldest/newest file on the flash drive

v1.3.1 Sep 20, 2019 
  - rearrange the folder structure to be 1.5 library format compatible

v1.3 Sep 17, 2019 
  - bug fix for moveCursor issue #3 , minor changes

v1.2.1 Apr 24, 2019 
  - In use of SPI, CS pin on the module must to be pulled to VCC otherwise communication can be instable on a higher clock rate
  - bug fix for timing issue on a higher clock rate (TSC)
                  
v1.2 Apr 20, 2019 
  - extended with SPI communication

v1.1 Feb 25, 2019 
  - initial version with UART communication

## API Reference
```C++
//The default SPI communication speed is reduced to 125 kHz because of stability if long cables or breadboard is used. 
// to change the SPI Clock rate, during instantiation use e.g. SPI_SCK_KHZ(500) - to use 500kHz
//                                                     or e.g. SPI_SCK_MHZ(8) - to use 8MHz (see in examples/lcd_menu)
	//CONSTRUCTORS
	   //UART
	 //For hardware serial leave the communication settings on the module at default speed (9600bps) 
	Ch376msc(HardwareSerial, speed);//Select the serial port to which the module is connected and the desired speed(9600, 19200, 57600, 115200)
	
	 //For software serial select the desired communication speed on the module(look on the picture above)
	Ch376msc(SoftwareSerial);
	
	   //SPI
	 //If no other device is connected to the SPI port it`s possible to save one MCU pin
	Ch376msc(spiSelect, *optional SPI CLK rate*);// ! Don`t use this if the SPI port is shared with other devices
	
	 //If the SPI port is shared with other devices, use this constructor and one extra MCU pin need to be sacrificed for the INT pin
	Ch376msc(spiSelect, interruptPin, *optional SPI CLK rate*);
	////////////////////
	
	 // Must be initialized before any other command are called from this class.
	init();
	
	 // call frequently to get any interrupt message of the module(attach/detach drive)
	checkIntMessage(); //return TRUE if an interrupt request has been received, FALSE if not.
	
	 // can call before any file operation
	driveReady(); //returns FALSE if no drive is present or TRUE if drive is attached and ready.
	
	 // check the communication between MCU and the CH376
	pingDevice(); //returns FALSE if there is a communication failure, TRUE if communication  is ok
	
	 // 8.3 filename, also called a short filename is accepted 
	setFileName(filename);//8 char long name + 3 char long extension
	
	 // open file before any file operation. Use first setFileName() function
	openFile();
	
	 // always call this after finishing with file operations otherwise data loss or file corruption may occur
	closeFile();
	
	 // repeatedly call this function to read data to buffer until the return value is TRUE
	readFile(buffer, length);// buffer - char array, buffer size
	
	 // Read text until reach the terminator character, rest is same as readFile
	readFileUntil(terminator, buffer, length);//returns boolean true if the given buffer
	                                    //      is full and not reached the terminator character
	      
	 //Same as readFile except the buffer type is byte(uint8) array and not added terminating 0 char
	readRaw(buffer, length);// buffer - byte array, buffer size
	
	 //Read, extract numbers of txt file, read until reach EOF (see getEOF())
	readLong(terminator);//returns long value,terminator char is optional, default char is '\n'
	readULong(terminator);//returns unsigned long value,terminator char is optional, default char is '\n'
	readDouble(terminator);//returns double value,terminator char is optional, default char is '\n'
	
	 //Write, construct string of number and write on the storage(byte, int, u int, long, u long, double)
	writeNum(number);// write the given number
	writeNumln(number);// write the given number in new line
	    
	 //Write one character on the storage
	writeChar(char);// e.g. new line character '\n' or comma ',' to 
	
	 // repeatedly call this function to write data to the drive until there is no more data for write or the return value is FALSE
	writeFile(buffer, length);// buffer - char array, string size in the buffer
	
	 // switch between source drive's, 0 = USB(default), 1 = SD card
	 // !!Before calling this function and activate the SD card please do the required modification 
	 // on the pcb, please read **PCB modding for SD card** section otherwise you can damage the CH376 chip.
	setSource(srcDrive);// 0 or 1
	
	setYear(year); // 1980 - 2099
	setMonth(month);// 1 - 12
	setDay(day);// 1 - 31
	setHour(hour);// 0 - 23
	setMinute(minute);// 0 - 59
	setSecond(second);// 0 - 59 saved with 2 second resolution (0, 2, 4 ... 58)
	
	 // when new file is created the defult file creation date/time is (2004-1-1 0.0.0), 
	 // it is possible to change date/time with this function, use first set functions above to set the file attributes
	saveFileAttrb();
	
	 // move the file cursor to specified position
	moveCursor(position);// 00000000h - FFFFFFFFh
	
	 // delete the specified file, use first setFileName() function
	deleteFile();
	
	 // repeatedly call this function with getFileName until the return value is TRUE to get the file names from the current directory
	 // limited possibility to use with wildcard character e.g. listDir("AB*") will list files with names starting with AB
	 // listDir("*AB") will not work, wildcard char+string must to be less than 8 character long
	 // if no argument is passed while calling listDir(), all files will be printed from the current directory
	listDir();// returns FALSE if no more file is in the current directory
	
	 // reset file process state machine to default
	 // useful e.g. to make LCD menu with file's list without using large buffer to store the file names
	resetFileList();
	 
	 //dirPath = e.g. "/DIR1/DIR2/DIR3" , "/" - root dir
	 //CreateDir = 0(open directories if they not exist, don`t create them) or 1(create directories if they do not exist and open them)
	 //if working in subfolders, before file operations ALWAYS call this function with the full directory path
	 //limited to 3 subfolders depth (see /src/Ch376msc.h file. MAXDIRDEPTH) and 8 character long directory names
	cd(dirPath,CreateDir);// returns byte value,see example .ino
	
	getFreeSectors();// returns unsigned long value
	getTotalSectors();// returns unsigned long value
	getFileSize();// returns unsigned long value (byte)
	getSource();// returns boolean value, false USB, true SD card
	
	getYear();// returns int value
	getMonth();// returns int value
	getDay();// returns int value
	getHour();// returns int value
	getMinute();// returns int value
	getSecond();// returns int value
	
	 // get the last error code (see datasheet and/or CommDef.h)
	getError();// returns byte value
	
	getFileSystem();// returns byte value, 01h-FAT12, 02h-FAT16, 03h-FAT32
	getFileName();// returns the file name in a 11+1 character long string value
	getFileSizeStr();// returns file size in a formatted 9+1 character long string value
	getFileAttrb();// returns byte value, see /src/CommDef.h , (File attributes)
	getCursorPos();// returns unsigned long value
	getEOF();// returns boolean value, true EOF is reached
	getChipVer();// returns byte value, returns the CH chip firmware version number
```
## Tested boards
|Board(arch) | SPI | HW Serial | SW Serial|
|:---|:---:|:---:|:---:|
|Arduino (AVR)|OK|OK|OK|
|DUE (SAM)|OK(with INT pin)|OK|NO|
|ZERO (SAMD)|OK|?|NO|
|*STM32 cores|OK|!NO|NO|
|**STM32duino|OK|OK|NO|
|***ESP8266|OK(with INT pin)|NO|OK|
|ESP32 ([ref](https://github.com/djuseeq/Ch376msc/issues/18))|OK|?|?|

Be careful when choosing SoftSerial because it has its own limitations. See [issues#15](https://github.com/djuseeq/Ch376msc/issues/15)

> `*` Tested on NUCLEO F446RE(no signal at all on UART ports)

> `**` Tested on Generic STM32F103C alias Blue pill with STM32duino bootloader

> `***` Tested on NodeMCU,(i'm not familiar with ESP MCUs) it looks they have default enabled WDT so i have to call
>	`yield()` periodically during file operations, otherwise ESP will restart with a ugly message.
>	Working SPI configuration (for me)is MISO-12(D6), MOSI-13(D7), SCK-14(D5), CS-4(D2), INT-5(D1)

### Test

I compared the two libraries with the same instructions, create file, write some text in it and read back the created file and send it to serial (SPI used)
Used Arduino IDE 1.8.10 on x64 linux, ArduinoUno board choosed

Sketch from SD library(SparkFun 1.2.4) ReadWrite example:

Program space used: 10704 bytes 33% ,

SRAM used: 882 bytes 43%

```C++
#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  Serial.begin(9600);
  SD.begin(4);
  myFile = SD.open("TEST.TXT", FILE_WRITE);
  if (myFile) {
    myFile.println("testing 1, 2, 3.");
    myFile.close();
  }
  myFile = SD.open("TEST.TXT");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
}

void loop() {}

```

Second sketch is with Ch376msc library(1.4.2)

1. if i put in comments the setSorce function and use the default USB storage

    Program space used: 6760 bytes 20% ,
    SRAM used: 315 bytes 15%
2. with setSorce function choosed USB storage

    Program space used: 6810 bytes 21% ,
    SRAM used: 315 bytes 15%
3. with setSorce function choosed SD storage

    Program space used: 6824 bytes 21% ,
    SRAM used: 315 bytes 15%

```C++
#include <Ch376msc.h>

Ch376msc flashDrive(10); // chipSelect
char adat[]={"testing 1, 2, 3."};
boolean readMore = true;

void setup() {
  Serial.begin(9600);
  flashDrive.init();
  flashDrive.setSource(0);//0 - USB, 1 - SD
  flashDrive.setFileName("TEST.TXT");
  flashDrive.openFile();
  flashDrive.writeFile(adat, strlen(adat));
  flashDrive.closeFile(); 
  flashDrive.setFileName("TEST.TXT");
  flashDrive.openFile();
  while(readMore){
     readMore = flashDrive.readFile(adat, sizeof(adat));
     Serial.print(adat);
  }
  flashDrive.closeFile();
}

void loop() {}
```

### Acknowledgments

Thanks for the idea to [Scott C](https://arduinobasics.blogspot.com/2015/05/ch376s-usb-readwrite-module.html)

## License
The MIT License (MIT)

Copyright (c) 2019 György Kovács

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

