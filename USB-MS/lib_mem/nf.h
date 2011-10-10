/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief NAND FLASH types and information in regards of manufacturer Ids
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

#ifndef _NF_H_
#define _NF_H_

// ooo ooo                            o
//  o   o
//  o   o
//  o   o   oooo   ooo oo  oo oo    ooo    oo oo    oooooo
//  o o o       o    oo  o  oo  o     o     oo  o  o    o
//  o o o   ooooo    o      o   o     o     o   o  o    o
//  o o o  o    o    o      o   o     o     o   o  o    o
//   o o   o    o    o      o   o     o     o   o   ooooo
//   o o    oooo o ooooo   ooo ooo  ooooo  ooo ooo      o
//                                                      o
//                                                  oooo
//
// Please note that auto-detect mode (NF_AUTO_DETECT_2KB=TRUE or
// NF_AUTO_DETECT_512B=TRUE) may give different performances
// than static compilation (#define NF_TYPE_...)
//

//! Nand Flash Maker definition
#define  M_ID_MICRON       0x2C
#define  M_ID_HYNIX        0xAD
#define  M_ID_SANDISK      0x98
#define  M_ID_ST           0x20
#define  M_ID_TOSHIBA      0x98
#define  M_ID_TOSHIBA96    0x96
#define  M_ID_SAMSUNG      0xEC
#define  M_ID_INFINEON     0xC1

// Structs used in generic mode

// Device Indentifiant
typedef struct {
   U8  manuf;
   U8  dev;
   U8  conf;
} St_nf_id;

// Table to link Device Indentifiant and Number of block
typedef struct {
   U8  dev_id;
   U8  nb_zones;     // zone = 1024 blocks
}St_nf_link_id_block;

// Device Configuration
typedef struct {
   U8  copy_back_cont   :4;
   U8  copy_back_discont:4;
   U8  cache_program    :1;
   U8  ce_toggle        :1;
} St_nf_conf;


//-- Pre Check configuration
//
#define NF_BAD_CONFIG            FALSE
#define NF_GENERIC_DRIVER        FALSE // the configuration TRUE, it is not available
#define NF_SHIFT_SECTOR_BYTE     9     // (1<<n) size of sector, unit in bytes (other value not supported)
#if (!defined NF_AUTO_DETECT_2KB)
#  define   NF_AUTO_DETECT_2KB   FALSE
#endif
#if (!defined NF_AUTO_DETECT_512B)
#  define   NF_AUTO_DETECT_512B  FALSE
#endif
#if (NF_AUTO_DETECT_2KB==TRUE) && (NF_AUTO_DETECT_512B==TRUE)
#  undef  NF_BAD_CONFIG
#  define NF_BAD_CONFIG (TRUE)
#  error NF_AUTO_DETECT_2KB and NF_AUTO_DETECT_512B can not be activated at the same time.
#endif

#include "nf_512B.h"
#include "nf_2KB.h"

//-- Final Check configuration
//
#if (NF_AUTO_DETECT_2KB==TRUE) && (NF_TYPE_512B_KNOWN==TRUE)
#  undef  NF_BAD_CONFIG
#  define NF_BAD_CONFIG (TRUE)
#  error NF_AUTO_DETECT_2KB and NF_TYPE_x 512B can not be activated at the same time.
#endif
#if (NF_AUTO_DETECT_512B==TRUE) && (NF_TYPE_2KB_KNOWN==TRUE)
#  undef  NF_BAD_CONFIG
#  define NF_BAD_CONFIG (TRUE)
#  error NF_AUTO_DETECT_512B and NF_TYPE_x 2KB can not be activated at the same time.
#endif
#if (NF_AUTO_DETECT_2KB==FALSE) && (NF_AUTO_DETECT_512B==FALSE)
#  define NF_DETECTION_ID  (DISABLE)
#  if (NF_TYPE_2KB_KNOWN ==FALSE) && (NF_TYPE_512B_KNOWN ==FALSE)
#     undef  NF_BAD_CONFIG
#     define NF_BAD_CONFIG (TRUE)
#     error No NandFlash configuration found (NF_AUTO_DETECT_2KB, NF_AUTO_DETECT_512B, NF_TYPE_x)
#  endif
#else
#  define NF_DETECTION_ID  (ENABLE)
#endif


#endif   // _NF_H_
