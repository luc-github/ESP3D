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
#define DBG_FILE "ExFatFile.cpp"
#include "../common/DebugMacros.h"
#include "ExFatFile.h"
#include "ExFatVolume.h"
#include "upcase.h"
//------------------------------------------------------------------------------
bool ExFatFile::close() {
  bool rtn = sync();
  m_attributes = FILE_ATTR_CLOSED;
  m_flags = 0;
  return rtn;
}
//------------------------------------------------------------------------------
bool ExFatFile::contiguousRange(uint32_t* bgnSector, uint32_t* endSector) {
  if (!isContiguous()) {
    return false;
  }
  if (bgnSector) {
    *bgnSector = firstSector();
  }
  if (endSector) {
    *endSector = firstSector() +
                 ((m_validLength - 1) >> m_vol->bytesPerSectorShift());
  }
  return true;
}
//------------------------------------------------------------------------------
void ExFatFile::fgetpos(fspos_t* pos) const {
  pos->position = m_curPosition;
  pos->cluster = m_curCluster;
}
//------------------------------------------------------------------------------
int ExFatFile::fgets(char* str, int num, char* delim) {
  char ch;
  int n = 0;
  int r = -1;
  while ((n + 1) < num && (r = read(&ch, 1)) == 1) {
    // delete CR
    if (ch == '\r') {
      continue;
    }
    str[n++] = ch;
    if (!delim) {
      if (ch == '\n') {
        break;
      }
    } else {
      if (strchr(delim, ch)) {
        break;
      }
    }
  }
  if (r < 0) {
    // read error
    return -1;
  }
  str[n] = '\0';
  return n;
}
//------------------------------------------------------------------------------
uint32_t ExFatFile::firstSector() const {
  return m_firstCluster ? m_vol->clusterStartSector(m_firstCluster) : 0;
}
//------------------------------------------------------------------------------
void ExFatFile::fsetpos(const fspos_t* pos) {
  m_curPosition = pos->position;
  m_curCluster = pos->cluster;
}
//------------------------------------------------------------------------------
bool ExFatFile::getAccessDateTime(uint16_t* pdate, uint16_t* ptime) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  *pdate = getLe16(df->accessDate);
  *ptime = getLe16(df->accessTime);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::getCreateDateTime(uint16_t* pdate, uint16_t* ptime) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  *pdate = getLe16(df->createDate);
  *ptime = getLe16(df->createTime);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::getModifyDateTime(uint16_t* pdate, uint16_t* ptime) {
  DirFile_t* df = reinterpret_cast<DirFile_t*>
                 (m_vol->dirCache(&m_dirPos, FsCache::CACHE_FOR_READ));
  if (!df) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  *pdate = getLe16(df->modifyDate);
  *ptime = getLe16(df->modifyTime);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
size_t ExFatFile::getName(ExChar_t* name, size_t length) {
  DirName_t* dn;
  DirPos_t pos = m_dirPos;
  size_t n = 0;
  if (!isOpen()) {
      DBG_FAIL_MACRO;
      goto fail;
  }
  for (uint8_t is = 1; is < m_setCount; is++) {
    if (m_vol->dirSeek(&pos, is == 1 ? 64: 32) != 1) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    dn = reinterpret_cast<DirName_t*>
         (m_vol->dirCache(&pos, FsCache::CACHE_FOR_READ));
    if (!dn || dn->type != EXFAT_TYPE_NAME) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    for (uint8_t in = 0; in < 15; in++) {
      uint16_t c = getLe16(dn->unicode + 2*in);
      if (c == 0 || (n + 1) >= length) {
        goto done;
      }
      name[n++] = sizeof(ExChar_t) > 1 || c < 0X7F ? c : '?';
    }
  }
 done:
  name[n] = 0;
  return n;

 fail:
  *name = 0;
  return 0;
}
//------------------------------------------------------------------------------
bool ExFatFile::isBusy() {
  return m_vol->isBusy();
}
//------------------------------------------------------------------------------
bool ExFatFile::open(const ExChar_t* path, int oflag) {
  return open(ExFatVolume::cwv(), path, oflag);
}
//------------------------------------------------------------------------------
bool ExFatFile::open(ExFatVolume* vol, const ExChar_t* path, int oflag) {
  return vol && open(vol->vwd(), path, oflag);
}
//------------------------------------------------------------------------------
bool ExFatFile::open(ExFatFile* dirFile, const ExChar_t* path, oflag_t oflag) {
  ExFatFile tmpDir;
  ExName_t fname;
  // error if already open
  if (isOpen() || !dirFile->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isDirSeparator(*path)) {
    while (isDirSeparator(*path)) {
      path++;
    }
    if (*path == 0) {
      return openRoot(dirFile->m_vol);
    }
    if (!tmpDir.openRoot(dirFile->m_vol)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    dirFile = &tmpDir;
  }
  while (1) {
    if (!parsePathName(path, &fname, &path)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (*path == 0) {
      break;
    }
    if (!open(dirFile, &fname, O_RDONLY)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    tmpDir = *this;
    dirFile = &tmpDir;
    close();
  }
  return open(dirFile, &fname, oflag);

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::open(ExFatFile* dirFile, uint32_t index, oflag_t oflag) {
  if (dirFile->seekSet(32*index) && openNext(dirFile, oflag)) {
    if (dirIndex() == index) {
      return true;
    }
    close();
    DBG_FAIL_MACRO;
  }
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::openNext(ExFatFile* dir, oflag_t oflag) {
  if (isOpen() || !dir->isDir() || (dir->curPosition() & 0X1F)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return openRootFile(dir, nullptr, 0, oflag);

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::openRootFile(ExFatFile* dir, const ExChar_t* name,
                          uint8_t nameLength, oflag_t oflag) {
  int n;
  uint8_t nameOffset = 0;
  uint8_t nCmp;
  uint8_t modeFlags;
  uint16_t nameHash = 0;
  uint32_t curCluster __attribute__((unused));
  uint8_t* cache __attribute__((unused));
  DirPos_t freePos __attribute__((unused));

  DirFile_t*   dirFile;
  DirStream_t* dirStream;
  DirName_t*   dirName;
  uint8_t buf[32];
  uint8_t freeCount = 0;
  uint8_t freeNeed;
  bool inSet = false;

  // error if already open
  if (isOpen() || !dir->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  switch (oflag & O_ACCMODE) {
    case O_RDONLY:
      modeFlags = FILE_FLAG_READ;
      break;
    case O_WRONLY:
      modeFlags = FILE_FLAG_WRITE;
      break;
    case O_RDWR:
      modeFlags = FILE_FLAG_READ | FILE_FLAG_WRITE;
      break;
    default:
      DBG_FAIL_MACRO;
      goto fail;
  }
  modeFlags |= oflag & O_APPEND ? FILE_FLAG_APPEND : 0;
  if (name) {
    nameHash = exFatHashName(name, nameLength, 0);
    dir->rewind();
  }
  freeNeed = 2 + (nameLength + 14)/15;

  while (1) {
    n = dir->read(buf, 32);
    if (n == 0) {
      goto create;
    }
    if (n != 32) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!(buf[0] & 0x80)) {
      if (freeCount == 0) {
        freePos.position = dir->curPosition() - 32;
        freePos.cluster = dir->curCluster();
      }
      if (freeCount < freeNeed) {
        freeCount++;
      }
      if (!buf[0]) {
        goto create;
      }
    } else if (!inSet) {
      if (freeCount < freeNeed) {
        freeCount = 0;
      }
      if (buf[0] != EXFAT_TYPE_FILE) {
        continue;
      }
      inSet = true;
    }
    switch (buf[0]) {
      case EXFAT_TYPE_FILE:
        memset(this, 0, sizeof(ExFatFile));
        dirFile = reinterpret_cast<DirFile_t*>(buf);
        m_setCount = dirFile->setCount;
        m_attributes = getLe16(dirFile->attributes) & FILE_ATTR_COPY;
        if (!(m_attributes & EXFAT_ATTRIB_DIRECTORY)) {
          m_attributes |= FILE_ATTR_FILE;
        }
        m_vol = dir->volume();

        m_dirPos.cluster = dir->curCluster();
        m_dirPos.position = dir->curPosition() - 32;
        m_dirPos.isContiguous = dir->isContiguous();
        break;

      case EXFAT_TYPE_STREAM:
        dirStream = reinterpret_cast<DirStream_t*>(buf);
        m_flags = modeFlags;
        if (dirStream->flags & EXFAT_FLAG_CONTIGUOUS) {
          m_flags |= FILE_FLAG_CONTIGUOUS;
        }
        nameOffset = 0;
        m_validLength = getLe64(dirStream->validLength);
        m_firstCluster = getLe32(dirStream->firstCluster);
        m_dataLength = getLe64(dirStream->dataLength);
        if (!name) {
          goto found;
        }
        if (nameLength != dirStream->nameLength ||
            nameHash != getLe16(dirStream->nameHash)) {
          inSet = false;
          break;
        }
        break;

      case EXFAT_TYPE_NAME:
        dirName = reinterpret_cast<DirName_t*>(buf);
        nCmp = nameLength - nameOffset;
        if (nCmp > 15) {
          nCmp = 15;
        }
        if (!exFatCmpName(dirName, name, nameOffset, nCmp)) {
          inSet = false;
          break;
        }
        nameOffset += nCmp;

        if (nameOffset == nameLength) {
          goto found;
        }
        break;

      default:
        break;
    }
  }

 found:
  // Don't open if create only.
  if (oflag & O_EXCL) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Write, truncate, or at end is an error for a directory or read-only file.
  if ((oflag & (O_TRUNC | O_AT_END)) || (m_flags & FILE_FLAG_WRITE)) {
    if (isSubDir() || isReadOnly() || READ_ONLY) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }

#if !READ_ONLY
  if (oflag & O_TRUNC) {
    if (!(m_flags & FILE_FLAG_WRITE)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!truncate(0)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else if ((oflag & O_AT_END) && !seekSet(fileSize())) {
    DBG_FAIL_MACRO;
    goto fail;
  }
#endif  // READ_ONLY
  return true;

 create:
#if READ_ONLY
  DBG_FAIL_MACRO;
  goto fail;
#else  // READ_ONLY
  // don't create unless O_CREAT and write
  if (!(oflag & O_CREAT) || !(modeFlags & FILE_FLAG_WRITE) || !name) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  while (freeCount < freeNeed) {
    n = dir->read(buf, 32);
    if (n == 0) {
      curCluster = dir->m_curCluster;
      if (!dir->addDirCluster()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      dir->m_curCluster = curCluster;
      continue;
    }
    if (n != 32) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (freeCount == 0) {
      freePos.position = dir->curPosition() - 32;
      freePos.cluster = dir->curCluster();
    }
    freeCount++;
  }

  freePos.isContiguous = dir->isContiguous();
  memset(this, 0, sizeof(ExFatFile));
  m_vol = dir->volume();
  m_attributes = FILE_ATTR_FILE;
  m_dirPos = freePos;
  for (uint8_t i = 0; i < freeNeed; i++) {
    if (i) {
      if (1 != m_vol->dirSeek(&freePos, 32)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    cache = m_vol->dirCache(&freePos, FsCache::CACHE_FOR_WRITE);
    if (!cache || (cache[0] & 0x80)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    memset(cache, 0 , 32);
    if (i == 0) {
      dirFile = reinterpret_cast<DirFile_t*>(cache);
      dirFile->type = EXFAT_TYPE_FILE;
      m_setCount = freeNeed - 1;
      dirFile->setCount = m_setCount;

      if (FsDateTime::callback) {
        uint16_t date, time;
        uint8_t ms10;
        FsDateTime::callback(&date, &time, &ms10);
        setLe16(dirFile->createDate, date);
        setLe16(dirFile->createTime, time);
        dirFile->createTimeMs = ms10;
      } else {
        setLe16(dirFile->createDate, FS_DEFAULT_DATE);
        setLe16(dirFile->modifyDate, FS_DEFAULT_DATE);
        setLe16(dirFile->accessDate, FS_DEFAULT_DATE);
       if (FS_DEFAULT_TIME) {
         setLe16(dirFile->createTime, FS_DEFAULT_TIME);
         setLe16(dirFile->modifyTime, FS_DEFAULT_TIME);
         setLe16(dirFile->accessTime, FS_DEFAULT_TIME);
       }
      }
    } else if (i == 1) {
      dirStream = reinterpret_cast<DirStream_t*>(cache);
      dirStream->type = EXFAT_TYPE_STREAM;
      dirStream->flags = EXFAT_FLAG_ALWAYS1 | EXFAT_FLAG_CONTIGUOUS;
      m_flags = modeFlags | FILE_FLAG_CONTIGUOUS | FILE_FLAG_DIR_DIRTY;

      dirStream->nameLength = nameLength;
      setLe16(dirStream->nameHash, nameHash);
    } else {
      dirName = reinterpret_cast<DirName_t*>(cache);
      dirName->type = EXFAT_TYPE_NAME;
      nameOffset = 15*(i - 2);
      nCmp = nameLength - nameOffset;
      if (nCmp > 15) {
        nCmp = 15;
      }
      for (size_t k = 0; k < nCmp; k++) {
        setLe16(dirName->unicode + 2*k, name[k + nameOffset]);
      }
    }
  }
  return sync();
#endif  // READ_ONLY
 fail:

  // close file
  m_attributes = FILE_ATTR_CLOSED;
  m_flags = 0;
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::openRoot(ExFatVolume* vol) {
  if (isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  memset(this, 0, sizeof(ExFatFile));
  m_attributes = FILE_ATTR_ROOT;
  m_vol = vol;
  m_flags = FILE_FLAG_READ;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::parsePathName(const ExChar_t* path,
                            ExName_t* fname, const ExChar_t** ptr) {
  ExChar_t c;
  int end;
  int len = 0;

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
      return false;
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
  if (len > EXFAT_MAX_NAME_LENGTH) {
    return false;
  }
  fname->len = len;
  return true;
}
//------------------------------------------------------------------------------
int ExFatFile::peek() {
  uint64_t curPosition = m_curPosition;
  uint32_t curCluster = m_curCluster;
  int c = read();
  m_curPosition = curPosition;
  m_curCluster = curCluster;
  return c;
}
//------------------------------------------------------------------------------
int ExFatFile::read(void* buf, size_t count) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  int8_t fg;
  size_t toRead = count;
  size_t n;
  uint8_t* cache;
  uint16_t sectorOffset;
  uint32_t sector;
  uint32_t clusterOffset;

  if (!isReadable()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isContiguous() || isFile()) {
    if ((m_curPosition + count) > m_validLength) {
      count = toRead = m_validLength - m_curPosition;
    }
  }
  while (toRead) {
    clusterOffset = m_curPosition & m_vol->clusterMask();
    sectorOffset = clusterOffset & m_vol->sectorMask();
    if (clusterOffset == 0) {
      if (m_curPosition == 0) {
        m_curCluster = isRoot()
                       ? m_vol->rootDirectoryCluster() : m_firstCluster;
      } else if (isContiguous()) {
        m_curCluster++;
      } else {
        fg = m_vol->fatGet(m_curCluster, &m_curCluster);
        if (fg < 0) {
          DBG_FAIL_MACRO;
          goto fail;
        }
        if (fg == 0) {
          // EOF if directory.
          if (isDir()) {
            break;
          }
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
    }
    sector = m_vol->clusterStartSector(m_curCluster) +
             (clusterOffset >> m_vol->bytesPerSectorShift());
    if (sectorOffset != 0 || toRead < m_vol->bytesPerSector()
                          || sector == m_vol->dataCacheSector()) {
      n = m_vol->bytesPerSector() - sectorOffset;
      if (n > toRead) {
        n = toRead;
      }
      // read sector to cache and copy data to caller
      cache = m_vol->dataCacheGet(sector, FsCache::CACHE_FOR_READ);
      if (!cache) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      uint8_t* src = cache + sectorOffset;
      memcpy(dst, src, n);
#if USE_MULTI_SECTOR_IO
    } else if (toRead >= 2*m_vol->bytesPerSector()) {
      uint32_t ns = toRead >> m_vol->bytesPerSectorShift();
      // Limit reads to current cluster.
      uint32_t maxNs = m_vol->sectorsPerCluster()
                       - (clusterOffset >> m_vol->bytesPerSectorShift());
      if (ns > maxNs) {
        ns = maxNs;
      }
      n = ns << m_vol->bytesPerSectorShift();
     if (!m_vol->cacheSafeRead(sector, dst, ns)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // USE_MULTI_SECTOR_IO
    } else {
      // read single sector
      n = m_vol->bytesPerSector();
      if (!m_vol->cacheSafeRead(sector, dst)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    dst += n;
    m_curPosition += n;
    toRead -= n;
  }
  return count - toRead;

 fail:
  m_error |= READ_ERROR;
  return -1;
}
//------------------------------------------------------------------------------
bool ExFatFile::remove(const ExChar_t* path) {
  ExFatFile file;
  if (!file.open(this, path, O_WRONLY)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return file.remove();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool ExFatFile::seekSet(uint64_t pos) {
  uint32_t nCur;
  uint32_t nNew;
  uint32_t tmp = m_curCluster;
  // error if file not open
  if (!isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Optimize O_APPEND writes.
  if (pos == m_curPosition) {
    return true;
  }
  if (pos == 0) {
    // set position to start of file
    m_curCluster = 0;
    goto done;
  }
  if (isFile()) {
    if (pos > m_validLength) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // calculate cluster index for new position
  nNew = (pos - 1) >> m_vol->bytesPerClusterShift();
  if (isContiguous()) {
    m_curCluster = m_firstCluster + nNew;
    goto done;
  }
  // calculate cluster index for current position
  nCur = (m_curPosition - 1) >> m_vol->bytesPerClusterShift();
  if (nNew < nCur || m_curPosition == 0) {
    // must follow chain from first cluster
    m_curCluster = isRoot() ? m_vol->rootDirectoryCluster() : m_firstCluster;
  } else {
    // advance from curPosition
    nNew -= nCur;
  }
  while (nNew--) {
    if (m_vol->fatGet(m_curCluster, &m_curCluster) <= 0) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }

 done:
  m_curPosition = pos;
  return true;

 fail:
  m_curCluster = tmp;
  return false;
}
