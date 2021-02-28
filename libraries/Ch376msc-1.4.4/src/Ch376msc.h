/*
 * Ch376msc.h
 *
 *  Created on: Feb 25, 2019
 *      Author: György Kovács
 *  Copyright (c) 2019, György Kovács
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 ******************************************************
 * Versions:                                          *
 * ****************************************************
 * v1.4.4 Sep 29, 2020
 * - error handling improvement
 * - new function, getChipVer()
 * - bug fix, issue #34 Variable Naming conflicts with core ESP32 Variables
 *
 * v1.4.3 Feb 06, 2020
 * - bug fix issue #22 unknown partition
 * - new functions as requested in #21 , #23
 * - reorganizing the library
 * - added function-like macros to easy configure the SPI clock rate
 *
 * v1.4.2 Jan 07, 2020
 * - support SD card manage(API ref. - setSource())
 * - a new example of using an SD card
 * - the checkDrive function name was misleading, renamed to checkIntMessage
 * - improvements, bug fixes
 * - unnecessary examples removed
 * ****************************************************
 * v1.4.1 Dec 22, 2019
 * - supports other architectures
 * - constructor update (skip use BUSY pin)
 * - improved logic to the mount/unmount flash drive
 * - directory support ( cd(); function )
 * - advanced file listing with (*) wildcard character(API reference, listDir() function)
 *
 ******************************************************    
 * v1.4.0 Sep 26, 2019 
 * 	- new functions
 *   	getTotalSectors() - returns a unsigned long number, total sectors on the drive
 *   	getFreeSectors() - returns a unsigned long number, free sectors on the drive
 *   	getFileSystem() - returns a byte number, 0x01-FAT12, 0x02-FAT16, 0x03-FAT32
 * 	- updated example files with a new functions
 * 	- new example file, searching the oldest/newest file on the flash drive
 * **************************************************** 
 * 	v1.3 Sep 17, 2019
 * 		-bug fix for moveCursor issue #3  
 *	https://github.com/djuseeq/Ch376msc/issues/3
 * ****************************************************
 *  v1.2 Apr 24, 2019
 *  	-bug fix for timing issue on higher SPI clock
 *  	 datasheet 7.3 Time Sequence table (TSC)
 ******************************************************
 *  v1.2 Apr 20, 2019
 *  	-extended with SPI communication
 ******************************************************
 *	v1.1 Feb 25, 2019
 *		-initial version with UART communication
 ******************************************************
 */

#ifndef Ch376msc_H_
#define Ch376msc_H_

#include <Arduino.h>
#include "CommDef.h"
#include <Stream.h>
#include <SPI.h>

#if defined(__STM32F1__)
	#include "itoa.h"
#endif
#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
	#include "avr/dtostrf.h"
#endif

#define ANSWTIMEOUT 1000 // waiting for data from CH
#define MAXDIRDEPTH 3 // 3 = /subdir1/subdir2/subdir3

class Ch376msc {
public:
	/////////////Constructors////////////////////////
	Ch376msc(HardwareSerial &usb, uint32_t speed);//HW serial
	Ch376msc(Stream &sUsb);// SW serial

	//Ch376msc(uint8_t spiSelect, uint8_t intPin, uint32_t speed = SPI_SCK_KHZ(125));
	//Ch376msc(uint8_t spiSelect, uint32_t speed = SPI_SCK_KHZ(125));//SPI with MISO as Interrupt pin
	Ch376msc(uint8_t spiSelect, uint8_t intPin, SPISettings speed = SPI_SCK_KHZ(125));
	Ch376msc(uint8_t spiSelect, SPISettings speed = SPI_SCK_KHZ(125));//SPI with MISO as Interrupt pin

	virtual ~Ch376msc();//destructor
	////////////////////////////////////////////////
	void init();

	uint8_t saveFileAttrb();
	uint8_t openFile();
	uint8_t closeFile();
	uint8_t moveCursor(uint32_t position);
	uint8_t deleteFile();
	uint8_t pingDevice();
	uint8_t listDir(const char* filename = "*");
	uint8_t readFile(char* buffer, uint8_t b_size);
	uint8_t readRaw(uint8_t* buffer, uint8_t b_size);
	int32_t readLong(char trmChar = '\n');
	uint32_t readULong(char trmChar = '\n');
	double readDouble(char trmChar = '\n');
	uint8_t writeChar(char trmChar);
	uint8_t writeFile(char* buffer, uint8_t b_size);
	uint8_t writeRaw(uint8_t* buffer, uint8_t b_size);
	uint8_t writeNum(uint8_t buffer);
	uint8_t writeNum(int8_t buffer);
	uint8_t writeNum(uint16_t buffer);
	uint8_t writeNum(int16_t buffer);
	uint8_t writeNum(uint32_t buffer);
	uint8_t writeNum(int32_t buffer);
	uint8_t writeNum(double buffer);

