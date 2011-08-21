/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the possible external configuration of the explorer
//!
//!  This file will be given to any external customer
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CONF_EXPLORER_H_
#define _CONF_EXPLORER_H_

//! FAT supported (ENABLED or DISABLE)
#define  FS_FAT_12   ENABLED
#define  FS_FAT_16   ENABLED
#define  FS_FAT_32   ENABLED

//! The navigator may be support only the first partition (DISABLED), or the multiple partion (ENABLED)
#define  FS_MULTI_PARTITION    DISABLED

//! Level of feature
//! Choose between:
//! FSFEATURE_READ                  All read fonctions
//! FSFEATURE_WRITE                 nav_file_copy(), nav_file_paste(), nav_file_del(), file_create(), file_open(MODE_WRITE), file_write(), file_putc()
//! FSFEATURE_WRITE_COMPLET         FSFEATURE_WRITE + nav_drive_format(), nav_dir_make(), nav_file_rename(), nav_file_dateset(), nav_file_attributset()
//! FSFEATURE_ALL
// RLE #define  FS_LEVEL_FEATURES             (FSFEATURE_READ | FSFEATURE_WRITE_COMPLET )
#define  FS_LEVEL_FEATURES             (FSFEATURE_READ | FSFEATURE_WRITE_COMPLET)

//! Number of cache used to store a cluster list of files (interresting in case of many "open file")
#define  FS_NB_CACHE_CLUSLIST 1  // In usual mode, 1 is OK (shall be > 0)

//! Maximum of navigator
#define  FS_NB_NAVIGATOR      3

//! *** Define the affiliation of navigator (Rq: the explorer has always the navigator ID 0)
// The play list use the navigator ID 1
#define  FS_NAV_ID_PLAYLIST   1
// The explorer use the navigator ID 2 to open the "copy file" and ID 0 to open the "paste file"
#define  FS_NAV_ID_COPYFILE   2


//!**** INOFRMATION ABOUT LIMITATION OF FILE SYSTEM ****
//! Maximum file supported in a directory is 65534
//! This navigator support only the file system with 2 FAT
//! This navigator support only the drives who are a sector egal at 512 bytes
//!*****************************************************

#define UNICODENAME_FILE_SETTING     {'c','o','n','f','i','g','.','s','y','s','\0'}
#endif  //! _CONF_EXPLORER_H_

