/*
  lv_flash_drv.cpp - ESP3D flash (spiffs/Fat/etc) driver for lvgl

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
#include "../../include/esp3d_config.h"
#if defined (DISPLAY_DEVICE) && (DISPLAY_UI_TYPE == UI_TYPE_ADVANCED)
#include "lv_flash_drv.h"
#ifndef FILESYSTEM_FEATURE
#error No FileSystem defined
#endif
#if FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM
#include "SPIFFS.h"
#else
#if FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM
#include "FFat.h"
#else
#error Unknow FileSystem defined
#endif
#endif

//Driver wrote based on lvg 6.0.3 documentation

/**
 * Open a file from the PC
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable
 * @param fn name of the file.
 * @param mode element of 'fs_mode_t' enum or its 'OR' connection (e.g. FS_MODE_WR | FS_MODE_RD)
 * @return LV_FS_RES_OK: no error, the file is opened
 *         any error from lv_fs_res_t enum
 */
lv_fs_res_t esp_flash_open(lv_fs_drv_t * drv, void * file_p, const char * fn, lv_fs_mode_t mode)
{
    (void) drv; /*Unused*/
    const char * flags = "";
    if(mode == LV_FS_MODE_WR) {
        flags = "w";
    } else if(mode == LV_FS_MODE_RD) {
        flags = "r";
    } else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) {
        flags = "a";
    }
    char buf[256];
    sprintf(buf, "/%s", fn);
    File * f= new File();
    if (f == nullptr) {
        return LV_FS_RES_OUT_OF_MEM;
    }
#if FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM
    *f = SPIFFS.open(buf, flags);
#else
    *f = FFat.open(buf, flags);
#endif
    if(!(*f)) {
        return LV_FS_RES_UNKNOWN;
    } else {
        f->seek(0);
        file_t * ft = (file_t *)file_p;
        ft->fileobjecthandle = f;
    }
    return LV_FS_RES_OK;
}


/**
 * Close an opened file
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
lv_fs_res_t esp_flash_close(lv_fs_drv_t * drv, void * file_p)
{
    (void) drv; /*Unused*/
    file_t * ft = (file_t *)file_p;
    if(ft->fileobjecthandle) {
        (ft->fileobjecthandle)->close();
        delete(ft->fileobjecthandle);
        ft->fileobjecthandle = nullptr;
    }
    return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
lv_fs_res_t esp_flash_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    (void) drv; /*Unused*/
    file_t * ft = (file_t *)file_p;
    if(ft->fileobjecthandle) {
        *br =  (ft->fileobjecthandle)->read((uint8_t*)buf,btr);
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
lv_fs_res_t esp_flash_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos)
{
    (void) drv; /*Unused*/
    file_t * ft = (file_t *)file_p;
    if(ft->fileobjecthandle) {
        (ft->fileobjecthandle)->seek(pos);
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to the current driver
 * @param file_p pointer to a FILE* variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv__fs_res_t enum
 */
lv_fs_res_t esp_flash_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    (void) drv; /*Unused*/
    file_t * ft = (file_t *)file_p;
    if(ft->fileobjecthandle) {
        *pos_p = (ft->fileobjecthandle)->position();
        return LV_FS_RES_OK;
    } else {
        return LV_FS_RES_UNKNOWN;
    }
}

#endif //(DISPLAY_DEVICE) && (DISPLAY_UI_TYPE == UI_TYPE_ADVANCED)