	uint8_t writeNumln(uint8_t buffer);
	uint8_t writeNumln(int8_t buffer);
	uint8_t writeNumln(uint16_t buffer);
	uint8_t writeNumln(int16_t buffer);
	uint8_t writeNumln(uint32_t buffer);
	uint8_t writeNumln(int32_t buffer);
	uint8_t writeNumln(double buffer);
	uint8_t cd(const char* dirPath, bool mkDir);
	bool readFileUntil(char trmChar, char* buffer, uint8_t b_size);
	bool checkIntMessage(); // check is it any interrupt message came from CH(drive attach/detach)
	bool driveReady(); // call before file operation to check thumb drive or SD card are present
	void resetFileList();

//set/get

	uint32_t getFreeSectors();
	uint32_t getTotalSectors();
	uint32_t getFileSize();
	uint32_t getCursorPos();
	uint16_t getYear();
	uint16_t getMonth();
	uint16_t getDay();
	uint16_t getHour();
	uint16_t getMinute();
	uint16_t getSecond();
	uint8_t getStatus();
	uint8_t getFileSystem();
	uint8_t getFileAttrb();
	uint8_t getSource();
	uint8_t getError();
	uint8_t getChipVer();
	char* getFileName();
	char* getFileSizeStr();
	bool getDeviceStatus(); // usb device mounted, unmounted
	bool getCHpresence();
	bool getEOF();

	void setFileName(const char* filename = "");
	void setYear(uint16_t year);
	void setMonth(uint16_t month);
	void setDay(uint16_t day);
	void setHour(uint16_t hour);
	void setMinute(uint16_t minute);
	void setSecond(uint16_t second);
	void setSource(uint8_t inpSource);

private:

	void write(uint8_t data);
	void print(const char str[]);
	void spiBeginTransfer();
	void spiEndTransfer();
	void driveAttach();
	void driveDetach();

	uint8_t spiWaitInterrupt();
	uint8_t spiReadData();
	uint8_t mount();
	uint8_t getInterrupt();
	uint8_t fileEnumGo();
	uint8_t byteRdGo();
	uint8_t fileCreate();
	uint8_t byteWrGo();
	uint8_t reqByteRead(uint8_t a);
	uint8_t reqByteWrite(uint8_t a);
	uint8_t readSerDataUSB();
	uint8_t writeMachine(uint8_t* buffer, uint8_t b_size);
	uint8_t writeDataFromBuff(uint8_t* buffer);
	uint8_t readDataToBuff(uint8_t* buffer, uint8_t siz);
	uint8_t readMachine(uint8_t* buffer, uint8_t b_size);
	uint8_t dirInfoRead();
	uint8_t setMode(uint8_t mode);
	uint8_t dirCreate();

	void rdFatInfo();
	void setSpeed();
	void setError(uint8_t errCode);
	void clearError();
	void sendCommand(uint8_t b_parancs);
	void sendFilename();
	void writeFatData();
	void constructDate(uint16_t value, uint8_t ymd);
	void constructTime(uint16_t value, uint8_t hms);
	void rdDiskInfo();
	void rstFileContainer();
	void rstDriveContainer();

	///////Global Variables///////////////////////////////
	bool _deviceAttached = false;	//false USB detached, true attached
	bool _controllerReady = false; // ha sikeres a kommunikacio
	bool _hwSerial;
	uint8_t _fileWrite = 0; // read or write mode, needed for close operation
	uint8_t _dirDepth = 0;// Don't check SD card if it's in subdir
	uint8_t _byteCounter = 0; //vital variable for proper reading,writing
	uint8_t _answer = 0;	//a CH jelenlegi statusza INTERRUPT
	uint8_t _driveSource = 0;//0 = USB, 1 = SD
	uint8_t _spiChipSelect; // chip select pin SPI
	uint8_t _intPin; // interrupt pin
	uint8_t _errorCode = 0; // Store the last error code(see datasheet or CommDef.h)
	uint16_t _sectorCounter = 0;// variable for proper reading
	uint32_t _speed ; // Serial communication speed

	fSizeContainer _cursorPos; //unsigned long union

	char _filename[12];

	HardwareSerial* _comPortHW; // Serial interface
	Stream* _comPort;
	SPISettings _spiSpeed;

	commInterface _interface;
	fileProcessENUM fileProcesSTM = REQUEST;

	fatFileInfo _fileData;
	diskInfo _diskData;


};//end class

#endif /* Ch376msc_H_ */


