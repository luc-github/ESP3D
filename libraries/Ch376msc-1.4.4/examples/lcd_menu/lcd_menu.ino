/*------------------------------------------------------------------------------------------------------------------
 *    Author: György Kovács                                                                                         |
 *    Created: 05 Feb 2020                                                                                          |
 *    Description: One variation of the LCD menu without using a large buffer to store filenames                    |
 *    In fact, we select the current file based on the file number (Folder handling is not implemented)             |
 *------------------------------------------------------------------------------------------------------------------
 */


#include <Ch376msc.h>
#include <LiquidCrystal.h>
//..............................................................................................................................
// Connect to SPI port: MISO, MOSI, SCK

// These function-like macros are optionally used when creating an object
//e.g. SPI_SCK_KHZ(500) - set the clock speed to 500 kHz
//e.g. SPI_SCK_MHZ(1) - set the clock speed to 1 MHz
// use this if no other device are attached to SPI port(MISO pin used as interrupt)
Ch376msc flashDrive(10,SPI_SCK_KHZ(500)); // chipSelect
//If the SPI port shared with other devices e.g TFT display, etc. remove from comment the code below and put the code above in a comment
//Ch376msc flashDrive(10, 9); // chipSelect, interrupt pin

//LCD in 4bit mode (RS,E,D4,D5,D6,D7)
LiquidCrystal lcd(3, 2, 7, 6, 5, 4);

#define BTN_UP A0 // Button UP
#define BTN_DOWN A1 // Button DOWN
#define BTN_OK A2 // Button OK
#define LCD_ROW 2 // Lcd Row count
#define LCD_CLMN 16 // Lcd Column count
//..............................................................................................................................

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  Serial.begin(115200);
  flashDrive.init();
  lcd.begin(LCD_CLMN,LCD_ROW);
  lcd.print(F("Press OK"));
}

void loop() {
  if(digitalRead(BTN_OK) == LOW){ // Press OK to show files
   delay(100);// debounce
   int fileCnt = 0;// variable to store the total file count
   if(flashDrive.driveReady()){// if drive is attached
	   flashDrive.resetFileList();
		   while(flashDrive.listDir()){
			   fileCnt++; // count all the files + directories
		   }
	   menu(fileCnt);// start menu
	   lcd.clear();
	   lcd.print(F("Press OK"));
   }  //end if drive attached
  }//end if button OK pressed
}//end loop

void menu(int fileCount){
  boolean b_exit = false;
  int cursorPos = 0;//cursor position
  int filePos = 0;//file position
  int oldFilePos = 0;//old file position
  char adatBuffer[25];// max length 255 = 254 char + 1 NULL character
  fileListToLCD(filePos);//lcd print first files

  while(!b_exit){ // stay in while loop as long as b_exit value is false
    if(digitalRead(BTN_DOWN) == LOW && filePos < (fileCount-1)){
      delay(200);
      filePos++;
    }

    if(digitalRead(BTN_UP) == LOW && filePos > 0){
      delay(200);
      filePos--;
    }

    if(filePos != oldFilePos) { // when button pressed
      lcd.setCursor(0,cursorPos);// delete the old cursor
      lcd.print(" ");
      if(filePos > oldFilePos){ //move forward or backward depending on the button press
        cursorPos++;
      } else {
        cursorPos--;
      }
      oldFilePos = filePos;

      if(cursorPos>(LCD_ROW-1) ){// if moved forward and reached the last row
        fileListToLCD(filePos);//get the next files
        cursorPos = 0;// put the cursor on the first line
      } else if(cursorPos < 0){//if moved backward and reached the first row
        flashDrive.resetFileList();//reset the state machine
        searchFileName(filePos);// find the previous file by its number
        fileListToLCD(filePos);//print them on to lcd
        cursorPos = 0;//put the cursor on the first line
        filePos -= (LCD_ROW-1);
        oldFilePos = filePos;
      }

      lcd.setCursor(0,cursorPos);// print cursor to lcd
      lcd.print(">");
    }

    if(digitalRead(BTN_OK) == LOW) {// choose a file and press ok to print content to serial
      flashDrive.resetFileList();//reset the state machine
      while(flashDrive.listDir()){ // find the selected file name by its number
        if(!filePos) break;
        filePos--;
      }
      if(flashDrive.getFileAttrb() == CH376_ATTR_DIRECTORY){// if the selected item is directory the do nothing
        b_exit = true;// done with menu function
      } else {// if the selected item is a valid file then print it to serial
        flashDrive.setFileName();
        flashDrive.openFile();
        while(!flashDrive.getEOF()){ //read until EOF
        	flashDrive.readFile(adatBuffer, sizeof(adatBuffer));
        	Serial.print(adatBuffer);          //print the contents of the temporary buffer
        }
        flashDrive.closeFile();
        b_exit = true;// done with menu function
      }//end if directory
    }//end if OK pressed
  }//end if while
  delay(500);
}

void fileListToLCD(byte a) {
  lcd.clear();
  for (byte i=0; i<LCD_ROW; i++){
    lcd.setCursor(1, i);
    if(flashDrive.listDir()){
      if(flashDrive.getFileAttrb() == CH376_ATTR_DIRECTORY){
        lcd.print('/');
      }
      lcd.print(flashDrive.getFileName());
    } else {
      break;
    }
  }
  if(!a){
    lcd.setCursor(0,0);
    lcd.print(">");
  }

}

void searchFileName(int fileNum){
  fileNum-=(LCD_ROW-1);
    while(fileNum && flashDrive.listDir()){
      fileNum--;
    }
}
