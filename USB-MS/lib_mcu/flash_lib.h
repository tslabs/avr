/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains a set of routines to perform flash access.
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

#ifndef FLASH_LIB_H
#define FLASH_LIB_H


//_____ I N C L U D E S ____________________________________________________

#include "config.h"


//_____ D E F I N I T I O N S ______________________________________________


//! This macro function allows to read device ID1 of the product.
//!
//! @return word  Read value
//!
#define Flash_read_id1()            ( (*boot_flash_read_sig)(0x0000))

//! This macro function allows to read device ID2 of the product.
//!
//! @return word  Read value
//!
#define Flash_read_id2()            ( (*boot_flash_read_sig)(0x0002))

//! This macro function allows to read device ID3 of the product.
//!
//! @return word  Read value
//!
#define Flash_read_id3()            ( (*boot_flash_read_sig)(0x0004))

//! This macro function allows to read the OSCAL byte of the product.
//!
//! @return word  Read value
//!
#define Flash_read_osccal()         ( (*boot_flash_read_sig)(0x0001))

//! This macro function allows to read the low fuse byte of the product.
//!
//! @return word  Read value
//!
#define Flash_read_fuse_low()       ( (*boot_flash_read_fuse)(0x0000))

//! This macro function allows to read device high fuse byte of the product.
//!
//! @return word  Read value
//!
#define Flash_read_fuse_high()      ( (*boot_flash_read_fuse)(0x0003))

//! This macro function allows to read extended fuse byte of the product.
//!
//! @return word  Read value
//!
#define Flash_read_fuse_extended()  ( (*boot_flash_read_fuse)(0x0002))

//! This macro function allows to read lock byte of the product.
//!
//! @return word  Read value
//!
#define Flash_read_lock()           ( (*boot_flash_read_fuse)(0x0001))


//_____ D E C L A R A T I O N S ____________________________________________


extern U8 (*boot_flash_read_sig) (unsigned long adr);
extern U8 (*boot_flash_read_fuse) (unsigned long adr);


//! This function checks the presence of bootloader
//!
//! @return FALSE, if no code loaded in bootloader area
//!
Bool flash_lib_check( void );

//! This function allows to write a byte in the flash memory.
//!
//! @param addr_byte   Address in flash memory to write the byte.
//! @param value    Value to write in the flash memory
//!
void flash_wr_byte(Uint32 addr_byte, Uchar value);

//! This function allows to write up to 65535 bytes in the flash memory.
//! This function manages alignement issue.
//!
//! @param *src   Address of data to write.
//! @param dst    Start address in flash memory where write data
//! @param n      number of byte to write
//!
Uchar flash_wr_block(Byte _MemType_* src, Uint32 dst, U16 n);

//! This function allows to read a byte in the flash memory.
//!
//! @param *add   Address of flash memory to read.
//! @return byte  Read value
//!
U8 flash_rd_byte(Uchar farcode* addr);

//! This function allows to read a word in the flash memory.
//!
//! @param *add   Address of flash memory to read.
//! @return word  Read value
//!
U16 flash_rd_word(U16 farcode* addr);

#endif  // FLASH_LIB_H 














