/*
 * Read.cpp
 *
 *  Created on: Jan 30, 2020
 *      Author: György Kovács
 */


#include "Ch376msc.h"

bool Ch376msc::readFileUntil(char trmChar, char* buffer, uint8_t b_size){//buffer for reading, buffer size
	char tmpBuff[2];//temporary buffer to read string and analyze
	bool readMore = true;
	uint8_t charCnt = 0;
	b_size --;// last byte is reserved for NULL terminating character
	if(!_deviceAttached) return false;
	for(; charCnt < b_size; charCnt ++){
		readMore = readFile(tmpBuff, sizeof(tmpBuff));
		buffer[charCnt] = tmpBuff[0];
		if((tmpBuff[0] == trmChar) || !readMore){// reach terminate character or EOF
			readMore = false;
			break;
		}
	}
	buffer[charCnt+1] = 0x00;//string terminate
	return readMore;
}
///////////////////////////////////////////////////////////////////
uint8_t Ch376msc::readFile(char* buffer, uint8_t b_size){
	uint8_t tmpReturn;
	b_size--;// last byte is reserved for NULL terminating character
	tmpReturn = readMachine((uint8_t*) buffer,b_size);
	buffer[_byteCounter] = '\0';// NULL terminating char
	_cursorPos.value += _byteCounter;
	_byteCounter = 0;
	return tmpReturn;
}
//////////////////////////////////////////////////////////////////
uint8_t Ch376msc::readRaw(uint8_t* buffer, uint8_t b_size){
	uint8_t tmpReturn;
	tmpReturn = readMachine(buffer,b_size);
	_cursorPos.value += _byteCounter;
	_byteCounter = 0; // make it 0 when the buffer is full
	return tmpReturn;
}
//////////////////////////////////////////////////////////////////
int32_t Ch376msc::readLong(char trmChar){
	char workBuffer[18];
	int32_t retval;
	readFileUntil(trmChar,workBuffer,14);
	retval = atol(workBuffer);
	return retval;
}
//////////////////////////////////////////////////////////////////
uint32_t Ch376msc::readULong(char trmChar){
	char workBuffer[18];
	uint32_t retval;
	readFileUntil(trmChar,workBuffer,14);
	retval = atol(workBuffer);
	return retval;
}
//////////////////////////////////////////////////////////////////
double Ch376msc::readDouble(char trmChar){
	double retval;
	char workBuffer[18];
	readFileUntil(trmChar,workBuffer,14);
	retval = atof(workBuffer);
	return retval;
}

/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::readDataToBuff(uint8_t* buffer, uint8_t siz){
	uint8_t oldCounter = _byteCounter; //old buffer counter
	uint8_t dataLength; // data stream size
	if(_interface == UARTT) {
		sendCommand(CMD_RD_USB_DATA0);
		dataLength = readSerDataUSB(); // data stream size
		if(dataLength > siz){
			setError(CH376_ERR_OVERFLOW);//overflow
			return 0;
		}
		while(_byteCounter < (dataLength + oldCounter)){
			buffer[_byteCounter]=readSerDataUSB(); // incoming data add to buffer
			_byteCounter ++;
		}//end while
	} else {
	spiBeginTransfer();
	sendCommand(CMD_RD_USB_DATA0);
	dataLength = spiReadData(); // data stream size
	if(dataLength > siz){
		setError(CH376_ERR_OVERFLOW);//overflow
		return 0;
	}
	while(_byteCounter < (dataLength + oldCounter)){
		buffer[_byteCounter]=spiReadData(); // incoming data add to buffer
		_byteCounter ++;
	}//end while
	spiEndTransfer();
	}
	return dataLength;
}

//////////////////////////////////////////////////////////////
uint8_t Ch376msc::readMachine(uint8_t* buffer, uint8_t b_size){ //buffer for reading, buffer size
	uint8_t tmpReturn = 0;// more data
	uint8_t byteForRequest = 0 ;
	bool bufferFull = false;
	uint32_t tmOutCnt = 0;
	_fileWrite = 0; // read mode, required for close procedure
	if(_answer == ANSW_ERR_FILE_CLOSE || _answer == ANSW_ERR_MISS_FILE){
		bufferFull = true;
		tmpReturn = 0;// we have reached the EOF
	}
	tmOutCnt = millis();

	while(!bufferFull){
		if(millis() - tmOutCnt >= ANSWTIMEOUT) setError(CH376_ERR_TIMEOUT);
		if(!_deviceAttached){
			tmpReturn = 0;
			break;
		}
		switch (fileProcesSTM) {
			case REQUEST:
				byteForRequest = b_size - _byteCounter;
				if(_sectorCounter == SECTORSIZE){ //if one sector has read out
					_sectorCounter = 0;
					fileProcesSTM = NEXT;
					break;
				} else if((_sectorCounter + byteForRequest) > SECTORSIZE){
					byteForRequest = SECTORSIZE - _sectorCounter;
				}
				////////////////
				_answer = reqByteRead(byteForRequest);
				if(_answer == ANSW_USB_INT_DISK_READ){
					fileProcesSTM = READWRITE;
					tmpReturn = 1; //we have not reached the EOF
				} else if(_answer == ANSW_USB_INT_SUCCESS){ // no more data, EOF
					fileProcesSTM = DONE;
					tmpReturn = 0;
				}
				break;
			case READWRITE:
				_sectorCounter += readDataToBuff(buffer, byteForRequest);	//fillup the buffer
				if(_byteCounter != b_size) {
					fileProcesSTM = REQUEST;
				} else {
					fileProcesSTM = DONE;
				}
				break;
			case NEXT:
				_answer = byteRdGo();
				fileProcesSTM = REQUEST;
				break;
			case DONE:
				fileProcesSTM = REQUEST;
				bufferFull = true;
				break;
		}//end switch
	}//end while
		return tmpReturn;
}

