/*
 * Ch376msc.cpp
 *
 *  Created on: Feb 25, 2019
 *      Author: György Kovács
 */

#include "Ch376msc.h"

//with HW serial
Ch376msc::Ch376msc(HardwareSerial &usb, uint32_t speed) { // @suppress("Class members should be properly initialized")
	_interface = UARTT;
	_comPortHW = &usb;
	_comPort = &usb;
	_speed = speed;
	_hwSerial = true;
}
//with soft serial
Ch376msc::Ch376msc(Stream &sUsb) { // @suppress("Class members should be properly initialized")
	_interface = UARTT;
	_comPort = &sUsb;
	_speed = 9600;
	_hwSerial = false;
}

Ch376msc::Ch376msc(uint8_t spiSelect, uint8_t intPin, SPISettings speed){ // @suppress("Class members should be properly initialized")
	_interface = SPII;
	_intPin = intPin;
	_spiChipSelect = spiSelect;
	//_speed = speed;
	_spiSpeed = speed;
}

//with SPI, MISO as INT pin(the SPI bus is only available for CH376, SPI bus can`t be shared with other SPI devices)
Ch376msc::Ch376msc(uint8_t spiSelect, SPISettings speed){ // @suppress("Class members should be properly initialized")
	_interface = SPII;
	_intPin = MISO; // use the SPI MISO for interrupt JUST if no other device is using the SPI bus!!
	_spiChipSelect = spiSelect;
	//_speed = speed;
	_spiSpeed = speed;
}
Ch376msc::~Ch376msc() {
	//  Auto-generated destructor stub
}

/////////////////////////////////////////////////////////////////
void Ch376msc::init(){
	delay(60);//wait for VCC to normalize
	if(_interface == SPII){
		if(_intPin != MISO){
			pinMode(_intPin, INPUT_PULLUP);
		}
		pinMode(_spiChipSelect, OUTPUT);
		digitalWrite(_spiChipSelect, HIGH);
		SPI.begin();
		spiBeginTransfer();
		sendCommand(CMD_RESET_ALL);
		spiEndTransfer();
		delay(100);// wait after reset command
		if(_intPin == MISO){ // if we use MISO as interrupt pin, then tell it to the device ;)
			spiBeginTransfer();
			sendCommand(CMD_SET_SD0_INT);
			write(0x16);
			write(0x90);
			spiEndTransfer();
		}//end if
	} else {//UART
		if(_hwSerial) _comPortHW->begin(9600);// start with default speed
		sendCommand(CMD_RESET_ALL);
		delay(100);// wait after reset command, according to the datasheet 35ms is required, but that was too short
		if(_hwSerial){ // if Hardware serial is initialized
			setSpeed(); // Dynamically configure the com speed
		}
	}//end if UART
	_controllerReady = pingDevice();// check the communication
	if(_controllerReady) clearError();// reinit clear last error code
	setMode(MODE_HOST_0);
	checkIntMessage();
}

/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::pingDevice(){
	uint8_t tmpReturn = 0;
	if(_interface == UARTT){
		sendCommand(CMD_CHECK_EXIST);
		write(0x01); // ez ertek negaltjat adja vissza
		if(readSerDataUSB() == 0xFE){
			tmpReturn = 1;//true
		}
	} else {
		spiBeginTransfer();
		sendCommand(CMD_CHECK_EXIST);
		write(0x01); // ez ertek negaltjat adja vissza
		if(spiReadData() == 0xFE){
			tmpReturn = 1;//true
		}
		spiEndTransfer();
	}
	if(!tmpReturn){
		setError(CH376_ERR_NO_RESPONSE);
	}
	return tmpReturn;
}
/////////////////////////////////////////////////////////////////
bool Ch376msc::driveReady(){//returns TRUE if the drive ready
	uint8_t tmpReturn = 0;
	if(_driveSource == 1){//if SD
		if(!_dirDepth){// just check SD card if it's in root dir
			setMode(MODE_DEFAULT);//reinit otherwise is not possible to detect if the SD card is removed
			setMode(MODE_HOST_SD);
			tmpReturn = mount();
			if(tmpReturn == ANSW_USB_INT_SUCCESS){
				rdDiskInfo();
			} else {
				driveDetach(); // do reinit otherwise mount will return always "drive is present"
			}//end if INT_SUCCESS
		} else tmpReturn = ANSW_USB_INT_SUCCESS;//end if not ROOT
	} else {//if USB
		tmpReturn = mount();
		if(tmpReturn == ANSW_USB_INT_SUCCESS){
			rdDiskInfo();
		}//end if not INT SUCCESS
	}//end if interface
	return _deviceAttached;

}

