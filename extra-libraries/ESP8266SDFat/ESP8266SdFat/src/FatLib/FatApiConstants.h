/**
 * Copyright (c) 2011-2018 Bill Greiman
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
#ifndef FatApiConstants_h
#define FatApiConstants_h

#include "SdFatConfig.h"
#if USE_FCNTL_H
#include <fcntl.h>
#endif

namespace sdfat {

#if USE_FCNTL_H
/* values for GNU Arm Embedded Toolchain.
 * O_RDONLY:   0x0
 * O_WRONLY:   0x1
 * O_RDWR:     0x2
 * O_ACCMODE:  0x3
 * O_APPEND:   0x8
 * O_CREAT:    0x200
 * O_TRUNC:    0x400
 * O_EXCL:     0x800
 * O_SYNC:     0x2000
 * O_NONBLOCK: 0x4000
 */
/** Use O_NONBLOCK for open at EOF */
#define O_AT_END O_NONBLOCK  ///< Open at EOF.
typedef int oflag_t;
#else  // USE_FCNTL_H
// Temp fix for particle mesh.
#ifdef O_RDONLY
#undef O_RDONLY
#endif  // O_RDONLY
#ifdef O_RDWR
#undef O_RDWR
#endif  // O_RDWR
#ifdef O_WRONLY
#undef O_WRONLY
#endif  // O_WRONLY
//------------------------------------------------------------------------------
// use the gnu style oflag in open()
/** open() oflag for reading */
const uint8_t O_READ = 0X01;
/** open() oflag - same as O_IN */
const uint8_t O_RDONLY = O_READ;
/** open() oflag for write */
const uint8_t O_WRITE = 0X02;
/** open() oflag - same as O_WRITE */
const uint8_t O_WRONLY = O_WRITE;
/** open() oflag for reading and writing */
const uint8_t O_RDWR = (O_READ | O_WRITE);
/** open() oflag mask for access modes */
const uint8_t O_ACCMODE = (O_READ | O_WRITE);
/** The file offset shall be set to the end of the file prior to each write. */
const uint8_t O_APPEND = 0X04;
/** synchronous writes - call sync() after each write */
const uint8_t O_SYNC = 0X08;
/** truncate the file to zero length */
const uint8_t O_TRUNC = 0X10;
/** set the initial position at the end of the file */
const uint8_t O_AT_END = 0X20;
/** create the file if nonexistent */
const uint8_t O_CREAT = 0X40;
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
const uint8_t O_EXCL = 0X80;
#if 0
#define O_RDONLY  0X00  ///< Open for reading only.
#define O_WRONLY  0X01  ///< Open for writing only.
#define O_RDWR    0X02  ///< Open for reading and writing.
#define O_AT_END  0X04  ///< Open at EOF.
#define O_APPEND  0X08  ///< Set append mode.
#define O_CREAT   0x10  ///< Create file if it does not exist.
#define O_TRUNC   0x20  ///< Truncate file to zero length.
#define O_EXCL    0x40  ///< Fail if the file exists.
#define O_SYNC    0x80  ///< Synchronized write I/O operations.

#define O_ACCMODE (O_RDONLY|O_WRONLY|O_RDWR)  ///< Mask for access mode.
#endif
typedef uint8_t oflag_t;
#endif  // USE_FCNTL_H

#define O_READ    O_RDONLY
#define O_WRITE   O_WRONLY

inline bool isWriteMode(oflag_t oflag) {
  oflag &= O_ACCMODE;
  return oflag == O_WRONLY || oflag == O_RDWR;
}

// FatFile class static and const definitions
// flags for ls()
/** ls() flag for list all files including hidden. */
#define LS_A 1
/** ls() flag to print modify. date */
#define LS_DATE 2
/** ls() flag to print file size. */
#define LS_SIZE 4
/** ls() flag for recursive list of subdirectories */
#define LS_R 8

// flags for timestamp
/** set the file's last access date */
#define T_ACCESS 1
/** set the file's creation date and time */
#define T_CREATE 2
/** Set the file's write date and time */
#define T_WRITE 4

}; // namespace sdfat

#endif  // FatApiConstants_h
