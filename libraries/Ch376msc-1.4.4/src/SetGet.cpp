/*
 * SetGet.cpp
 *
 *  Created on: Mar 27, 2019
 *      Author: György Kovács
 */

#include "Ch376msc.h"

//////////////////SetGet////////////////////////////

void Ch376msc::setSpeed(){ //set communication speed for HardwareSerial and device
	if(_speed == 9600){ // default speed for CH
 // do nothing
	} else {
		sendCommand(CMD_SET_BAUDRATE);
		switch (_speed) {
			case 19200:
				_comPortHW->write(uint8_t(0x02));//detach freq. coef
				_comPortHW->write(uint8_t(0xD9));//detach freq. constant
				break;
			case 57600:
				_comPortHW->write(uint8_t(0x03));
				_comPortHW->write(uint8_t(0x98));
				break;
			case 115200:
				_comPortHW->write(uint8_t(0x03));
				_comPortHW->write(uint8_t(0xCC));
				break;
			default:
				_speed = 9600;
				break;
		}//end switch
		_comPortHW->end();
		_comPortHW->begin(_speed);
		delay(2);// according to datasheet 2ms
	}// end if

}
//////////////////////////////////////////////////////////
void Ch376msc::setSource(uint8_t inpSource){
	if(_driveSource != inpSource){
		_driveSource = inpSource;
		if(_driveSource == 1){// SD mode
			driveDetach();
			setMode(MODE_DEFAULT);
			setMode(MODE_HOST_SD);
			driveAttach();
		} else {// USB mode
			driveDetach();
			driveAttach();
		}//end if SD
	}//end if not
}

/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::setMode(uint8_t mode){
	uint8_t tmpReturn = 0;
	if(_interface == UARTT){
		sendCommand(CMD_SET_USB_MODE);
		write(mode);
		tmpReturn = readSerDataUSB();
	} else {//spi
		spiBeginTransfer();
		sendCommand(CMD_SET_USB_MODE);
		write(mode);
		delayMicroseconds(10);
		tmpReturn = spiReadData();
		spiEndTransfer();
		delayMicroseconds(40);
	}
	return tmpReturn; // success or fail
}
/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::getSource(){
	return _driveSource;
}

bool Ch376msc::getDeviceStatus(){
	return _deviceAttached;
}
bool Ch376msc::getCHpresence(){
	return _controllerReady;
}

uint8_t Ch376msc::getFileAttrb(){
	return _fileData.fattr;
}

char* Ch376msc::getFileName(){
	strncpy(_filename,_fileData.name,11);
	_filename[11] = '\0';
	return _filename;
}
void Ch376msc::setFileName(const char* filename){
	if(_deviceAttached){
		if(strlen(filename)){//copy if file name is given
			strncpy(_filename,filename,12);//copy the filename string to internal filename variable
		} else {
			getFileName();
		}
		sendFilename(); // send to the CH376
	}
}
uint8_t Ch376msc::getStatus(){
	return _answer;
}

uint32_t Ch376msc::getFileSize(){
	return _fileData.size;
}

uint32_t Ch376msc::getCursorPos(){
	return _cursorPos.value;
}

char* Ch376msc::getFileSizeStr(){ // make formatted file size string from unsigned long
	// final string is declared as static, return value
	static char fsizeString[10];// e.g 1023 byte\0 , 9 char long + NULL terminating char
	fsizeString[0] = 0;						  // or 1023,9 Kb\0
	uint32_t ul_size = _fileData.size;
	float fl_size;
	char strNumber[7]; // e.g 1023,9\0 , temporary buffer
	if(_deviceAttached){
		if(ul_size >= 1048576){ // if the filesize is 1Mb or bigger
			fl_size = ul_size / 1048576.0;
			dtostrf(fl_size, 1, 1, fsizeString);//convert float to string
			strcat(fsizeString," Mb");  // concatenate unit symbol
		} else if(ul_size >= 1024){ // if the filesize is in Kb range
			fl_size = ul_size / 1024.0;
			dtostrf(fl_size, 1, 1, fsizeString);//convert float to string
			strcat(fsizeString," Kb");
		} else { // if the filesize is in byte range
			ltoa(ul_size, strNumber, 10);// convert long to string
			strcpy(fsizeString,strNumber);// copy to the final string
			strcat(fsizeString," byte");// concatenate unit symbol
		}//end size
	}//end if attached

	return fsizeString; //return the final string
}

void Ch376msc::setYear(uint16_t year){ //Year(0 = 1980, 119 = 2099 supported under DOS/Windows, theoretically up to 127 = 2107)
	if(year > 2099) year = 2099;
	if(year < 1980) year = 1980;
	year -= 1980;
	constructDate(year, 0);
}

uint16_t Ch376msc::getYear(){
	uint16_t year = _fileData.modDate;
	year = year>> 9;
	year += 1980;
	return year;
}
void Ch376msc::setMonth(uint16_t month){
	if(month > 12) month = 12;
	if(month < 1) month = 1;
	constructDate(month, 1);
}
uint16_t Ch376msc::getMonth(){
	uint16_t month = _fileData.modDate;
	month = month << 7;
	month = month >> 12;
	return month;
}
void Ch376msc::setDay(uint16_t day){
	if(day > 31) day = 31;
	if(day < 1) day = 1;
	constructDate(day, 2);
}
uint16_t Ch376msc::getDay(){
	uint16_t day = _fileData.modDate;
	day = day << 11;
	day = day >> 11;
	return day;
}
//
void Ch376msc::setHour(uint16_t hour){ //Coordinated Universal Time (UTC)
	if(hour > 23) hour = 23;
	constructTime(hour, 0);
}