/////////////////////////////////////////////////////////////////
bool Ch376msc::checkIntMessage(){ //always call this function to get INT# message if thumb drive are attached/detached
	uint8_t tmpReturn = 0;
	bool intRequest = false;
		if(_interface == UARTT){
			while(_comPort->available()){ // while is needed, after connecting media, the ch376 send 3 message(connect, disconnect, connect)
				tmpReturn = readSerDataUSB();
			}//end while
		} else {//spi
			while(!digitalRead(_intPin)){
				tmpReturn = getInterrupt(); // get int message
				delay(10);//sadly but it required for stability, sometime prior attaching drive the CH376 produce more interrupts
			}// end while
		}//end if interface
		switch(tmpReturn){ // 0x15 device attached, 0x16 device disconnect
		case ANSW_USB_INT_CONNECT:
			intRequest = true;
			driveAttach();//device attached
			break;
		case ANSW_USB_INT_DISCONNECT:
			intRequest = true;
			driveDetach();//device detached
			break;
		}//end switch
	return intRequest;
}

/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::openFile(){
	if(!_deviceAttached) return 0x00;
	if(_interface == UARTT){
		sendCommand(CMD_FILE_OPEN);
		_answer = readSerDataUSB();
	} else {//spi
		spiBeginTransfer();
		sendCommand(CMD_FILE_OPEN);
		spiEndTransfer();
		_answer = spiWaitInterrupt();
	}
	if(_answer == ANSW_USB_INT_SUCCESS){ // get the file size
		dirInfoRead();
	}
	return _answer;
}


/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::saveFileAttrb(){
	uint8_t tmpReturn = 0;
	if(!_deviceAttached) return 0x00;
	_fileWrite = 1;
	if(_interface == UARTT) {
		sendCommand(CMD_DIR_INFO_READ);
		write(0xff);// current file is 0xff
		readSerDataUSB();
		writeFatData();//send fat data
		sendCommand(CMD_DIR_INFO_SAVE);
		tmpReturn = readSerDataUSB();
	} else {//spi
		spiBeginTransfer();
		sendCommand(CMD_DIR_INFO_READ);
		write(0xff);// current file is 0xff
		spiEndTransfer();
		spiWaitInterrupt();
		writeFatData();//send fat data
		spiBeginTransfer();
		sendCommand(CMD_DIR_INFO_SAVE);
		spiEndTransfer();
		tmpReturn = spiWaitInterrupt();
	}
	return tmpReturn;
}


////////////////////////////////////////////////////////////////
uint8_t Ch376msc::closeFile(){ // 0x00 - w/o filesize update, 0x01 with filesize update
	uint8_t tmpReturn = 0;
	uint8_t d = 0x00;
	if(!_deviceAttached) return 0x00;

	if(_fileWrite == 1){ // if closing file after write procedure
		d = 0x01; // close with 0x01 (to update file length)
	}
	if(_interface == UARTT){
		sendCommand(CMD_FILE_CLOSE);
		write(d);
		tmpReturn = readSerDataUSB();
	} else {
		spiBeginTransfer();
		sendCommand(CMD_FILE_CLOSE);
		write(d);
		spiEndTransfer();
		tmpReturn = spiWaitInterrupt();
	}

	cd("/", 0);//back to the root directory if any file operation has occurred
	rstFileContainer();
	return tmpReturn;
}

////////////////////////////////////////////////////////////////
uint8_t Ch376msc::deleteFile(){
	if(!_deviceAttached) return 0x00;
	if(_interface == UARTT) {
		sendCommand(CMD_FILE_ERASE);
		_answer = readSerDataUSB();
	} else {
		spiBeginTransfer();
		sendCommand(CMD_FILE_ERASE);
		spiEndTransfer();
		_answer = spiWaitInterrupt();
	}
	cd("/",0);
	return _answer;
}

