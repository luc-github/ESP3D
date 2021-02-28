/*------------------------------------------------------------------------------------------------------------------
 *    Author: György Kovács                                                                                         |
 *    Created: 03 Feb 2020                                                                                          |
 *    Description: Basic usage of functions read/write numbers                                                      |
 *                                                                                                                  |
 *------------------------------------------------------------------------------------------------------------------
 */

#include <Ch376msc.h>

// use this if no other device are attached to SPI port(MISO pin used as interrupt)
Ch376msc flashDrive(10); // chipSelect

//If the SPI port shared with other devices e.g TFT display, etc. remove from comment the code below and put the code above in a comment
//Ch376msc flashDrive(10, 9); // chipSelect, interrupt pin

// buffer for reading
char adatBuffer[25];// max length 255 = 254 char + 1 NULL character

void setup() {
  Serial.begin(115200);
  flashDrive.init();

  if(flashDrive.driveReady()){
    flashDrive.setFileName("INTEGER.TXT");
    flashDrive.openFile();
    Serial.println(F("Writing integer numbers :"));
    for(int a = 0; a < 20; a++){
      int dd = random(-4000, 4000);
      Serial.println(dd);
      flashDrive.writeNumln(dd);
    }
    flashDrive.closeFile();
    Serial.println(F("Writing done"));
    Serial.println();
    ////////////////////////////////////////
    flashDrive.setFileName("INTEGER.TXT");
    flashDrive.openFile();
    Serial.println(F("Reading integer numbers :"));
    while(!flashDrive.getEOF()){
      Serial.println(flashDrive.readLong());
    }
    flashDrive.closeFile();
    Serial.println(F("Reading done"));
    Serial.println();

    /////////////////////////////////////////////
    flashDrive.setFileName("FLOAT.TXT");
    flashDrive.openFile();
    Serial.println(F("Writing float numbers :"));
    for(int a = 0; a < 20; a++){
      float dd = random(-40, 40) * 3.14;
      Serial.println(dd);
      flashDrive.writeNumln(dd);
    }
    flashDrive.closeFile();
    Serial.println(F("Writing done"));
    Serial.println();
    ////////////////////////////////////////
    flashDrive.setFileName("FLOAT.TXT");
    flashDrive.openFile();
    Serial.println(F("Reading float numbers :"));
    while(!flashDrive.getEOF()){
      Serial.println(flashDrive.readDouble());
    }
    flashDrive.closeFile();
    Serial.println(F("Reading done"));
    Serial.println();
  } else {
     Serial.println(F("Drive error"));
  }//end drive ready 
}//end setup

void loop() {

}
