### Warning: This is SdFat Version 2.

Earlier releases of Version 1 are here:

https://github.com/greiman/SdFat/releases

UTF-8 encoded filenames are supported in v2.1.0 or later.

Try the UnicodeFilenames example.  Here is output from ls:
<pre>
Type any character to begin
ls:
         0 😀/
          20 россиянин
          17 très élégant
           9 狗.txt
</pre>

SdFat Version 2 supports FAT16/FAT32 and exFAT SD cards. It is mostly
backward compatible with SdFat Version 1 for FAT16/FAT32 cards.

exFAT supports files larger than 4GB so files sizes and positions are
type uint64_t for classes that support exFAT.

exFAT has many features not available in FAT16/FAT32.  exFAT has excellent
support for contiguous files on flash devices and supports preallocation.

If the SD card is the only SPI device, use dedicated SPI mode. This can
greatly improve performance. See the bench example.

Here is write performance for an old, 2011, card on a Due board.
```
Shared SPI:
write speed and latency
speed,max,min,avg
KB/Sec,usec,usec,usec
294.45,24944,1398,1737

Dedicated SPI:
write speed and latency
speed,max,min,avg
KB/Sec,usec,usec,usec
3965.11,16733,110,127
```
The default version of SdFatConfig.h enables support for dedicated SPI and
optimized access to contiguous files.  This makes SdFat Version 2 slightly
larger than Version 1.  If these features are disabled, Version 2 is smaller
than Version 1.

The types for the classes SdFat and File are defined in SdFatConfig.h.
The default version of SdFatConfig.h defines SdFat to only support FAT16/FAT32.
SdFat and File are defined in terms of more basic classes by typedefs.  You
can use these basic classes in applications.

Support for exFAT requires a substantial amount of flash.  Here are sizes on
an UNO for a simple program that opens a file, prints one line, and closes
the file.
```
FAT16/FAT32 only: 9780 bytes flash, 875 bytes SRAM.

exFAT only: 13830 bytes flash, 938 bytes SRAM.

FAT16/FAT32/exFAT: 19326 bytes flash, 928 bytes SRAM.
```
The section below of SdFatConfig.h has been edited to uses FAT16/FAT32 for
small AVR boards and FAT16/FAT32/exFAT for all other boards.
```
/**
 * File types for SdFat, File, SdFile, SdBaseFile, fstream,
 * ifstream, and ofstream.
 *
 * Set SDFAT_FILE_TYPE to:
 *
 * 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
 */
#if defined(__AVR__) && FLASHEND < 0X8000
// FAT16/FAT32 for 32K AVR boards.
#define SDFAT_FILE_TYPE 1
#else  // defined(__AVR__) && FLASHEND < 0X8000
// FAT16/FAT32 and exFAT for all other boards.
#define SDFAT_FILE_TYPE 3
#endif  // defined(__AVR__) && FLASHEND < 0X8000
```
The SdBaseFile class has no Arduino Stream or Print support.

The File class is derived from Stream and SdBaseFile.

The SdFile class is derived from SdBaseFile and Print.

Please try the examples.  Start with SdInfo, bench, and ExFatLogger.

To use SdFat Version 2, unzip the download file, rename the library folder
SdFat and place the SdFat folder into the libraries sub-folder in your main
sketch folder.

For more information see the Manual installation section of this guide:

http://arduino.cc/en/Guide/Libraries

A number of configuration options can be set by editing SdFatConfig.h
define macros.  See the html documentation File tab for details.

Please read the html documentation for this library in SdFat/doc/SdFat.html.
Start with the  Main Page.  Next go to the Classes tab and read the
documentation for the classes SdFat32, SdExFat, SdFs, File32, ExFile, FsFile.

The SdFat and File classes are defined in terms of the above classes by
typedefs. Edit SdFatConfig.h to select class options.

Please continue by reading the html documentation in the SdFat/doc folder.