///////////////////Listing files////////////////////////////
uint8_t Ch376msc::listDir(const char* filename){
/* __________________________________________________________________________________________________________
 * | 00 - 07 | 08 - 0A |  	0B     |     0C    |     0D     | 0E  -  0F | 10  -  11 | 12 - 13|  14 - 15 |
 * |Filename |Extension|File attrib|User attrib|First ch del|Create time|Create date|Owner ID|Acc rights|
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * | 16 - 17 | 18 - 19 |   1A - 1B   |  1C  -  1F |
 * |Mod. time|Mod. date|Start cluster|  File size |
 *
 * https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
 * http://www.tavi.co.uk/phobos/fat.html
 */
	bool moreFiles = true;  // more files waiting for read out
	bool doneFiles = false; // done with reading a file
	uint32_t tmOutCnt = millis();

	while(!doneFiles){
		if(millis() - tmOutCnt >= ANSWTIMEOUT) setError(CH376_ERR_TIMEOUT);
		if(!_deviceAttached){
			moreFiles = false;
			break;
		}
		switch (fileProcesSTM) {
			case REQUEST:
				setFileName(filename);
				_answer = openFile();
				//_fileWrite = 2; // if in subdir
				fileProcesSTM = READWRITE;
				break;
			case READWRITE:
				if(_answer == ANSW_ERR_MISS_FILE){
					fileProcesSTM =DONE;
					moreFiles = false;// no more files in the directory
				}//end if
				if(_answer == ANSW_USB_INT_DISK_READ){
					rdFatInfo(); // read data to fatInfo buffer
					fileProcesSTM = NEXT;
				}
				break;
			case NEXT:
				_answer = fileEnumGo(); // go for the next filename
				fileProcesSTM = DONE;
				break;
			case DONE:
				if(!moreFiles){
					//closeFile(); // if no more files in the directory, close the file
					//closing file is not required after print dir (return value was always 0xB4 File is closed)
					fileProcesSTM = REQUEST;
				} else {
					fileProcesSTM = READWRITE;
				}
				doneFiles = true;
				break;
		}// end switch
	}//end while
	return moreFiles;
}

/////////////////////////////////////////////////////////////////
uint8_t Ch376msc::moveCursor(uint32_t position){
	uint8_t tmpReturn = 0;
	if(!_deviceAttached) return 0x00;

	if(position > _fileData.size){	//fix for moveCursor issue #3 Sep 17, 2019
		_sectorCounter = _fileData.size % SECTORSIZE;
	} else {
		_sectorCounter = position % SECTORSIZE;
	}
	_cursorPos.value = position;//temporary
	if(_interface == SPII) spiBeginTransfer();
	sendCommand(CMD_BYTE_LOCATE);
	write(_cursorPos.b[0]);
	write(_cursorPos.b[1]);
	write(_cursorPos.b[2]);
	write(_cursorPos.b[3]);
	if(_interface == UARTT){
		tmpReturn = readSerDataUSB();
	} else {
		spiEndTransfer();
		tmpReturn = spiWaitInterrupt();
	}
	if(_cursorPos.value > _fileData.size){
		_cursorPos.value = _fileData.size;//set the valid position
	}
	return tmpReturn;
}

//////////////////////////////////////////////////////////////////////
uint8_t Ch376msc::cd(const char* dirPath, bool mkDir){
	uint8_t tmpReturn = 0;
	uint8_t pathLen = strlen(dirPath);
	if(!_deviceAttached) return 0x00;

	_dirDepth = 0;
	if(pathLen < ((MAXDIRDEPTH*8)+(MAXDIRDEPTH+1)) ){//depth*(8char filename)+(directory separators)
		char input[pathLen + 1];
		strcpy(input,dirPath);
			  setFileName("/");
			  tmpReturn = openFile();
		char* command = strtok(input, "/");//split path into tokens
		  while (command != NULL && !_errorCode){
			  if(strlen(command) > 8){//if a dir name is longer than 8 char
				  tmpReturn = CH376_ERR_LONGFILENAME;
				  break;
			  }
			  setFileName(command);
			  tmpReturn = openFile();
			  if(tmpReturn == ANSW_USB_INT_SUCCESS){//if file already exist with this name
				  tmpReturn = ANSW_ERR_FOUND_NAME;
				  closeFile();
				  break;
			  } else if(mkDir && (tmpReturn == ANSW_ERR_MISS_FILE)){
				  tmpReturn = dirCreate();
				  if(tmpReturn != ANSW_USB_INT_SUCCESS) break;
			  }//end if file exist
			  _dirDepth++;
			  command = strtok (NULL, "/");
		  }
	} else {
		tmpReturn = CH376_ERR_LONGFILENAME;
	}//end if path is to long
	return tmpReturn;
}


