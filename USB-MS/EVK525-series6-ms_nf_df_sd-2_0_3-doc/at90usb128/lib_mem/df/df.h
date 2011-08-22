/*This file has been prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the low-level dataflash routines
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************

/* Copyright (c) 2007, Atmel Corporation All rights reserved.
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
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DF_H_
#define _DF_H_

//_____ I N C L U D E S ____________________________________________________

#include "config.h"


//_____ M A C R O S ________________________________________________________

#define  MEM_BSY           0
#define  MEM_OK            1
#define  MEM_KO            2

//----- DataFlash definition -----
#define  DF_MSK_DENSITY    ((Byte)0x3C)      // status density mask 
#define  DF_MSK_BIT_BUSY   ((Byte)0x80)
#define  DF_MEM_BUSY       ((Byte)0x00)
        
#define  DF_RD_STATUS      ((Byte)0xD7)      // read status cmd 
#define  DF_PG_ERASE       ((Byte)0x81)      // page erase cmd  
#define  DF_BK_ERASE       ((Byte)0x50)      // block erase cmd 
        
#define  DF_WR_BUF_1       ((Byte)0x84)      // write buffer 1 cmd 
#define  DF_WR_BUF_2       ((Byte)0x87)      // write buffer 2 cmd 
#define  DF_B1_MAIN        ((Byte)0x83)      // buffer 1 to main program with erase cmd 
#define  DF_B2_MAIN        ((Byte)0x86)      // buffer 2 to main program with erase cmd 
        
#define  DF_RD_MAIN        ((Byte)0xD2)      // main memory page read cmd 
#define  DF_TF_BUF_1       ((Byte)0x53)      // main memory page to buffer 1 transfer cmd 
#define  DF_TF_BUF_2       ((Byte)0x55)      // main memory page to buffer 2 transfer cmd 
#define  DF_RD_BUF_1       ((Byte)0xD4)      // buffer 1 read cmd 
#define  DF_RD_BUF_2       ((Byte)0xD6)      // buffer 2 read cmd 
        
        
#define  DF_4MB            ((Byte)0)
#define  DF_8MB            ((Byte)1)
#define  DF_16MB           ((Byte)2)
#define  DF_32MB           ((Byte)3)
#define  DF_64MB           ((Byte)4)


// BUSY Management and Memory Selection 

// The variable "df_mem_busy" can contain the state of 4 dataflash 
#define  df_set_busy(i)       (df_mem_busy |= (1<<i))
#define  df_release_busy(i)   (df_mem_busy &= ~(1<<i))
#define  is_df_busy(i)        (((df_mem_busy&(1<<i)) != 0) ? TRUE : FALSE)


//_____ D E F I N I T I O N ________________________________________________

#ifdef DF_4_MB             // AT45DB321 memories 
  #define DF_SHFT_DFIDX    (22)              // RShift to apply to an absolute
                                             // * Byte address to get the DF idx 
  #define DF_DENSITY       ((Byte)0x34)
  #define DF_PG_BUF_1      ((Byte)0x82)      // main memory program through buf1 
  #define DF_PG_BUF_2      ((Byte)0x85)      // main memory program through buf2 
  #define DF_PAGE_SIZE     (512)             // page length 
  #define DF_PAGE_MASK     ((Byte)0x01)      // mask MSB page bits 
  #define DF_SHFT_B1       (1)
  #define DF_SHFT_B2       (7)
#endif                    
                          
#ifdef DF_8_MB             // AT45DB642 memories 
  #define DF_SHFT_DFIDX    (23)              // RShift to apply to an absolute
                                             // * Byte address to get the DF idx 
  #define DF_DENSITY       ((Byte)0x3C)
  #define DF_PG_BUF_1      ((Byte)0x82)      // fast main memory program through buf1 
  #define DF_PG_BUF_2      ((Byte)0x85)      // fast main memory program through buf2 
  #define DF_PAGE_SIZE     (1024)            // page length 
  #define DF_PAGE_MASK     ((Byte)0x03)      // mask MSB page bits 
  #define DF_SHFT_B1       (1)
  #define DF_SHFT_B2       (7)
#endif


//_____ D E C L A R A T I O N ______________________________________________

void    df_init                  (void);
Bool    df_mem_check             (void);
Bool    df_read_open             (Uint32);
void    df_read_close            (void);
Bool    df_write_open            (Uint32);
void    df_write_close           (void);

//! Funtions to link USB DEVICE flow with data flash
Bool    df_write_sector          (Uint16);
Bool    df_read_sector           (Uint16);
//! Funtions to link USB HOST flow with data flash
bit     df_host_write_sector (Uint16);
bit     df_host_read_sector (Uint16);

//! Functions to read/write one sector (512btes) with ram buffer pointer
Bool    df_read_sector_2_ram     (U8 *ram);
Bool    df_write_sector_from_ram (U8 *ram);

#endif  // _DF_H_ 


