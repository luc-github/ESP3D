/**
 * Copyright (c) 2011-2020 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "FatVolume.h"
#include "FatFile.h"
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//------------------------------------------------------------------------------
static void printHex(print_t* pr, uint8_t h) {
  if (h < 16) {
    pr->write('0');
  }
  pr->print(h, HEX);
}
//------------------------------------------------------------------------------
static void printHex(print_t* pr, uint16_t val) {
  bool space = true;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t h = (val >> (12 - 4*i)) & 15;
    if (h || i == 3) {
      space = false;
    }
    if (space) {
      pr->write(' ');
    } else {
      pr->print(h, HEX);
    }
  }
}
//------------------------------------------------------------------------------
static void printHex(print_t* pr, uint32_t val) {
  bool space = true;
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t h = (val >> (28 - 4*i)) & 15;
    if (h || i == 7) {
      space = false;
    }
    if (space) {
      pr->write(' ');
    } else {
      pr->print(h, HEX);
    }
  }
}
//------------------------------------------------------------------------------
static void printDir(print_t* pr, DirFat_t* dir) {
  if (!dir->name[0] || dir->name[0] == FAT_NAME_DELETED) {
    pr->println(F("Not Used"));
  } else if (isFileOrSubdir(dir)) {
    pr->print(F("name: "));
    pr->write(dir->name, 11);
    pr->println();
    uint32_t fc = ((uint32_t)getLe16(dir->firstClusterHigh) << 16)
                 | getLe16(dir->firstClusterLow);
    pr->print(F("firstCluster: "));
    pr->println(fc, HEX);
    pr->print(F("fileSize: "));
    pr->println(getLe32(dir->fileSize));
  } else if (isLongName(dir)) {
    pr->println(F("LFN"));
  } else {
    pr->println(F("Other"));
  }
}
//------------------------------------------------------------------------------
void FatPartition::dmpDirSector(print_t* pr, uint32_t sector) {
  DirFat_t dir[16];
  if (!readSector(sector, reinterpret_cast<uint8_t*>(dir))) {
    pr->println(F("dmpDir failed"));
    return;
  }
  for (uint8_t i = 0; i < 16; i++) {
    printDir(pr, dir + i);
  }
}
//------------------------------------------------------------------------------
void FatPartition::dmpRootDir(print_t* pr) {
  uint32_t sector;
  if (fatType() == 16) {
    sector = rootDirStart();
  } else if (fatType() == 32) {
    sector = clusterStartSector(rootDirStart());
  } else {
    pr->println(F("dmpRootDir failed"));
    return;
  }
  dmpDirSector(pr, sector);
}
//------------------------------------------------------------------------------
void FatPartition::dmpSector(print_t* pr, uint32_t sector, uint8_t bits) {
  uint8_t data[512];
  if (!readSector(sector, data)) {
    pr->println(F("dmpSector failed"));
    return;
  }
  for (uint16_t i = 0; i < 512;) {
    if (i%32 == 0) {
      if (i) {
        pr->println();
      }
      printHex(pr, i);
    }
    pr->write(' ');
    if (bits == 32) {
      printHex(pr, *reinterpret_cast<uint32_t*>(data + i));
      i += 4;
    } else if (bits == 16) {
      printHex(pr, *reinterpret_cast<uint16_t*>(data + i));
      i += 2;
    } else {
      printHex(pr, data[i++]);
    }
  }
  pr->println();
}
//------------------------------------------------------------------------------
void FatPartition::dmpFat(print_t* pr, uint32_t start, uint32_t count) {
  uint16_t nf = fatType() == 16 ? 256 : fatType() == 32 ? 128 : 0;
  if (nf == 0) {
    pr->println(F("Invalid fatType"));
    return;
  }
  pr->println(F("FAT:"));
  uint32_t sector = m_fatStartSector + start;
  uint32_t cluster = nf*start;
  for (uint32_t i = 0; i < count; i++) {
    cache_t* pc = cacheFetchFat(sector + i, FsCache::CACHE_FOR_READ);
    if (!pc) {
      pr->println(F("cache read failed"));
      return;
    }
    for (size_t k = 0; k < nf; k++) {
      if (0 == cluster%8) {
        if (k) {
          pr->println();
        }
        printHex(pr, cluster);
      }
      cluster++;
      pr->write(' ');
      uint32_t v = fatType() == 32 ? pc->fat32[k] : pc->fat16[k];
      printHex(pr, v);
    }
    pr->println();
  }
}
#endif  // DOXYGEN_SHOULD_SKIP_THIS
