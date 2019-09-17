/*
 * full system / not flat fs
 * directory mode supported
 * parsing  only File variables
 * directory creation: yes
 * parsing: one level at once 
 */

#include <FS.h>
#include "FFat.h"

void setup(){
    Serial.begin(115200);
    delay(5000);
    Serial.printf("Format()\n");
    if (!FFat.format()) {
    Serial.printf("Unable to format(), aborting\n");
    //return;
    }
  Serial.printf("begin()\n");
  if (!FFat.begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }
  File f = FFat.open("/test.txt","w");
  if (f){
      Serial.printf("/test.txt created\n");
      f.print("hello world");
      f.close();
  } else {
         Serial.printf("/test.txt creation failed\n");
  }
  if (FFat.mkdir("/myDir")){
     Serial.printf("/myDir/ created\n"); 
    } else {
   Serial.printf("/myDir directory creation failed\n");   
  }
  if (FFat.mkdir("/myDir2")){
     Serial.printf("/myDir2/ created\n"); 
    } else {
   Serial.printf("/myDir2 directory creation failed\n");   
  }
      f = FFat.open("/myDir/test2.txt","w");
  if (f){
      Serial.printf("/myDir/test2.txt created\n");
      f.print("hello world");
      f.close();
  } else {
         Serial.printf("/myDir/test.txt creation failed\n");
  }
}

void loop(){
    File root = FFat.open("/");
    uint8_t nbf = 0;
    uint8_t nbd = 0;
    File dir = root.openNextFile();
    while (dir) {
      String filename = dir.name();
      size_t fileSize = dir.size();
      if (dir.isDirectory())
        {
            Serial.printf("Dir %s\n",filename.c_str() );
             nbd++;
        }
    if (!dir.isDirectory())
        {
            Serial.printf("File %s %d\n",filename.c_str(), fileSize );
            nbf++;
        }
    dir = root.openNextFile();
  }
    Serial.printf("NB Dir: %d,  NB File: %d\n",nbd, nbf);
    delay(5000);
}
