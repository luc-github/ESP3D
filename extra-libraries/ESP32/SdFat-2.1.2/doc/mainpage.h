/**
 * Copyright (c) 2011-2021 Bill Greiman
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
/**
\mainpage Arduino %SdFat Library

\section Warn Warnings for SdFat V2

This is a major new version of SdFat. It is mostly
backward compatible with SdFat Version 1 for FAT16/FAT32 cards.

You should edit SdFatConfig.h to select features. The default version of
SdFatConfig.h is suitable for UNO and other small AVR boards. 

\section Intro Introduction
 
The Arduino %SdFat library supports FAT16, FAT32, and exFAT file systems 
on Standard SD, SDHC, and SDXC cards.
 
In %SdFat version 1, SdFat and File are the main classes.

In %SdFat version 2, SdFat and File are defined by typedefs in terms of the
following classes. 

The file system classes in the %SdFat library are SdFat32, SdExFat, and SdFs. 
SdFat32 supports FAT16 and FAT32. SdExFat supports exFAT, SdFs supports
FAT16, FAT32, and exFAT.

The corresponding file classes are File32, ExFile, and FsFile.

The types for SdFat and File are defined in SdFatConfig.h. This version
uses FAT16/FAT32 for small AVR boards and FAT16/FAT32/exFAT for all other
boards.

\code{.cpp}
// File types for SdFat, File, SdFile, SdBaseFile, fstream,
// ifstream, and ofstream.
//
// Set SDFAT_FILE_TYPE to:
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
//
#if defined(__AVR__) && FLASHEND < 0X8000
// FAT16/FAT32 for 32K AVR boards.
#define SDFAT_FILE_TYPE 1
#else  // defined(__AVR__) && FLASHEND < 0X8000
// FAT16/FAT32 and exFAT for all other boards.
#define SDFAT_FILE_TYPE 3
#endif  // defined(__AVR__) && FLASHEND < 0X8000
\endcode

It is possible to use option three, support or FAT16/FAT32 and exFat 
on an Uno or other AVR board with 32KB flash and 2KB SRAM but memory
will be very limited.

Uno memory use for a simple data logger is:

> option 1, FAT16/FAT32, 11902 bytes of flash and 855 bytes of SRAM.
>
> option 2, exFAT, 14942 bytes of flash and 895 bytes of SRAM.
>
> option 3, FAT16/FAT32 and exFAT, 21834 bytes of flash and 908 bytes of SRAM. 

Please read documentation under the above classes tab for more information.

A number of example are provided in the %SdFat/examples folder.  These were
developed to test %SdFat and illustrate its use.

\section exFAT exFAT Features

exFAT has many features not available in FAT16/FAT32.

Files larger than 4GiB, 64-bit file size and file position.

Free space allocation performance improved by using a free space bitmap.

Removal of the physical "." and ".." directory entries that appear in
FAT16/FAT32 subdirectories.

Better support for large flash pages with boundary alignment offsets
for the FAT table and data region.

exFAT files have two separate 64-bit length fields.  The DataLength
field indicate how much space is allocate to the file. The ValidDataLength
field indicates how much actual data has been written to the file.

An exFAT file can be contiguous with pre-allocate clusters and bypass the
use of the FAT table.  In this case the contiguous flag is set in the
directory entry.  This allows an entire file to be written as one large
multi-block write.

\section SDPath Paths and Working Directories

Relative paths in %SdFat are resolved in a manner similar to Windows.

Each instance of SdFat32, SdExFat, and SdFs has a current directory.  
This directory is called the volume working directory, vwd.  
Initially this directory is the root directory for the volume.

The volume working directory is changed by calling the chdir(path).

The call sd.chdir("/2014") will change the volume working directory
for sd to "/2014", assuming "/2014" exists.

Relative paths for member functions are resolved by starting at
the volume working directory.

For example, the call sd.mkdir("April") will create the directory
"/2014/April" assuming the volume working directory is "/2014".

There is current working directory, cwd, that is used to resolve paths
for file.open() calls.

For a single SD card, the current working directory is always the volume
working directory for that card.

For multiple SD cards the current working directory is set to the volume
working directory of a card by calling the chvol() member function.
The chvol() call is like the Windows \<drive letter>: command.

The call sd2.chvol() will set the current working directory to the volume
working directory for sd2.

If the volume working directory for sd2 is "/music" the call

file.open("BigBand.wav", O_READ);

will open "/music/BigBand.wav" on sd2.

\section Install Installation

You must manually install %SdFat by renaming the download folder %SdFat 
and copy the %SdFat folder to the Arduino libraries folder in your
sketchbook folder. 

It will be necessary to unzip and rename the folder if you download a zip
file from GitHub.

See the Manual installation section of this guide.

http://arduino.cc/en/Guide/Libraries

\section SDconfig SdFat Configuration

Several configuration options may be changed by editing the SdFatConfig.h
file in the %SdFat/src folder.

Here are a few of the key options.

If the symbol ENABLE_DEDICATED_SPI is nonzero, multi-block SD I/O may
be used for better performance.  The SPI bus may not be shared with
other devices in this mode.

The symbol SPI_DRIVER_SELECT is used to select the SPI driver.

> If the symbol SPI_DRIVER_SELECT is:
>
> 0 - An optimized custom SPI driver is used if it exists
>     else the standard library driver is used.
>
> 1 - The standard library driver is always used.
>
> 2 - The software SPI driver is always used.
>
> 3 - An external SPI driver derived from SdSpiBaseClass is always used.

To enable SD card CRC checking in SPI mode set USE_SD_CRC nonzero.

See SdFatConfig.h for other options.

\section Hardware Hardware Configuration

The hardware interface to the SD card should not use a resistor based level
shifter. Resistor based level shifters results in signal rise times that are
 too slow for many newer SD cards.


\section HowTo How to format SD Cards

The best way to restore an SD card's format on a PC or Mac is to use
SDFormatter which can be downloaded from:

http://www.sdcard.org/downloads

A formatter program, SdFormatter.ino, is included in the
%SdFat/examples/SdFormatter directory.  This program attempts to
emulate SD Association's SDFormatter.

SDFormatter aligns flash erase boundaries with file
system structures which reduces write latency and file system overhead.

The PC/Mac SDFormatter does not have an option for FAT type so it may format
very small cards as FAT12.  Use the %SdFormatter example to force FAT16
formatting of small cards.

Do not format the SD card with an OS utility, OS utilities do not format SD
cards in conformance with the SD standard.

You should use a freshly formatted SD card for best performance.  FAT
file systems become slower if many files have been created and deleted.
This is because the directory entry for a deleted file is marked as deleted,
but is not deleted.  When a new file is created, these entries must be scanned
before creating the file.  Also files can become
fragmented which causes reads and writes to be slower.

\section ExampleFiles Examples

A number of examples are provided in the SdFat/examples folder.

To access these examples from the Arduino development environment
go to:  %File -> Examples -> %SdFat -> \<program Name\>

Compile, upload to your Arduino and click on Serial Monitor to run
the example.

Here is a list:

AvrAdcLogger - Fast AVR ADC logger using Timer/ADC interrupts.

BackwardCompatibility - Demonstrate SD.h compatibility with %SdFat.h.

bench - A read/write benchmark.

%BufferedPrint - Demo a buffered print class for AVR loggers.

debug folder - Some of my debug programs - will be remove in the future.

DirectoryFunctions - Use of chdir(), ls(), mkdir(), and rmdir().

examplesV1 folder - Examples from SdFat V1 for compatibility tests.

ExFatLogger - A data-logger optimized for exFAT features.

MinimumSizeSdReader - Example of small file reader for FAT16/FAT32.

OpenNext - Open all files in the root dir and print their filename.

QuickStart - Quick hardware test for SPI card access.

ReadCsvFile - Function to read a CSV text file one field at a time.

rename - demonstrates use of rename().

RtcTimestampTest - Demonstration of timestamps with RTClib.

SdErrorCodes - Produce a list of error codes.

SdFormatter - This program will format an SD, SDHC, or SDXC card.

SdInfo - Initialize an SD card and analyze its structure for trouble shooting.

SoftwareSpi - Demo of limited Software SPI support in SdFat V2.

STM32Test - Example use of two SPI ports on an STM32 board.

TeensyDmaAdcLogger - Fast logger using DMA ADC.

TeensyRtcTimestamp - %File timestamps for Teensy3.

TeensySdioDemo - Demo of SDIO and SPI modes for the Teensy 3.5/3.6 built-in SD.

TeensySdioLogger -  Fast logger using a ring buffer.

UnicodeFilenames - Test program for Unicode file names.

UserChipSelectFunction - Useful for port expanders or replacement of the standard GPIO functions.

UserSPIDriver - An example of an external SPI driver.
 */
