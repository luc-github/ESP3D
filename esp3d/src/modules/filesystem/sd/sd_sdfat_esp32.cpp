/*
sd_sdfat_esp32.cpp - ESP3D sd support class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../../../include/esp3d_config.h"
#if defined (ARDUINO_ARCH_ESP32) && defined(SD_DEVICE)
#if (SD_DEVICE == ESP_SDFAT)
#include "../esp_sd.h"
#include "../../../core/genLinkedList.h"
#include "../../../core/settings_esp3d.h"
#include <SdFat.h>
extern File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];

//Max Freq Working
#define FREQMZ 40
SdFat SD;

void dateTime (uint16_t* date, uint16_t* dtime)
{
    struct tm  tmstruct;
    time_t now;
    time (&now);
    localtime_r (&now, &tmstruct);
    *date = FAT_DATE ( (tmstruct.tm_year) + 1900, ( tmstruct.tm_mon) + 1, tmstruct.tm_mday);
    *dtime = FAT_TIME (tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
}

time_t getDateTimeFile(File & filehandle)
{
    static time_t dt = 0;
#ifdef SD_TIMESTAMP_FEATURE
    struct tm timefile;
    dir_t d;
    if(filehandle) {
        if (filehandle.dirEntry(&d)) {
            timefile.tm_year = FAT_YEAR(d.lastWriteDate) - 1900;
            timefile.tm_mon = FAT_MONTH(d.lastWriteDate) - 1;
            timefile.tm_mday = FAT_DAY(d.lastWriteDate);
            timefile.tm_hour = FAT_HOUR(d.lastWriteTime);
            timefile.tm_min = FAT_MINUTE(d.lastWriteTime);
            timefile.tm_sec = FAT_SECOND(d.lastWriteTime);
            timefile.tm_isdst = -1;
            dt =  mktime(&timefile);
            if (dt == -1) {
                log_esp3d("mktime failed");
            }
        } else {
            log_esp3d("stat file failed");
        }
    } else {
        log_esp3d("check stat file failed");
    }
#endif //SD_TIMESTAMP_FEATURE
    return dt;
}


uint8_t ESP_SD::getState(bool refresh)
{
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
    //no need to go further if SD detect is not correct
    if (!((digitalRead (ESP_SD_DETECT_PIN) == ESP_SD_DETECT_VALUE) ? true : false)) {
        _state = ESP_SDCARD_NOT_PRESENT;
        return _state;
    }
#endif  //ESP_SD_DETECT_PIN
    //if busy doing something return state
    if (!((_state == ESP_SDCARD_NOT_PRESENT) || _state == ESP_SDCARD_IDLE)) {
        return _state;
    }
    if (!refresh) {
        return _state;  //to avoid refresh=true + busy to reset SD and waste time
    } else {
        _sizechanged = true;
    }
    //SD is idle or not detected, let see if still the case
    _state = ESP_SDCARD_NOT_PRESENT;
    log_esp3d("Spi : CS: %d,  Miso: %d, Mosi: %d, SCK: %d",ESP_SD_CS_PIN!=-1?ESP_SD_CS_PIN:SS, ESP_SD_MISO_PIN!=-1?ESP_SD_MISO_PIN:MISO, ESP_SD_MOSI_PIN!=-1?ESP_SD_MOSI_PIN:MOSI, ESP_SD_SCK_PIN!=-1?ESP_SD_SCK_PIN:SCK);
    //refresh content if card was removed
    if (SD.begin((ESP_SD_CS_PIN == -1)?SS:ESP_SD_CS_PIN, SD_SCK_MHZ(FREQMZ/_spi_speed_divider))) {
        if (SD.card()->cardSize() > 0 ) {
            _state = ESP_SDCARD_IDLE;
        }
    }
    return _state;
}

bool ESP_SD::begin()
{
#if (ESP_SD_CS_PIN != -1) || (ESP_SD_MISO_PIN != -1) || (ESP_SD_MOSI_PIN != -1) || (ESP_SD_SCK_PIN != -1)
    SPI.begin(ESP_SD_SCK_PIN, ESP_SD_MISO_PIN, ESP_SD_MOSI_PIN, ESP_SD_CS_PIN);
#endif
    _started = true;
    _state = ESP_SDCARD_NOT_PRESENT;
    _spi_speed_divider = Settings_ESP3D::read_byte(ESP_SD_SPEED_DIV);
    //sanity check
    if (_spi_speed_divider <= 0) {
        _spi_speed_divider = 1;
    }
#ifdef SD_TIMESTAMP_FEATURE
    //set callback to get time on files on SD
    SdFile::dateTimeCallback (dateTime);
#endif //SD_TIMESTAMP_FEATURE
//Setup pins
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
    pinMode (ESP_SD_DETECT_PIN, INPUT);
#endif //ESP_SD_DETECT_PIN
#if SD_DEVICE_CONNECTION  == ESP_SHARED_SD
#if defined(ESP_FLAG_SHARED_SD_PIN) && ESP_FLAG_SHARED_SD_PIN != -1
    pinMode (ESP_FLAG_SHARED_SD_PIN, OUTPUT);
    digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
#endif //ESP_FLAG_SHARED_SD_PIN
#endif //SD_DEVICE_CONNECTION  == ESP_SHARED_SD
    return _started;
}

void ESP_SD::end()
{
    _state = ESP_SDCARD_NOT_PRESENT;
    _started = false;
}

void ESP_SD::refreshStats(bool force)
{
    if (force || _sizechanged) {
        usedBytes(true);
    }
    _sizechanged = false;
}

uint64_t ESP_SD::totalBytes(bool refresh)
{
    static uint64_t _totalBytes = 0;
    if (refresh || _totalBytes==0) {
        _totalBytes = SD.vol()->clusterCount();
        uint8_t blocks = SD.vol()->blocksPerCluster();
        _totalBytes = _totalBytes * blocks * 512;
    }
    return _totalBytes;
}

uint64_t ESP_SD::usedBytes(bool refresh)
{
    return totalBytes(refresh) - freeBytes(refresh);
}

uint64_t ESP_SD::freeBytes(bool refresh)
{
    static uint64_t _freeBytes = 0;
    if (refresh || _freeBytes==0) {
        uint64_t volFree = SD.vol()->freeClusterCount();
        uint8_t blocks = SD.vol()->blocksPerCluster();
        _freeBytes = volFree * blocks * 512;
    }
    return _freeBytes;
}

uint ESP_SD::maxPathLength()
{
    return 255;
}

bool ESP_SD::rename(const char *oldpath, const char *newpath)
{
    return SD.rename(oldpath,newpath);
}

//  strings needed in file system structures
#define noName "NO NAME    "
#define fat16str "FAT16   "
#define fat32str "FAT32   "
// constants for file system structure
#define BU16 128
#define BU32 8192
#define ERASE_SIZE 262144L

//------------------------------------------------------------------------------
// write cached block to the card
uint8_t writeCache(uint32_t lbn, Sd2Card & card, cache_t & cache)
{
    return card.writeBlock(lbn, cache.data);
}

//------------------------------------------------------------------------------
// initialize appropriate sizes for SD capacity
bool initSizes(uint32_t cardCapacityMB, uint8_t & sectorsPerCluster, uint8_t & numberOfHeads, uint8_t & sectorsPerTrack)
{
    if (cardCapacityMB <= 6) {
        return false;
    } else if (cardCapacityMB <= 16) {
        sectorsPerCluster = 2;
    } else if (cardCapacityMB <= 32) {
        sectorsPerCluster = 4;
    } else if (cardCapacityMB <= 64) {
        sectorsPerCluster = 8;
    } else if (cardCapacityMB <= 128) {
        sectorsPerCluster = 16;
    } else if (cardCapacityMB <= 1024) {
        sectorsPerCluster = 32;
    } else if (cardCapacityMB <= 32768) {
        sectorsPerCluster = 64;
    } else {
        // SDXC cards
        sectorsPerCluster = 128;
    }

    // set fake disk geometry
    sectorsPerTrack = cardCapacityMB <= 256 ? 32 : 63;

    if (cardCapacityMB <= 16) {
        numberOfHeads = 2;
    } else if (cardCapacityMB <= 32) {
        numberOfHeads = 4;
    } else if (cardCapacityMB <= 128) {
        numberOfHeads = 8;
    } else if (cardCapacityMB <= 504) {
        numberOfHeads = 16;
    } else if (cardCapacityMB <= 1008) {
        numberOfHeads = 32;
    } else if (cardCapacityMB <= 2016) {
        numberOfHeads = 64;
    } else if (cardCapacityMB <= 4032) {
        numberOfHeads = 128;
    } else {
        numberOfHeads = 255;
    }
    return true;
}

//------------------------------------------------------------------------------
// zero cache and optionally set the sector signature
void clearCache(uint8_t addSig, cache_t & cache)
{
    memset(&cache, 0, sizeof(cache));
    if (addSig) {
        cache.mbr.mbrSig0 = BOOTSIG0;
        cache.mbr.mbrSig1 = BOOTSIG1;
    }
}
//------------------------------------------------------------------------------
// zero FAT and root dir area on SD
bool clearFatDir(uint32_t bgn, uint32_t count, Sd2Card & card, cache_t & cache, ESP3DOutput * output)
{
    clearCache(false, cache);
    if (!card.writeStart(bgn, count)) {
        return false;
    }
    for (uint32_t i = 0; i < count; i++) {
        if ((i & 0XFF) == 0) {
            if (output) {
                output->print(".");
            }
        }
        if (!card.writeData(cache.data)) {
            return false;
        }
    }
    if (!card.writeStop()) {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
// return cylinder number for a logical block number
uint16_t lbnToCylinder(uint32_t lbn, uint8_t numberOfHeads, uint8_t sectorsPerTrack)
{
    return lbn / (numberOfHeads * sectorsPerTrack);
}
//------------------------------------------------------------------------------
// return head number for a logical block number
uint8_t lbnToHead(uint32_t lbn, uint8_t numberOfHeads, uint8_t sectorsPerTrack)
{
    return (lbn % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
}
//------------------------------------------------------------------------------
// return sector number for a logical block number
uint8_t lbnToSector(uint32_t lbn, uint8_t sectorsPerTrack)
{
    return (lbn % sectorsPerTrack) + 1;
}

//------------------------------------------------------------------------------
// format and write the Master Boot Record
bool writeMbr(Sd2Card & card, cache_t & cache, uint8_t partType, uint32_t relSector, uint32_t partSize, uint8_t numberOfHeads, uint8_t sectorsPerTrack)
{
    clearCache(true, cache);
    part_t* p = cache.mbr.part;
    p->boot = 0;
    uint16_t c = lbnToCylinder(relSector, numberOfHeads, sectorsPerTrack);
    if (c > 1023) {
        return false;
    }
    p->beginCylinderHigh = c >> 8;
    p->beginCylinderLow = c & 0XFF;
    p->beginHead = lbnToHead(relSector, numberOfHeads, sectorsPerTrack);
    p->beginSector = lbnToSector(relSector, sectorsPerTrack);
    p->type = partType;
    uint32_t endLbn = relSector + partSize - 1;
    c = lbnToCylinder(endLbn,numberOfHeads, sectorsPerTrack);
    if (c <= 1023) {
        p->endCylinderHigh = c >> 8;
        p->endCylinderLow = c & 0XFF;
        p->endHead = lbnToHead(endLbn, numberOfHeads, sectorsPerTrack);
        p->endSector = lbnToSector(endLbn, sectorsPerTrack);
    } else {
        // Too big flag, c = 1023, h = 254, s = 63
        p->endCylinderHigh = 3;
        p->endCylinderLow = 255;
        p->endHead = 254;
        p->endSector = 63;
    }
    p->firstSector = relSector;
    p->totalSectors = partSize;
    if (!writeCache(0, card, cache)) {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
// generate serial number from card size and micros since boot
uint32_t volSerialNumber(uint32_t cardSizeBlocks)
{
    return (cardSizeBlocks << 8) + micros();
}

// format the SD as FAT16
bool makeFat16(uint32_t & dataStart, Sd2Card & card, cache_t & cache, uint8_t numberOfHeads, uint8_t sectorsPerTrack, uint32_t cardSizeBlocks, uint8_t sectorsPerCluster, uint32_t &relSector, uint32_t partSize, uint8_t & partType, uint32_t &fatSize, uint32_t &fatStart, uint16_t reservedSectors, ESP3DOutput * output)
{
    uint32_t nc;
    for (dataStart = 2 * BU16;; dataStart += BU16) {
        nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
        fatSize = (nc + 2 + 255)/256;
        uint32_t r = BU16 + 1 + 2 * fatSize + 32;
        if (dataStart < r) {
            continue;
        }
        relSector = dataStart - r + BU16;
        break;
    }
    // check valid cluster count for FAT16 volume
    if (nc < 4085 || nc >= 65525) {
        return false;
    }
    reservedSectors = 1;
    fatStart = relSector + reservedSectors;
    partSize = nc * sectorsPerCluster + 2 * fatSize + reservedSectors + 32;
    if (partSize < 32680) {
        partType = 0X01;
    } else if (partSize < 65536) {
        partType = 0X04;
    } else {
        partType = 0X06;
    }
    // write MBR
    if (!writeMbr(card, cache, partType, relSector, partSize, numberOfHeads, sectorsPerTrack)) {
        return false;
    }
    clearCache(true, cache);
    fat_boot_t* pb = &cache.fbs;
    pb->jump[0] = 0XEB;
    pb->jump[1] = 0X00;
    pb->jump[2] = 0X90;
    for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
        pb->oemId[i] = ' ';
    }
    pb->bytesPerSector = 512;
    pb->sectorsPerCluster = sectorsPerCluster;
    pb->reservedSectorCount = reservedSectors;
    pb->fatCount = 2;
    pb->rootDirEntryCount = 512;
    pb->mediaType = 0XF8;
    pb->sectorsPerFat16 = fatSize;
    pb->sectorsPerTrack = sectorsPerTrack;
    pb->headCount = numberOfHeads;
    pb->hidddenSectors = relSector;
    pb->totalSectors32 = partSize;
    pb->driveNumber = 0X80;
    pb->bootSignature = EXTENDED_BOOT_SIG;
    pb->volumeSerialNumber = volSerialNumber(cardSizeBlocks);
    memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
    memcpy(pb->fileSystemType, fat16str, sizeof(pb->fileSystemType));
    // write partition boot sector
    if (!writeCache(relSector, card, cache)) {
        return false;
    }
    // clear FAT and root directory
    clearFatDir(fatStart, dataStart - fatStart, card, cache, output);
    clearCache(false, cache);
    cache.fat16[0] = 0XFFF8;
    cache.fat16[1] = 0XFFFF;
    // write first block of FAT and backup for reserved clusters
    if (!writeCache(fatStart, card, cache)
            || !writeCache(fatStart + fatSize, card, cache)) {
        return false;
    }
    return true;
}

// format the SD as FAT32
bool makeFat32(uint32_t & dataStart, Sd2Card & card, cache_t & cache, uint8_t numberOfHeads, uint8_t sectorsPerTrack, uint32_t cardSizeBlocks, uint8_t sectorsPerCluster, uint32_t &relSector, uint32_t partSize, uint8_t & partType, uint32_t &fatSize, uint32_t &fatStart, uint16_t reservedSectors, ESP3DOutput * output)
{
    uint32_t nc;
    relSector = BU32;
    for (dataStart = 2 * BU32;; dataStart += BU32) {
        nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
        fatSize = (nc + 2 + 127)/128;
        uint32_t r = relSector + 9 + 2 * fatSize;
        if (dataStart >= r) {
            break;
        }
    }
    // error if too few clusters in FAT32 volume
    if (nc < 65525) {
        return false;
    }
    reservedSectors = dataStart - relSector - 2 * fatSize;
    fatStart = relSector + reservedSectors;
    partSize = nc * sectorsPerCluster + dataStart - relSector;
    // type depends on address of end sector
    // max CHS has lbn = 16450560 = 1024*255*63
    if ((relSector + partSize) <= 16450560) {
        // FAT32
        partType = 0X0B;
    } else {
        // FAT32 with INT 13
        partType = 0X0C;
    }
    if (!writeMbr(card, cache, partType, relSector, partSize, numberOfHeads, sectorsPerTrack)) {
        return false;
    }
    clearCache(true, cache);

    fat32_boot_t* pb = &cache.fbs32;
    pb->jump[0] = 0XEB;
    pb->jump[1] = 0X00;
    pb->jump[2] = 0X90;
    for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
        pb->oemId[i] = ' ';
    }
    pb->bytesPerSector = 512;
    pb->sectorsPerCluster = sectorsPerCluster;
    pb->reservedSectorCount = reservedSectors;
    pb->fatCount = 2;
    pb->mediaType = 0XF8;
    pb->sectorsPerTrack = sectorsPerTrack;
    pb->headCount = numberOfHeads;
    pb->hidddenSectors = relSector;
    pb->totalSectors32 = partSize;
    pb->sectorsPerFat32 = fatSize;
    pb->fat32RootCluster = 2;
    pb->fat32FSInfo = 1;
    pb->fat32BackBootBlock = 6;
    pb->driveNumber = 0X80;
    pb->bootSignature = EXTENDED_BOOT_SIG;
    pb->volumeSerialNumber = volSerialNumber(cardSizeBlocks);
    memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
    memcpy(pb->fileSystemType, fat32str, sizeof(pb->fileSystemType));
    // write partition boot sector and backup
    if (!writeCache(relSector, card, cache)
            || !writeCache(relSector + 6, card, cache)) {
        return false;
    }
    clearCache(true, cache);
    // write extra boot area and backup
    if (!writeCache(relSector + 2, card, cache)
            || !writeCache(relSector + 8, card, cache)) {
        return false;
    }
    fat32_fsinfo_t* pf = &cache.fsinfo;
    pf->leadSignature = FSINFO_LEAD_SIG;
    pf->structSignature = FSINFO_STRUCT_SIG;
    pf->freeCount = 0XFFFFFFFF;
    pf->nextFree = 0XFFFFFFFF;
    // write FSINFO sector and backup
    if (!writeCache(relSector + 1, card, cache)
            || !writeCache(relSector + 7, card, cache)) {
        return false;
    }
    clearFatDir(fatStart, 2 * fatSize + sectorsPerCluster, card, cache, output);
    clearCache(false, cache);
    cache.fat32[0] = 0x0FFFFFF8;
    cache.fat32[1] = 0x0FFFFFFF;
    cache.fat32[2] = 0x0FFFFFFF;
    // write first block of FAT and backup for reserved clusters
    if (!writeCache(fatStart, card, cache)
            || !writeCache(fatStart + fatSize, card, cache)) {
        return false;
    }
    return true;
}

bool eraseCard(Sd2Card & card, cache_t & cache, uint32_t cardSizeBlocks, ESP3DOutput * output)
{
    uint32_t firstBlock = 0;
    uint32_t lastBlock = 0;
    //uint16_t n = 0;
    if (output) {
        output->printMSG("Erasing ", false);
    }
    do {
        lastBlock = firstBlock + ERASE_SIZE - 1;
        if (lastBlock >= cardSizeBlocks) {
            lastBlock = cardSizeBlocks - 1;
        }
        if (!card.erase(firstBlock, lastBlock)) {
            return false;
        }
        if (output) {
            output->print(".");
        }
        firstBlock += ERASE_SIZE;
    } while (firstBlock < cardSizeBlocks);

    if (!card.readBlock(0, cache.data)) {
        return false;
    }
    if (output) {
        output->printLN("");
    }
    return true;
}

bool formatCard(uint32_t & dataStart, Sd2Card & card,
                cache_t & cache, uint8_t numberOfHeads,
                uint8_t sectorsPerTrack, uint32_t cardSizeBlocks,
                uint8_t sectorsPerCluster, uint32_t &relSector,
                uint32_t partSize, uint8_t & partType,
                uint32_t &fatSize, uint32_t &fatStart,
                uint32_t cardCapacityMB, uint16_t reservedSectors, ESP3DOutput * output)
{
    initSizes(cardCapacityMB, sectorsPerCluster, numberOfHeads, sectorsPerTrack);
    if (card.type() != SD_CARD_TYPE_SDHC) {
        if (output) {
            output->printMSG("Formating FAT16 ");
        }
        if(!makeFat16(dataStart, card, cache, numberOfHeads, sectorsPerTrack, cardSizeBlocks, sectorsPerCluster, relSector, partSize, partType, fatSize, fatStart, reservedSectors, output)) {
            return false;
        }
    } else {
        if (output) {
            output->printMSG("Formating FAT32 ", false);
        }
        if(!makeFat32(dataStart, card, cache, numberOfHeads, sectorsPerTrack, cardSizeBlocks, sectorsPerCluster, relSector, partSize, partType, fatSize, fatStart, reservedSectors, output)) {
            return false;
        }
    }
    if (output) {
        output->printLN("");
    }
    return true;
}

bool ESP_SD::format(ESP3DOutput * output)
{
    if (ESP_SD::getState(true) == ESP_SDCARD_IDLE) {
        Sd2Card card;
        uint32_t cardSizeBlocks;
        uint32_t cardCapacityMB;
        // cache for SD block
        cache_t cache;

        // MBR information
        uint8_t partType = 0;
        uint32_t relSector = 0;
        uint32_t partSize = 0;

        // Fake disk geometry
        uint8_t numberOfHeads = 0;
        uint8_t sectorsPerTrack = 0;

        // FAT parameters
        uint16_t reservedSectors = 0;
        uint8_t sectorsPerCluster = 0;
        uint32_t fatStart = 0;
        uint32_t fatSize = 0;
        uint32_t dataStart = 0;
        if (!card.begin((ESP_SD_CS_PIN == -1)?SS:ESP_SD_CS_PIN, SD_SCK_MHZ(FREQMZ/_spi_speed_divider))) {
            return false;
        }
        cardSizeBlocks = card.cardSize();
        if (cardSizeBlocks == 0) {
            return false;
        }
        cardCapacityMB = (cardSizeBlocks + 2047)/2048;
        if (output) {
            String s = "Capacity detected :" + String((1.048576*cardCapacityMB)/1024) + "GB";
            output->printMSG(s.c_str());
        }
        if (!eraseCard(card, cache, cardSizeBlocks, output)) {
            return false;
        }

        if (!formatCard(dataStart, card, cache, numberOfHeads,
                        sectorsPerTrack, cardSizeBlocks,
                        sectorsPerCluster, relSector, partSize, partType,
                        fatSize, fatStart, cardCapacityMB, reservedSectors,output)) {
            return false;
        }
        return true;
    }
    return false;
}

ESP_SDFile ESP_SD::open(const char* path, uint8_t mode)
{
    //do some check
    if(((strcmp(path,"/") == 0) && ((mode == ESP_FILE_WRITE) || (mode == ESP_FILE_APPEND))) || (strlen(path) == 0)) {
        _sizechanged = true;
        return ESP_SDFile();
    }
    // path must start by '/'
    if (path[0] != '/') {
        return ESP_SDFile();
    }
    if (mode != ESP_FILE_READ) {
        //check container exists
        String p = path;
        p.remove(p.lastIndexOf('/') +1);
        if (!exists(p.c_str())) {
            log_esp3d("Error opening: %s", path);
            return ESP_SDFile();
        }
    }
    File tmp = SD.open(path, (mode == ESP_FILE_READ)?FILE_READ:(mode == ESP_FILE_WRITE)?FILE_WRITE:FILE_WRITE);
    ESP_SDFile esptmp(&tmp, tmp.isDir(),(mode == ESP_FILE_READ)?false:true, path);
    return esptmp;
}

bool ESP_SD::exists(const char* path)
{
    bool res = false;
    //root should always be there if started
    if (strcmp(path, "/") == 0) {
        return _started;
    }
    res = SD.exists(path);
    if (!res) {
        ESP_SDFile root = ESP_SD::open(path, ESP_FILE_READ);
        if (root) {
            res = root.isDirectory();
        }
    }
    return res;
}

bool ESP_SD::remove(const char *path)
{
    _sizechanged = true;
    return SD.remove(path);
}

bool ESP_SD::mkdir(const char *path)
{
    return SD.mkdir(path);
}

bool ESP_SD::rmdir(const char *path)
{
    if (!exists(path)) {
        return false;
    }
    bool res = true;
    GenLinkedList<String > pathlist;
    String p = path;
    pathlist.push(p);
    while (pathlist.count() > 0) {
        File dir = SD.open(pathlist.getLast().c_str());
        dir.rewindDirectory();
        File f = dir.openNextFile();
        bool candelete = true;
        while (f) {
            if (f.isDir()) {
                candelete = false;
                String newdir;
                char tmp[255];
                f.getName(tmp,254);
                newdir = tmp;
                pathlist.push(newdir);
                f.close();
                f = File();
            } else {
                char tmp[255];
                f.getName(tmp,254);
                _sizechanged = true;
                SD.remove(tmp);
                f.close();
                f = dir.openNextFile();
            }
        }
        if (candelete) {
            if (pathlist.getLast() !="/") {
                res = SD.rmdir(pathlist.getLast().c_str());
            }
            pathlist.pop();
        }
        dir.close();
    }
    p = String();
    log_esp3d("count %d", pathlist.count());
    return res;
}

void ESP_SD::closeAll()
{
    for (uint8_t i = 0; i < ESP_MAX_SD_OPENHANDLE; i++) {
        tSDFile_handle[i].close();
        tSDFile_handle[i] = File();
    }
}

bool ESP_SDFile::seek(uint32_t pos, uint8_t mode)
{
    if (mode == ESP_SEEK_END) {
        return tSDFile_handle[_index].seek(-pos);    //based on SDFS comment
    }
    return tSDFile_handle[_index].seek(pos);
}

ESP_SDFile::ESP_SDFile(void* handle, bool isdir, bool iswritemode, const char * path)
{
    _isdir = isdir;
    _dirlist = "";
    _index = -1;
    _filename = "";
    _name = "";
    _lastwrite = 0 ;
    _iswritemode = iswritemode;
    _size = 0;
    if (!handle) {
        return ;
    }
    bool set =false;
    for (uint8_t i=0; (i < ESP_MAX_SD_OPENHANDLE) && !set; i++) {
        if (!tSDFile_handle[i]) {
            tSDFile_handle[i] = *((File*)handle);
            //filename
            char tmp[255];
            tSDFile_handle[i].getName(tmp,254);
            _filename = path;
            //name
            _name = tmp;
            if (_name.endsWith("/")) {
                _name.remove( _name.length() - 1,1);
                _isdir = true;
            }
            if (_name[0] == '/') {
                _name.remove( 0, 1);
            }
            int pos = _name.lastIndexOf('/');
            if (pos != -1) {
                _name.remove( 0, pos+1);
            }
            if (_name.length() == 0) {
                _name = "/";
            }
            //size
            _size = tSDFile_handle[i].size();
            //time
            if (!_isdir && !iswritemode) {
                _lastwrite = getDateTimeFile(tSDFile_handle[i]);

            } else {
                //no need date time for directory
                _lastwrite = 0;
            }
            _index = i;
            //log_esp3d("Opening File at index %d",_index);
            set = true;
        }
    }
}
//todo need also to add short filename
const char* ESP_SDFile::shortname() const
{
    static char sname[13];
    File ftmp = SD.open(_filename.c_str());
    if (ftmp) {
        ftmp.getSFN(sname);
        ftmp.close();
        return sname;
    } else {
        return _name.c_str();
    }
}

void ESP_SDFile::close()
{
    if (_index != -1) {
        //log_esp3d("Closing File at index %d", _index);
        tSDFile_handle[_index].close();
        //reopen if mode = write
        //udate size + date
        if (_iswritemode && !_isdir) {
            File ftmp = SD.open(_filename.c_str());
            if (ftmp) {
                _size = ftmp.size();
                _lastwrite = getDateTimeFile(ftmp);
                ftmp.close();
            }
        }
        tSDFile_handle[_index] = File();
        //log_esp3d("Closing File at index %d",_index);
        _index = -1;
    }
}

ESP_SDFile  ESP_SDFile::openNextFile()
{
    if ((_index == -1) || !_isdir) {
        log_esp3d("openNextFile failed");
        return ESP_SDFile();
    }
    File tmp = tSDFile_handle[_index].openNextFile();
    if (tmp) {
        char tmps[255];
        tmp.getName(tmps,254);
        log_esp3d("tmp name :%s %s", tmps, (tmp.isDir())?"isDir":"isFile");
        String s = _filename ;
        if (s!="/") {
            s+="/";
        }
        s += tmps;
        ESP_SDFile esptmp(&tmp, tmp.isDir(),false, s.c_str());
        esptmp.close();
        return esptmp;
    }
    return  ESP_SDFile();
}

const char * ESP_SD::FilesystemName()
{
    return "SDFat - " SD_FAT_VERSION_STR ;
}

#endif //SD_DEVICE == ESP_SDFAT
#endif //ARCH_ESP32 && SD_DEVICE