uint16_t Ch376msc::getHour(){
	uint16_t hour = _fileData.modTime;
	hour = hour >> 11;
	return hour;
}
void Ch376msc::setMinute(uint16_t minute){
	if(minute > 59) minute = 59;
	constructTime(minute, 1);
}
uint16_t Ch376msc::getMinute(){
	uint16_t minute = _fileData.modTime;
	minute = minute << 5;
	minute = minute >> 10;
	return minute;
}
void Ch376msc::setSecond(uint16_t second){ //! 0-58 2sec steps
	if(second > 59) second = 59;
	second /= 2;
	constructTime(second, 2);
}
uint16_t Ch376msc::getSecond(){
	uint16_t second = _fileData.modTime;
	second = second << 11;
	second = second >> 11;
	second *= 2;
	return second;
}
///////////////////////////////////

void Ch376msc::constructDate(uint16_t value, uint8_t ymd){ // 0-year, 1-month, 2-day
	uint16_t tmpInt = _fileData.modDate;
	uint16_t year;
	uint16_t month;
	uint16_t day;
	/*<------- 0x19 --------> <------- 0x18 -------->
	 *15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	 *y  y  y  y  y  y  y  m  m  m  m  d  d  d  d  d
	 */
	if(_deviceAttached){
		year = tmpInt >> 9;
		year = year << 9;

		month = tmpInt << 7;
		month = month >> 12;
		month = month << 5;

		day = tmpInt << 11;
		day = day >> 11;

		switch (ymd) {
			case 0://year
				year = value;
				year = year << 9;
				break;
			case 1://month
				month = value;
				month = month << 5;
				break;
			case 2://day
				day = value;
				break;
			default:
				break;
		}//end switch
		_fileData.modDate = year + month + day;
	}//end if attached
}

void Ch376msc::constructTime(uint16_t value, uint8_t hms){
	uint16_t tmpInt = _fileData.modTime;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	/*<------- 0x17 --------> <------- 0x16 -------->
	 *15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	 *h  h  h  h  h  m  m  m  m  m  m  s  s  s  s  s
	 */
	if(_deviceAttached){
		hour = tmpInt >> 11; //cut minute and second
		hour = hour << 11; // set hour

		minute = tmpInt << 5; // cut hour
		minute = minute >> 10;// cut seconds
		minute = minute << 5; // set minute

		second = tmpInt << 11; // cut hour and minute
		second = second >> 11; // set second

		switch (hms) {
			case 0://hour
				hour = value;
				hour = hour << 11;
				break;
			case 1://minute
				minute = value;
				minute = minute << 5;
				break;
			case 2://second
				second = value;
				break;
			default:
				break;
		}//end switch
		_fileData.modTime = hour + minute + second;
	}//end if attached
}
//////////////////////////////////////////////////////
uint32_t Ch376msc::getTotalSectors(){ // disk(partition?) size in bytes = totalSector * SECTORSIZE
	return _diskData.totalSector;
}
//////////////////////////////////////////////////////
uint32_t Ch376msc::getFreeSectors(){ // total free bytes = freeSector * SECTORSIZE
	return _diskData.freeSector;
}
//////////////////////////////////////////////////////
uint8_t Ch376msc::getFileSystem(){ //0x01-FAT12, 0x02-FAT16, 0x03-FAT32
	return _diskData.diskFat;
}
//////////////////////////////////////////////////////
void Ch376msc::clearError(){
	_errorCode = 0;
}

void Ch376msc::setError(uint8_t errCode){
	_errorCode = errCode;
	_deviceAttached = false;
	_dirDepth = 0;
	_byteCounter = 0;
	_answer = 0;
	resetFileList();
	rstDriveContainer();
	rstFileContainer();

}

void Ch376msc::rstDriveContainer(){
	memset(&_diskData, 0, sizeof(_diskData));// fill up with NULL disk data container
}

void Ch376msc::rstFileContainer(){
	memset(&_fileData, 0, sizeof(_fileData));// fill up with NULL file data container
	_filename[0] = '\0'; // put  NULL char at the first place in a name string
	_fileWrite = 0;
	_sectorCounter = 0;
	_cursorPos.value = 0;
}

uint8_t Ch376msc::getError(){
	return _errorCode;
}

void Ch376msc::resetFileList(){
	fileProcesSTM = REQUEST;
}

bool Ch376msc::getEOF(){
	if(_cursorPos.value < _fileData.size){
		return false;
	} else {
		return true;
	}
}

uint8_t Ch376msc::getChipVer(){
	uint8_t tmpReturn = 0;
	if(_interface == UARTT){
		sendCommand(CMD_GET_IC_VER);
		//write(mode);
		tmpReturn = readSerDataUSB();
	} else {//spi
		spiBeginTransfer();
		sendCommand(CMD_GET_IC_VER);
		//write(mode);
		delayMicroseconds(10);
		tmpReturn = spiReadData();
		spiEndTransfer();
		//delayMicroseconds(40);
	}
	return tmpReturn;
}
