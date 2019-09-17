/*
 * full system / flat fs
 * directory mode not supported
 * parsing only File variable
 * directory creation: no, need fake dir using . file
 * parsing: all levels at once 
 */

#include <FS.h>
#include <SPIFFS.h>

void setup(){
    Serial.begin(115200);
    delay(5000);
    Serial.printf("Format()\n");
    if (!SPIFFS.format()) {
    Serial.printf("Unable to format(), aborting\n");
    return;
    }
  Serial.printf("begin()\n");
  if (!SPIFFS.begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }
  File f = SPIFFS.open("/test.txt","w");
  if (f){
      Serial.printf("/test.txt created\n");
      f.print("hello world");
      f.close();
  } else {
         Serial.printf("/test.txt creation failed\n");
  }
  if (SPIFFS.mkdir("/myDir")){
     Serial.printf("/myDir/ created\n"); 
    } else {
   Serial.printf("/myDir directory creation failed\n");   
  }
  if (SPIFFS.mkdir("/myDir2")){
     Serial.printf("/myDir2/ created\n"); 
    } else {
   Serial.printf("/myDir2 directory creation failed\n");   
  }
      f = SPIFFS.open("/myDir/test2.txt","w");
  if (f){
      Serial.printf("/myDir/test2.txt created\n");
      f.print("hello world");
      f.close();
  } else {
         Serial.printf("/myDir/test.txt creation failed\n");
  }
  f = SPIFFS.open("/myDir/mysubdir/.","w");
  if (f) {
      Serial.printf("/myDir/mysubdir/. created\n");
      f.close();
  } else {
      Serial.printf("/myDir/mysubdir/. creation failed\n");
  }
  
  
}

void loop(){
    File root = SPIFFS.open("/");
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
