//! @file $RCSfile: nf.h,v $
//!
//! Copyright (c) 2004 Atmel.
//!
//! Use of this program is subject to Atmel's End User License Agreement.
//! Please read file license.txt for copyright notice.
//!
//! @brief NAND FLASH types and information in regards of manufacturer Ids
//!
//! @version $Revision: 2114 $ $Name:  $ $Id: nf.h 2114 2008-09-29 12:50:04Z sguyon $
//!
//! @todo
//! @bug

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
   U8  timing_read      :1;
   U8  dfc_nfc_clock    :5;
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
