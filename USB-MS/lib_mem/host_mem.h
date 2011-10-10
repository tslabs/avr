/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file is a template for writing C software programs.
//!
//!  This file contains the C51 RAM called CRAM management routines which are
//!  used for Mass storage when doing ISP
//!  This file calls routines of the cram_mem.c file
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
#ifndef _HOSTMEM_H_
#define _HOSTMEM_H_

#include "ctrl_status.h"
#include "conf_access.h"
#include "conf_usb.h"

//_____ D E F I N I T I O N S ______________________________________________

// Functions to manage the list of MS devices
void           host_mem_init              ( void );
Bool           host_mem_install           ( void );
void           host_mem_uninstall         ( void );
U8             host_mem_get_lun           ( void );

// Functions to control the state of each LUN (Logical Unit Number)
Ctrl_status    host_mem_test_unit_ready   ( U8 lun );
Ctrl_status    host_mem_read_capacity     ( U8 lun, U32 _MEM_TYPE_SLOW_ *u32_nb_sector );
U8             host_mem_read_sector_size  ( U8 lun );
Bool           host_mem_wr_protect_cache  ( U8 lun );
Bool           host_mem_wr_protect        ( U8 lun );
Bool           host_mem_removal           ( void );
Ctrl_status    host_mem_read_format_capacity( U8 lun );
Ctrl_status    host_mem_inquiry           ( U8 lun );

// Functions to control read/write operations
Ctrl_status    host_mem_mem_2_ram         ( U8 lun, U32 addr, U8 *ram );
Ctrl_status    host_mem_mem_2_ram_stop    ( void );
Ctrl_status    host_mem_ram_2_mem         ( U8 lun, U32 addr, U8 *ram );
Ctrl_status    host_mem_mem_2_ram_ext     ( U8 lun, U32 addr, U8 *ram , U8 u8_nb_sector);
Ctrl_status    host_mem_ram_2_mem_ext     ( U8 lun, U32 addr, U8 *ram , U8 u8_nb_sector);


#endif   // _HOSTMEM_H_
