/*------------------------------------------------------------------------------------------------------------------
 *    Author: György Kovács                                                                                         |
 *    Created: 27 Jan 2020                                                                                          |
 *    Description: Basic usage of function readFileUntil();                                                         |
 *                 Create TEST1.TXT file with e.g. basicUsageSPI.ino                                                |
 *------------------------------------------------------------------------------------------------------------------
 */

#include <Ch376msc.h>

// use this if no other device are attached to SPI port(MISO pin used as interrupt)
Ch376msc flashDrive(10); // chipSelect

//If the SPI port shared with other devices e.g TFT display, etc. remove from comment the code below and put the code above in a comment
//Ch376msc flashDrive(10, 9); // chipSelect, interrupt pin

// buffer for reading
char adatBuffer[25];// max length 255 = 254 char + 1 NULL character
boolean readMore = true;
int trmCharCounter = 0;

void setup() {
  Serial.begin(115200);
  flashDrive.init();

  if(flashDrive.driveReady()){
    flashDrive.setFileName("TEST1.TXT");  //set the file name
    flashDrive.openFile();                //open the file
                   //read data from flash drive until we reach EOF
    while(!flashDrive.getEOF()){
      readMore = true;
      while(readMore){
        // terminator character, temporary buffer where we read data from flash drive and the size of that buffer
        // returns boolean true if the given buffer is full and didn't found the terminator character
        readMore = flashDrive.readFileUntil('\n', adatBuffer, sizeof(adatBuffer));//new line character
        Serial.print(adatBuffer);          //print the contents of the temporary buffer
      }//end while readMore
      trmCharCounter++;
    }//end while not EOF
    Serial.print(F("Total trm. character found: "));
    Serial.println(trmCharCounter);
  } else {
	  Serial.println(F("Drive not initialized"));
  }//end if driveReady
}//end setup

void loop() {

}
