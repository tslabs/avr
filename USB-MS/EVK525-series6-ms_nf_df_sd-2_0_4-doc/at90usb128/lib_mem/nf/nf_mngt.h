/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file is the header file for the high level management of the
//!
//!  nand flash memory devices.
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

#ifndef _NF_MNGT_H_
#define _NF_MNGT_H_

//_____ I N C L U D E S ____________________________________________________

#include "config.h"
#include "conf_nf.h"
#include "nf.h"
#include "modules/control_access/ctrl_status.h"
#include "nf_drv.h"
#include "nf_raw.h"

//_____ M A C R O S ________________________________________________________

//_____ D E C L A R A T I O N ______________________________________________

#define NF_BLK_RCV_NO_RECOVERY        0xA5
#define NF_BLK_RCV_PENDING_RECOVERY   0x5A

#define NF_LOW_N_FREE_THRESHOLD      5     // Min number of free blocks, otherwize the memory need to be reformatted

#if (NF_N_DEVICES==1)
#  define NF_SHIFT_N_DEVICES           0 // (1<<n) Number of devices

#elif (NF_N_DEVICES==2)
#  define NF_SHIFT_N_DEVICES           1 // (1<<n) Number of devices

#elif (NF_N_DEVICES==4)
#  define NF_SHIFT_N_DEVICES           2 // (1<<n) Number of devices
#endif

#if (NF_GENERIC_DRIVER==(TRUE))
#  define NF_PAGE_BUFFER_SIZE         2048
#else
//#  define NF_PAGE_BUFFER_SIZE         (1L<<NF_SHIFT_PAGE_BYTE)
#  define NF_PAGE_BUFFER_SIZE         2048
#endif
#define NF_FULL_PAGE_BUFFER_SIZE    ( (NF_PAGE_BUFFER_SIZE) + (NF_PAGE_BUFFER_SIZE)/32 )
#define NF_SHIFT_SUBLUT_PHYS      ( (G_SHIFT_PAGE_BYTE)-1 )        // (1<<n) size of sublut, unit in physical block
#define NF_SUBLUT_SIZE            ( 1L<<(NF_SHIFT_SUBLUT_PHYS) )   // size of sublut, unit in physical block


#if (NF_AUTO_DETECT_2KB==TRUE)
#  define NF_N_MAX_BLOCKS    (8*1024)  // Allow 1GByte for 2kB-NF
#  define N_SUBLUT    (NF_N_DEVICES*NF_N_MAX_BLOCKS/(512/2))

#elif (NF_AUTO_DETECT_512B==TRUE)
#  define NF_N_MAX_BLOCKS    (8*1024)  // Allow 128MB for 512B-NF
#  define N_SUBLUT    (NF_N_DEVICES*NF_N_MAX_BLOCKS/(512/2))

#else
#  if (NF_GENERIC_DRIVER==(TRUE))
#     define N_SUBLUT    (NF_N_DEVICES*NF_N_BLOCKS/(512/2))     To check...
#  else
#     define N_SUBLUT    (NF_N_DEVICES*NF_N_BLOCKS/(1L<<(NF_SHIFT_PAGE_BYTE-1)))
#  endif
#endif

#if (NF_GENERIC_DRIVER==TRUE)
#  define S_SHIFT_SECTOR_BYTE       s_shift_sector_byte
#  define S_SHIFT_LOG_PAGE_SECTOR   s_shift_log_page_sector
#  define S_SHIFT_LOG_BLOCK_SECTOR  s_shift_log_block_sector
#  error  To be tested
#else
#  define S_SHIFT_SECTOR_BYTE       (NF_SHIFT_SECTOR_BYTE)
#  define S_SHIFT_LOG_PAGE_SECTOR   ((G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE) + NF_SHIFT_N_DEVICES)
#  define S_SHIFT_LOG_BLOCK_SECTOR  ((S_SHIFT_LOG_PAGE_SECTOR) + (G_SHIFT_BLOCK_PAGE))
#endif


