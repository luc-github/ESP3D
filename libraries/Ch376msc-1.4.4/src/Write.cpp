/*
 * Write.cpp
 *
 *  Created on: Jan 30, 2020
 *      Author: György Kovács
 */



#include "Ch376msc.h"

uint8_t Ch376msc::writeFile(char* buffer, uint8_t b_size){
	return writeMachine((uint8_t*)buffer,b_size);
}
////////////////////////////////////////////////////////////
uint8_t Ch376msc::writeRaw(uint8_t* buffer, uint8_t b_size){
	return writeMachine(buffer,b_size);
}
////////////////////////////////////////////////////////////
uint8_t Ch376msc::writeChar(char trmChar){
	return writeMachine((uint8_t*)&trmChar,1);
}
/////////////////////////////////////////////////////////////
//					Numbers
///////////////////////////8////////////////////////////////
uint8_t Ch376msc::writeNum(uint8_t buffer){
	char strBuffer[4];//max 255 = 3+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNum(int8_t buffer){
	char strBuffer[5];//max -128 = 4+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
///////////////////////////ln8/////////////////////////////////
uint8_t Ch376msc::writeNumln(uint8_t buffer){
	char strBuffer[6];//max 255 = 3+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNumln(int8_t buffer){
	char strBuffer[7];//max -128 = 4+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
/////////////////////////////////////////////////////////////

///////////////////////////16////////////////////////////////
uint8_t Ch376msc::writeNum(uint16_t buffer){
	char strBuffer[6];//max 65535 = 5+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNum(int16_t buffer){
	char strBuffer[7];//max -32768 = 6+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
///////////////////////////ln16/////////////////////////////////
uint8_t Ch376msc::writeNumln(uint16_t buffer){
	char strBuffer[8];//max 65535 = 5+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNumln(int16_t buffer){
	char strBuffer[9];//max -32768 = 6+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
/////////////////////////////////////////////////////////////

///////////////////////////32////////////////////////////////
uint8_t Ch376msc::writeNum(uint32_t buffer){
	char strBuffer[11];//max 4 294 967 295 = 10+1 char
	ltoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNum(int32_t buffer){
	char strBuffer[12];//max -2147483648 = 11+1 char
	ltoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}

///////////////////////////ln32/////////////////////////////////
uint8_t Ch376msc::writeNumln(uint32_t buffer){
	char strBuffer[13];//max 4 294 967 295 = 10+2+1 char
	ltoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNumln(int32_t buffer){
	char strBuffer[14];//max -2147483648 = 11+2+1 char
	ltoa(buffer, strBuffer, 10);
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
////////////////////////////////////////////////////////////////

////////////////////////double//////////////////////////////////
uint8_t Ch376msc::writeNum(double buffer){
	char strBuffer[15];
	if(buffer > 4100000.00 || buffer < -4100000.00){//beyond these values, the accuracy decreases rapidly
		strcpy(strBuffer,"ovf");					//constant determined by trial and error
	} else {
		dtostrf(buffer, 1, 2, strBuffer);
	}
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}
//	//	//	//	//
uint8_t Ch376msc::writeNumln(double buffer){
	char strBuffer[15];
	if(buffer > 4100000.00 || buffer < -4100000.00){
		strcpy(strBuffer,"ovf");
	} else {
		dtostrf(buffer, 1, 2, strBuffer);
	}
	strcat(strBuffer,"\r\n");
	return writeMachine((uint8_t*)strBuffer,strlen(strBuffer));
}

/////////////////////////////////////////////////////////////////

uint8_t Ch376msc::writeDataFromBuff(uint8_t* buffer){//====================
	uint8_t oldCounter = _byteCounter; //old buffer counter
	uint8_t dataLength; // data stream size
	if(_interface == UARTT) {
		sendCommand(CMD_WR_REQ_DATA);
		dataLength = readSerDataUSB(); // data stream size
	} else {
		spiBeginTransfer();
		sendCommand(CMD_WR_REQ_DATA);
		dataLength = spiReadData(); // data stream size
	}
	while(_byteCounter < (dataLength + oldCounter)){
		write(buffer[_byteCounter]); // read data from buffer and write to serial port
		_byteCounter ++;
	}//end while
	if(_interface == SPII) spiEndTransfer();
	return dataLength;
}

////////////////////////////////////////////////////////////////
uint8_t Ch376msc::writeMachine(uint8_t* buffer, uint8_t b_size){
	bool diskFree = true; //free space on a disk
	bool bufferFull = true; //continue to write while there is data in the temporary buffer
	uint32_t tmOutCnt = 0;
	if(!_deviceAttached) return 0x00;
	_fileWrite = 1; // read mode, required for close procedure
	_byteCounter = 0;

	if(_diskData.freeSector == 0){
		diskFree = false;
		return diskFree;
	}
	if(_answer == ANSW_ERR_MISS_FILE){ // no file with given name
		_answer = fileCreate();
	}//end if CREATED

	if(_answer == ANSW_ERR_FILE_CLOSE){
		_answer = openFile();
	}

	if(_answer == ANSW_USB_INT_SUCCESS){ // file created succesfully
		tmOutCnt = millis();
		while(bufferFull){
			if(millis() - tmOutCnt >= ANSWTIMEOUT) setError(CH376_ERR_TIMEOUT);
			if(!_deviceAttached){
				diskFree = false;
				break;
			}
			switch (fileProcesSTM) {
				case REQUEST:
					_answer = reqByteWrite(b_size - _byteCounter);

					if(_answer == ANSW_USB_INT_SUCCESS){
						fileProcesSTM = NEXT;

					} else if(_answer == ANSW_USB_INT_DISK_WRITE){
						fileProcesSTM = READWRITE;
						}
					break;
				case READWRITE:
					writeDataFromBuff(buffer);
					if(_byteCounter != b_size) {
						fileProcesSTM = NEXT;
					} else {
						fileProcesSTM = DONE;
					}
					break;
				case NEXT:
					if(_diskData.freeSector > 0){
						_diskData.freeSector --;
						_answer = byteWrGo();
						if(_answer == ANSW_USB_INT_SUCCESS){
							fileProcesSTM = REQUEST;
						} else if(_byteCounter != b_size ){
							fileProcesSTM = READWRITE;
						}
					} else { // if disk is full
						fileProcesSTM = DONE;
						diskFree = false;
					}
					break;
				case DONE:
					fileProcesSTM = REQUEST;
					_cursorPos.value += _byteCounter;
					_byteCounter = 0;
					_answer = byteWrGo();
					bufferFull = false;
					break;
			}//end switch
		}//end while
	}// end file created

	return diskFree;
}

