/*
 * full system / flat fs
 * directory mode not supported ?
 * parsing need Dir and File variables
 * directory creation: no, need fake dir using . file
 * parsing: all levels at once 
 */

#include <FS.h>

void setup(){
    Serial.begin(115200);
    delay(5000);
    Serial.printf("Format()\n");
   // if (!SPIFFS.format()) {
  //  Serial.printf("Unable to format(), aborting\n");
  //  return;
 // }
  Serial.printf("begin()\n");
  if (!SPIFFS.begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }
  File f = SPIFFS.open("/test.txt","w");
  if (f){
      Serial.printf("/test.txt created\n");
      f.write("hello world", strlen("hello world"));
      f.close();
  } else {
         Serial.printf("/test.txt creation failed\n");
  }
  if (SPIFFS.mkdir("/myDir")){
     Serial.printf("/myDir/ created\n"); 
    } else {
   Serial.printf("/myDir directory creation failed\n");   
  }
      f = SPIFFS.open("/myDir/test2.txt","w");
  if (f){
      Serial.printf("/myDir/test2.txt created\n");
      f.write("hello world", strlen("hello world"));
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
    Dir dir = SPIFFS.openDir("/");
    uint8_t nbf = 0;
    uint8_t nbd = 0;
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      if (dir.isDirectory())
        {
            Serial.printf("Dir %s\n",fileName.c_str() );
             nbd++;
        }
    if (dir.isFile())
        {
            Serial.printf("File %s %d\n",fileName.c_str(), fileSize );
            nbf++;
        }
  }
    Serial.printf("NB Dir: %d,  NB File: %d\n",nbd, nbf);
    delay(5000);
}
