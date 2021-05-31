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
#define DBG_FILE "FatFileLFN.cpp"
#include "../common/DebugMacros.h"
#include "FatFile.h"
#include "FatVolume.h"
//------------------------------------------------------------------------------
//
uint8_t FatFile::lfnChecksum(uint8_t* name) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < 11; i++) {
    sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + name[i];
  }
  return sum;
}
#if USE_LONG_FILE_NAMES
//------------------------------------------------------------------------------
// Saves about 90 bytes of flash on 328 over tolower().
inline char lfnToLower(char c) {
  return 'A' <= c && c <= 'Z' ? c + 'a' - 'A' : c;
}
//------------------------------------------------------------------------------
// Daniel Bernstein University of Illinois at Chicago.
// Original had + instead of ^
static uint16_t Bernstein(uint16_t hash, const char *str, size_t len) {
  for (size_t i = 0; i < len; i++) {
    // hash = hash * 33 ^ str[i];
    hash = ((hash << 5) + hash) ^ str[i];
  }
  return hash;
}
//------------------------------------------------------------------------------
/**
 * Fetch a 16-bit long file name character.
 *
 * \param[in] ldir Pointer to long file name directory entry.
 * \param[in] i Index of character.
 * \return The 16-bit character.
 */
static uint16_t lfnGetChar(DirLfn_t* ldir, uint8_t i) {
  if (i < 5) {
    return getLe16(ldir->unicode1 + 2*i);
  } else if (i < 11) {
    return getLe16(ldir->unicode2 + 2*i - 10);
  } else if (i < 13) {
    return getLe16(ldir->unicode3 + 2*i - 22);
  }
  return 0;
}
//------------------------------------------------------------------------------
static size_t lfnGetName(DirLfn_t* ldir, char* name, size_t n) {
  uint8_t i;
  size_t k = 13*((ldir->order & 0X1F) - 1);
  for (i = 0; i < 13; i++) {
    uint16_t c = lfnGetChar(ldir, i);
    if (c == 0 || k >= (n - 1)) {
      break;
    }
    name[k++] = c >= 0X7F ? '?' : c;
  }
  // Terminate with zero byte.
  if (k >= n) {
    k = n - 1;
  }
  name[k] = '\0';
  return k;
}
//------------------------------------------------------------------------------
inline bool lfnLegalChar(uint8_t c) {
  if (c == '/' || c == '\\' || c == '"' || c == '*' ||
      c == ':' || c == '<' || c == '>' || c == '?' || c == '|') {
    return false;
  }
  return 0X1F < c && c < 0X7F;
}
//------------------------------------------------------------------------------
/**
 * Store a 16-bit long file name character.
 *
 * \param[in] ldir Pointer to long file name directory entry.
 * \param[in] i Index of character.
 * \param[in] c  The 16-bit character.
 */
