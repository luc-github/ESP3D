/*
 * This sketch will remove the files and directories
 * created by the SdFatMakeDir.pde sketch.
 *
 * Performance is erratic due to the large number
 * of flash erase operations caused by many random
 * writes to file structures.
 */
#include <SdFat.h>

const uint8_t SD_CHIP_SELECT = SS;

SdFat sd;
typedef File file_t;

// store error strings in flash to save RAM
#define error(s) sd.errorHalt(&Serial, F(s))

/*
 * remove all files in dir.
 */
void deleteFiles(FatFile* dir) {
  char name[32];
  file_t file;

  // open and delete files
  for (uint16_t n = 0; ; n++){
     sprintf(name, "%u.TXT", n);

     // open start time
     uint32_t t0 = millis();

     // assume done if open fails
     if (!file.open(dir, name, O_WRONLY)) return;

     // open end time and remove start time
     uint32_t t1 = millis();
     if (!file.remove()) error("file.remove failed");

     // remove end time
     uint32_t t2 = millis();

     Serial.print(F("RM "));
     Serial.print(n);
     Serial.write(' ');

     // open time
     Serial.print(t1 - t0);
     Serial.write(' ');

     // remove time
     Serial.println(t2 - t1);
  }
}

void setup() {
  file_t root;
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo
  Serial.println(F("Type any character to start"));
  while (Serial.read() <= 0) {}
  delay(200);  // Catch Due reset problem

  // initialize the SD card at SPI_FULL_SPEED for best performance.
  // try lower speed if bus errors occur.
  if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) {
    sd.initErrorHalt(&Serial);
  }
  root.openRoot(&sd);
  // delete files in root if not FAT16.
  if (sd.fatType() != 16) {
    Serial.println(F("Remove files in root"));
    deleteFiles(&root);
  }

  // open SUB1 and delete files
  file_t sub1;
  if (!sub1.open("SUB1", O_RDONLY)) error("open SUB1 failed");
  Serial.println(F("Remove files in SUB1"));
  deleteFiles(&sub1);

  // open SUB2 and delete files
  file_t sub2;
  if (!sub2.open(&sub1, "SUB2", O_RDONLY)) error("open SUB2 failed");
  Serial.println(F("Remove files in SUB2"));
  deleteFiles(&sub2);

  // remove SUB2
  if (!sub2.rmdir()) error("sub2.rmdir failed");
  Serial.println(F("SUB2 removed"));

  // remove SUB1
  if (!sub1.rmdir()) error("sub1.rmdir failed");
  Serial.println(F("SUB1 removed"));

  Serial.println(F("Done"));
}

void loop() { }