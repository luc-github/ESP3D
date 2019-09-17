#include <LittleFS.h>
/*
 * full system / not flat
 * directory mode supported
 * parsing need Dir and File variables
 * directory creation / query no `/` at the end
 * parsing: one level at once 
 */

void setup(){
    Serial.begin(115200);
    delay(5000);
    if (!LittleFS.format()) {
    Serial.printf("Unable to format(), aborting\n");
    return;
  }
  if (!LittleFS.begin()) {
    Serial.printf("Unable to begin(), aborting\n");
    return;
  }
  File f = LittleFS.open("/test.txt","w");
  if (f){
      Serial.printf("/test.txt created\n");
      f.write("hello world", strlen("hello world"));
      f.close();
  } else {
         Serial.printf("/test.txt creation failed\n");
  }
  if (LittleFS.mkdir("/myDir")){
      if (LittleFS.mkdir("/myDir/mysubDir")){}
      f = LittleFS.open("/myDir/test2.txt","w");
  if (f){
      Serial.printf("/myDir/test2.txt created\n");
      f.write("hello world", strlen("hello world"));
      f.close();
  } else {
         Serial.printf("/myDir/test.txt creation failed\n");
  }
  } else {
   Serial.printf("/myDir directory creation failed\n");   
  }
  if (!LittleFS.mkdir("/myDir/mysubDir/mysubdir2")){
    Serial.printf("/myDir/mysubDir/mysubdir2 directory creation failed\n");  
  }
}

void loop(){
    Dir dir = LittleFS.openDir("/");
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