static void lfnPutChar(DirLfn_t* ldir, uint8_t i, uint16_t c) {
  if (i < 5) {
    setLe16(ldir->unicode1 + 2*i, c);
  } else if (i < 11) {
    setLe16(ldir->unicode2 + 2*i -10, c);
  } else if (i < 13) {
    setLe16(ldir->unicode3 + 2*i - 22, c);
  }
}
//------------------------------------------------------------------------------
static void lfnPutName(DirLfn_t* ldir, const char* name, size_t n) {
  size_t k = 13*((ldir->order & 0X1F) - 1);
  for (uint8_t i = 0; i < 13; i++, k++) {
    uint16_t c = k < n ? name[k] : k == n ? 0 : 0XFFFF;
    lfnPutChar(ldir, i, c);
  }
}
//==============================================================================
size_t FatFile::getName(char* name, size_t size) {
  size_t n = 0;
  FatFile dirFile;
  DirLfn_t* ldir;
  if (!isOpen() || size < 13) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!isLFN()) {
    return getSFN(name);
  }
  if (!dirFile.openCluster(this)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  for (uint8_t order = 1; order <= m_lfnOrd; order++) {
    if (!dirFile.seekSet(32UL*(m_dirIndex - order))) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    ldir = reinterpret_cast<DirLfn_t*>(dirFile.readDirCache());
    if (!ldir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (ldir->attributes != FAT_ATTRIB_LONG_NAME) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (order != (ldir->order & 0X1F)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    n = lfnGetName(ldir, name, size);
    if (n == 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (ldir->order & FAT_ORDER_LAST_LONG_ENTRY) {
      return n;
    }
  }
  // Fall into fail.
  DBG_FAIL_MACRO;

 fail:
  name[0] = '\0';
  return 0;
}
//------------------------------------------------------------------------------
bool FatFile::openCluster(FatFile* file) {
  if (file->m_dirCluster == 0) {
    return openRoot(file->m_vol);
  }
  memset(this, 0, sizeof(FatFile));
  m_attributes = FILE_ATTR_SUBDIR;
  m_flags = FILE_FLAG_READ;
  m_vol = file->m_vol;
  m_firstCluster = file->m_dirCluster;
  return true;
}
//------------------------------------------------------------------------------
bool FatFile::parsePathName(const char* path,
                            fname_t* fname, const char** ptr) {
  char c;
  bool is83;
  uint8_t bit = FAT_CASE_LC_BASE;
  uint8_t lc = 0;
  uint8_t uc = 0;
  uint8_t i = 0;
  uint8_t in = 7;
  int end;
  int len = 0;
  int si;
  int dot;

  // Skip leading spaces.
  while (*path == ' ') {
    path++;
  }
  fname->lfn = path;

  for (len = 0; ; len++) {
    c = path[len];
    if (c == 0 || isDirSeparator(c)) {
      break;
    }
    if (!lfnLegalChar(c)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Advance to next path component.
  for (end = len; path[end] ==  ' ' || isDirSeparator(path[end]); end++) {}
  *ptr = &path[end];

  // Back over spaces and dots.
  while (len) {
    c = path[len - 1];
    if (c != '.' && c != ' ') {
      break;
    }
    len--;
  }
  // Max length of LFN is 255.
  if (len > 255) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  fname->len = len;
  // Blank file short name.
  for (uint8_t k = 0; k < 11; k++) {
    fname->sfn[k] = ' ';
  }
  // skip leading spaces and dots.
  for (si = 0; path[si] == '.' || path[si] == ' '; si++) {}
  // Not 8.3 if leading dot or space.
  is83 = !si;

  // find last dot.
  for (dot = len - 1; dot >= 0 && path[dot] != '.'; dot--) {}
  for (; si < len; si++) {
    c = path[si];
    if (c == ' ' || (c == '.' && dot != si)) {
      is83 = false;
      continue;
    }
    if (!legal83Char(c) && si != dot) {
      is83 = false;
      c = '_';
    }
    if (si == dot || i > in) {
      if (in == 10) {
        // Done - extension longer than three characters.
        is83 = false;
        break;
      }
      if (si != dot) {
        is83 = false;
      }
      // Break if no dot and base-name is longer than eight characters.
      if (si > dot) {
        break;
      }
      si = dot;
      in = 10;  // Max index for full 8.3 name.
      i = 8;    // Place for extension.
      bit = FAT_CASE_LC_EXT;  // bit for extension.
    } else {
      if ('a' <= c && c <= 'z') {
        c += 'A' - 'a';
        lc |= bit;
      } else if ('A' <= c && c <= 'Z') {
        uc |= bit;
      }
      fname->sfn[i++] = c;
      if (i < 7) {
        fname->seqPos = i;
      }
    }
  }
  if (fname->sfn[0] == ' ') {
    DBG_FAIL_MACRO;
    goto fail;
  }

  if (is83) {
    fname->flags = lc & uc ? FNAME_FLAG_MIXED_CASE : lc;
  } else {
    fname->flags = FNAME_FLAG_LOST_CHARS;
    fname->sfn[fname->seqPos] = '~';
    fname->sfn[fname->seqPos + 1] = '1';
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::open(FatFile* dirFile, fname_t* fname, oflag_t oflag) {
  bool fnameFound = false;
  uint8_t lfnOrd = 0;
  uint8_t freeNeed;
  uint8_t freeFound = 0;
  uint8_t order = 0;
  uint8_t checksum = 0;
  uint8_t ms10;
  uint16_t freeIndex = 0;
  uint16_t curIndex;
  uint16_t date;
  uint16_t time;
  DirFat_t* dir;
  DirLfn_t* ldir;
  size_t len = fname->len;

  if (!dirFile->isDir() || isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Number of directory entries needed.
  freeNeed = fname->flags & FNAME_FLAG_NEED_LFN ? 1 + (len + 12)/13 : 1;

  dirFile->rewind();
  while (1) {
    curIndex = dirFile->m_curPosition/32;
    dir = dirFile->readDirCache(true);
    if (!dir) {
      if (dirFile->getError()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      // At EOF
      goto create;
    }
    if (dir->name[0] == FAT_NAME_DELETED || dir->name[0] == FAT_NAME_FREE) {
      if (freeFound == 0) {
        freeIndex = curIndex;
      }
      if (freeFound < freeNeed) {
        freeFound++;
      }
      if (dir->name[0] == FAT_NAME_FREE) {
        goto create;
      }
    } else {
      if (freeFound < freeNeed) {
        freeFound = 0;
      }
    }
    // skip empty slot or '.' or '..'
    if (dir->name[0] == FAT_NAME_DELETED || dir->name[0] == '.') {
      lfnOrd = 0;
    } else if (isLongName(dir)) {
      ldir = reinterpret_cast<DirLfn_t*>(dir);
      if (!lfnOrd) {
        if ((ldir->order & FAT_ORDER_LAST_LONG_ENTRY) == 0) {
          continue;
        }
        order = ldir->order & 0X1F;
        if (order != (freeNeed - 1)) {
          continue;
        }
        lfnOrd = order;
        checksum = ldir->checksum;
      } else if (ldir->order != --order || checksum != ldir->checksum) {
        lfnOrd = 0;
        continue;
      }
      size_t k = 13*(order - 1);
      if (k >= len) {
        // Not found.
        lfnOrd = 0;
        continue;
      }
      for (uint8_t i = 0; i < 13; i++) {
        uint16_t u = lfnGetChar(ldir, i);
        if (k == len) {
          if (u != 0) {
            // Not found.
            lfnOrd = 0;
          }
          break;
        }
        if (u > 255 || lfnToLower(u) != lfnToLower(fname->lfn[k++])) {
          // Not found.
          lfnOrd = 0;
          break;
        }
      }
    } else if (isFileOrSubdir(dir)) {
      if (lfnOrd) {
        if (1 == order && lfnChecksum(dir->name) == checksum) {
          goto found;
        }
        DBG_FAIL_MACRO;
        goto fail;
      }
      if (!memcmp(dir->name, fname->sfn, sizeof(fname->sfn))) {
        if (!(fname->flags & FNAME_FLAG_LOST_CHARS)) {
          goto found;
        }
        fnameFound = true;
      }
    } else {
      lfnOrd = 0;
    }
  }

 found:
  // Don't open if create only.
  if (oflag & O_EXCL) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  goto open;

 create:
  // don't create unless O_CREAT and write mode
  if (!(oflag & O_CREAT) || !isWriteMode(oflag)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // If at EOF start in next cluster.
  if (freeFound == 0) {
    freeIndex = curIndex;
  }

  while (freeFound < freeNeed) {
    dir = dirFile->readDirCache();
    if (!dir) {
      if (dirFile->getError()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      // EOF if no error.
      break;
    }
    freeFound++;
  }
  while (freeFound < freeNeed) {
    // Will fail if FAT16 root.
    if (!dirFile->addDirCluster()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // Done if more than one sector per cluster.  Max freeNeed is 21.
    if (dirFile->m_vol->sectorsPerCluster() > 1) {
      break;
    }
    freeFound += 16;
  }
  if (fnameFound) {
    if (!dirFile->lfnUniqueSfn(fname)) {
      goto fail;
    }
  }
  if (!dirFile->seekSet(32UL*freeIndex)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  lfnOrd = freeNeed - 1;
  for (order = lfnOrd ; order ; order--) {
    ldir = reinterpret_cast<DirLfn_t*>(dirFile->readDirCache());
    if (!ldir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    dirFile->m_vol->cacheDirty();
    ldir->order = order == lfnOrd ? FAT_ORDER_LAST_LONG_ENTRY | order : order;
    ldir->attributes = FAT_ATTRIB_LONG_NAME;
    ldir->mustBeZero1 = 0;
    ldir->checksum = lfnChecksum(fname->sfn);
    setLe16(ldir->mustBeZero2, 0);
    lfnPutName(ldir, fname->lfn, len);
  }
  curIndex = dirFile->m_curPosition/32;
  dir = dirFile->readDirCache();
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // initialize as empty file
  memset(dir, 0, sizeof(DirFat_t));
  memcpy(dir->name, fname->sfn, 11);

  // Set base-name and extension lower case bits.
  dir->caseFlags =  (FAT_CASE_LC_BASE | FAT_CASE_LC_EXT) & fname->flags;

  // Set timestamps.
  if (FsDateTime::callback) {
    // call user date/time function
    FsDateTime::callback(&date, &time, &ms10);
    setLe16(dir->createDate, date);
    setLe16(dir->createTime, time);
    dir->createTimeMs = ms10;
  } else {
    setLe16(dir->createDate, FS_DEFAULT_DATE);
    setLe16(dir->modifyDate, FS_DEFAULT_DATE);
    setLe16(dir->accessDate, FS_DEFAULT_DATE);
    if (FS_DEFAULT_TIME) {
      setLe16(dir->createTime, FS_DEFAULT_TIME);
      setLe16(dir->modifyTime, FS_DEFAULT_TIME);
    }
  }
  // Force write of entry to device.
  dirFile->m_vol->cacheDirty();

 open:
  // open entry in cache.
  if (!openCachedEntry(dirFile, curIndex, oflag, lfnOrd)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
size_t FatFile::printName(print_t* pr) {
  FatFile dirFile;
  DirLfn_t* ldir;
  size_t n = 0;
  uint16_t u;
  uint8_t buf[13];
  uint8_t i;

  if (!isLFN()) {
    return printSFN(pr);
  }
  if (!dirFile.openCluster(this)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  for (uint8_t order = 1; order <= m_lfnOrd; order++) {
    if (!dirFile.seekSet(32UL*(m_dirIndex - order))) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    ldir = reinterpret_cast<DirLfn_t*>(dirFile.readDirCache());
    if (!ldir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (ldir->attributes != FAT_ATTRIB_LONG_NAME ||
        order != (ldir->order & 0X1F)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    for (i = 0; i < 13; i++) {
      u = lfnGetChar(ldir, i);
      if (u == 0) {
        // End of name.
        break;
      }
      buf[i] = u < 0X7F ? u : '?';
      n++;
    }
    pr->write(buf, i);
  }
  return n;

 fail:
  return 0;
}
//------------------------------------------------------------------------------
bool FatFile::remove() {
  bool last;
  uint8_t checksum;
  FatFile dirFile;
  DirFat_t* dir;
  DirLfn_t* ldir;

  // Cant' remove not open for write.
  if (!isWritable()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Free any clusters.
  if (m_firstCluster && !m_vol->freeChain(m_firstCluster)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Cache directory entry.
  dir = cacheDirEntry(FsCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  checksum = lfnChecksum(dir->name);

  // Mark entry deleted.
  dir->name[0] = FAT_NAME_DELETED;

  // Set this file closed.
  m_attributes = FILE_ATTR_CLOSED;
  m_flags = 0;

  // Write entry to device.
  if (!m_vol->cacheSync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!isLFN()) {
    // Done, no LFN entries.
    return true;
  }
  if (!dirFile.openCluster(this)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  for (uint8_t order = 1; order <= m_lfnOrd; order++) {
    if (!dirFile.seekSet(32UL*(m_dirIndex - order))) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    ldir = reinterpret_cast<DirLfn_t*>(dirFile.readDirCache());
    if (!ldir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (ldir->attributes != FAT_ATTRIB_LONG_NAME ||
        order != (ldir->order & 0X1F) ||
        checksum != ldir->checksum) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    last = ldir->order & FAT_ORDER_LAST_LONG_ENTRY;
    ldir->order = FAT_NAME_DELETED;
    m_vol->cacheDirty();
    if (last) {
      if (!m_vol->cacheSync()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      return true;
    }
  }
  // Fall into fail.
  DBG_FAIL_MACRO;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::lfnUniqueSfn(fname_t* fname) {
  const uint8_t FIRST_HASH_SEQ = 2;  // min value is 2
  uint8_t pos = fname->seqPos;
  DirFat_t* dir;
  uint16_t hex;

  DBG_HALT_IF(!(fname->flags & FNAME_FLAG_LOST_CHARS));
  DBG_HALT_IF(fname->sfn[pos] != '~' && fname->sfn[pos + 1] != '1');

  for (uint8_t seq = 2; seq < 100; seq++) {
    if (seq < FIRST_HASH_SEQ) {
      fname->sfn[pos + 1] = '0' + seq;
    } else {
      DBG_PRINT_IF(seq > FIRST_HASH_SEQ);
      hex = Bernstein(seq + fname->len, fname->lfn, fname->len);
      if (pos > 3) {
        // Make space in name for ~HHHH.
        pos = 3;
      }
      for (uint8_t i = pos + 4 ; i > pos; i--) {
        uint8_t h = hex & 0XF;
        fname->sfn[i] = h < 10 ? h + '0' : h + 'A' - 10;
        hex >>= 4;
      }
    }
    fname->sfn[pos] = '~';
    rewind();
    while (1) {
      dir = readDirCache(true);
      if (!dir) {
        if (!getError()) {
          // At EOF and name not found if no error.
          goto done;
        }
        DBG_FAIL_MACRO;
        goto fail;
      }
      if (dir->name[0] == FAT_NAME_FREE) {
        goto done;
      }
      if (isFileOrSubdir(dir) && !memcmp(fname->sfn, dir->name, 11)) {
        // Name found - try another.
        break;
      }
    }
  }
  // fall inti fail - too many tries.
  DBG_FAIL_MACRO;

 fail:
  return false;

 done:
  return true;
}
#endif  // #if USE_LONG_FILE_NAMES
