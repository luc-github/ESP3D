/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

//Use ESP3DLib instead of Marlin file
#include "../esp3d_config.h"

//#include "../inc/MarlinConfig.h"
//stripped version of classreader

#if defined(SDSUPPORT) && defined (ESP3DLIB_ENV)

typedef struct {
    bool saving:1,
         logging:1,
         sdprinting:1,
         sdprintdone:1,
         mounted:1,
         filenameIsDir:1,
         workDirIsRoot:1,
         abort_sd_printing:1;
} card_flags_t;



class CardReader
{
public:
    static card_flags_t flag;                         // Flags (above)
    static void mount();
    static void release();
    static bool isMounted()
    {
        return flag.mounted;
    }

    static void abortFilePrintSoon()
    {
        flag.abort_sd_printing = isFileOpen();
    }
    static void pauseSDPrint()
    {
        flag.sdprinting = false;
    }
    static bool isPrinting()
    {
        return flag.sdprinting;
    }
    static bool isPaused()
    {
        return isFileOpen() && !isPrinting();
    }
    static bool isFileOpen()
    {
        return isMounted() && isPrinting();
    }
private:

};


#define IS_SD_PRINTING()  (card.flag.sdprinting && !card.flag.abort_sd_printing)
#define IS_SD_FETCHING()  (!card.flag.sdprintdone && IS_SD_PRINTING())
#define IS_SD_PAUSED()    card.isPaused()
#define IS_SD_FILE_OPEN() card.isFileOpen()

extern CardReader card;

#else // !SDSUPPORT

#define IS_SD_PRINTING()  false
#define IS_SD_FETCHING()  false
#define IS_SD_PAUSED()    false
#define IS_SD_FILE_OPEN() false

#endif // !SDSUPPORT
