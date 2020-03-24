/*
  lv_flash_drv.h - ESP3D flash (spiffs/Fat/etc) driver for lvgl

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
#ifndef _LV_FLASH_DRV_H
#define _LV_FLASH_DRV_H
#include "FS.h"
#include <lvgl.h>

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    File * fileobjecthandle;
} file_t;

/**********************
 *  PROTOTYPES
 **********************/
/*Interface functions to file functions (only the required ones to image handling)*/
extern lv_fs_res_t esp_flash_open(lv_fs_drv_t * drv, void * file_p, const char * fn, lv_fs_mode_t mode);
extern lv_fs_res_t esp_flash_close(lv_fs_drv_t * drv, void * file_p);
extern lv_fs_res_t esp_flash_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
extern lv_fs_res_t esp_flash_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos);
extern lv_fs_res_t esp_flash_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);


#endif //_LV_FLASH_DRV_H
