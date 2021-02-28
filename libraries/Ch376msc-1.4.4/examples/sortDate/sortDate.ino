/*------------------------------------------------------------------------------------------------------------------
 *    Author: György Kovács                                                                                         |
 *    Created: 26 Sep 2019                                                                                          |
 *    Description: Finding the oldest file, it can be useful e.g. writing log files to drive and                    |
 *    if the flash drive becomes full then we can delete the oldest.                                                |
 *------------------------------------------------------------------------------------------------------------------
 */


#include <Ch376msc.h>

//..............................................................................................................................
// Connect to SPI port: MISO, MOSI, SCK

// use this if no other device are attached to SPI port(MISO pin used as interrupt)
Ch376msc flashDrive(10); // chipSelect

//If the SPI port shared with other devices e.g SD card, display, etc. remove from comment the code below and put the code above in a comment
//Ch376msc flashDrive(10, 9); // chipSelect, interrupt pin

//..............................................................................................................................

//..............................................................................................................................
char fname[12] = "TEST1.TXT";
char adat[] = "Lorem ipsum dolor sit amet";

int fyear = 0;
int fmonth = 0;
int fday = 0;
int fhour = 0;
int fminute = 0;
int fsecond = 0;
unsigned long oldUnTime = 0;



void setup() {
  Serial.begin(115200);
  //Read more about randomSeed at https://www.arduino.cc/reference/en/language/functions/random-numbers/randomseed/
  randomSeed(analogRead(0));
  flashDrive.init();
  makeFiles(); // create 10 files on the flash drive

  while(flashDrive.listDir()){ // read root directory
	  if((convUnixTime() < oldUnTime) || !oldUnTime){ //looking for the oldest file or change the '<' symbol to '>'
		  updateOldestFile();                         // if you looking for the newest file
	  }//end if file is older
  }//end while
  printFileData(); // print the oldest file name in to serial terminal
}//end setup

void loop() {
	// do nothing
} //end loop

unsigned long convUnixTime(){ // calculate "quasi" Unix time without taking into account leap years
							  // Unix time is the number of seconds that have elapsed since 1970.01.01 00:00:00
							  // for the multipliers check the link https://www.epochconverter.com/
	unsigned long unxTime;
	int yyr = flashDrive.getYear();
	yyr -= 1970; 					//elapsed years since 1970
	unxTime = (yyr * 31556926UL);
	unxTime += (flashDrive.getMonth() * 2629743UL);
	unxTime += (flashDrive.getDay() * 86400UL);
	unxTime += (flashDrive.getHour() * 3600UL);
	unxTime += (flashDrive.getMinute() * 60UL);
	unxTime += flashDrive.getSecond();
	return unxTime;
}

void updateOldestFile(){
	  fyear = flashDrive.getYear();
	  fmonth = flashDrive.getMonth();
	  fday = flashDrive.getDay();
	  fhour = flashDrive.getHour();
	  fminute = flashDrive.getMinute();
	  fsecond = flashDrive.getSecond();
	  strcpy(fname,flashDrive.getFileName()); //copy file name to fname variable
	  oldUnTime = convUnixTime(); // update the oldest time variable
}

void printFileData(){ // Print data to the serial terminal
	  Serial.print("The oldest file is:\t");
	  Serial.print(fname);
	  Serial.print(' ');
	  Serial.print(fyear);
	  Serial.print('.');
	  Serial.print(fmonth);
	  Serial.print('.');
	  Serial.print(fday);
	  Serial.print(' ');
	  Serial.print(fhour);
	  Serial.print(':');
	  Serial.print(fminute);
	  Serial.print(':');
	  Serial.println(fsecond);
}

void makeFiles(){
	for(byte a = 0; a < 10; a++){
		fname[4] = (char)(a+0x30);//change the number in the file name(a + ASCII hex number(0))
		Serial.println(fname);
		flashDrive.setFileName(fname);
		flashDrive.openFile();
		flashDrive.writeFile(adat, strlen(adat));
		flashDrive.closeFile();

		flashDrive.setFileName(fname);
		flashDrive.openFile();

        flashDrive.setYear(random(1990, 2020)); // generate and set random year
        flashDrive.setMonth(random(1, 12));		// generate and set random month
        flashDrive.setDay(random(1, 30));		// generate and set random day
        flashDrive.setHour(random(0, 23));		// generate and set random hour
        flashDrive.setMinute(random(0, 59));	// generate and set random minute
        flashDrive.setSecond(random(0, 59));	// generate and set random second

        flashDrive.saveFileAttrb();           //save the changed data
		flashDrive.closeFile();

	}

}