#define S_MNGT_DEV                ((NF_N_DEVICES)-1)

#if (_ASSERT_==ENABLE)
_MEM_TYPE_SLOW_ U16 _debug;
static void nf_check_fbb( Bool b_ecc_err );
static void nf_check_lut( void );
#  define Nf_check_fbb(x)  nf_check_fbb(x)
#  define Nf_check_lut()   nf_check_lut()
#else
#  define Nf_check_fbb(x)
#  define Nf_check_lut()
#endif

typedef struct
{
   struct
   {
      U8 valid:1 ;
      U8 dirty:1 ;
   } ctrl;
   U16  first ; // first logical number (included)
   U16  last  ; // last  logical number (included)
#define CACHE_LUT_SIZE   (NF_CACHE_LUT_LOG_SZ*NF_N_DEVICES) // Size in U16
   U16  mem[ CACHE_LUT_SIZE ];
} Cache_lut;

typedef struct
{
   struct
   {
      U8 valid:1 ;
      U8 dirty:1 ;
   } ctrl;
   U8   p     ; // Current logical number position
   U8   max   ; // number of logical entry
#define CACHE_FBB_SIZE   (NF_CACHE_FBB_LOG_SZ*NF_N_DEVICES) // Size in U16
   U16  mem[ CACHE_FBB_SIZE ];
} Cache_fbb;

typedef enum
{
   NF_READ=0
,  NF_WRITE
} Nf_sense;

//_____ F U N C T I O N S __________________________________________________

Ctrl_status    nf_test_unit_ready         ( void ) ;
Ctrl_status    nf_read_capacity           ( U32* ) ;
Bool           nf_wr_protect              ( void ) ;
Bool           nf_removal                 ( void ) ;
Ctrl_status    nf_10                      ( U32 addr , U16 nb_sector, Nf_sense );

Ctrl_status    nf_dfc_read_resume         ( void );   // function that resume the dfc interface of the memory
Ctrl_status    nf_dfc_write_resume        ( void );   // function that resume the dfc interface of the memory
Ctrl_status    nf_dfc_read_stop           ( U16 remaining_sectors );   // function that stops the dfc interface of the memory
Ctrl_status    nf_dfc_write_stop          ( U16 remaining_sectors );   // function that stops the dfc interface of the memory

void           nf_dfc_read_standby        ( U16 );
void           nf_dfc_read_restart        ( void );

U32            nf_get_sectors_number      ( void ) ;
U8*            nf_get_buffer_addr         ( void ) ;
void           nf_init                    ( void ) ;
Status_bool    nf_verify                  ( void ) ;
Status_bool    nf_verify_resume           ( void ) ;
void           nf_cleanup_memory          ( void ) ;
void           nf_upload                  ( U8 _MEM_TYPE_SLOW_*, U8 );
void           nf_download                ( U8 _MEM_TYPE_SLOW_*, U8 );
U32            nf_block_2_page            ( U16 block_addr );
void           nf_ecc_mngt                ( void );
void           nf_sync                    ( void );
void           nf_copy                    ( U32 copy_dst );
Status_bool    nf_write_lut               ( U8 pos, U8 i_sub_lut, U16 sub_lut_log_sz );
Status_bool    nf_write_fbb               ( void );
void           nf_cache_fbb_refill        ( void );
void           nf_swap                    (U8 dev_id, U8 u8_ofst_lut, U8 u8_ofst_fbb);
void           nf_cache_fbb_flush         ( Bool );
void           nf_recovery(            U16 block_1, U16 block_2);
void           nf_copy_tail               ( void );

// New functions, to be tested
Ctrl_status    nf_read_10( U32 log_sector , U16 n_sectors);
Ctrl_status    nf_write_10( U32 log_sector , U16 n_sectors);
Ctrl_status    nf_ram_2_nf                (U32 addr, U8 *ram);
Ctrl_status    nf_nf_2_ram                (U32 addr, U8 *ram);

#endif  // _NF_MNGT_H_

