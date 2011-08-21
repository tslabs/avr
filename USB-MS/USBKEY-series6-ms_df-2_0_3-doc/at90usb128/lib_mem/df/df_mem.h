/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the interface routines of Data Flash memory.
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
 
#ifndef _DFMEM_H_
#define _DFMEM_H_

#include "conf/conf_access.h"
#include "modules/control_access/ctrl_status.h"


//---- CONTROL FONCTIONS ----

void           df_mem_init(void);
Ctrl_status    df_test_unit_ready(void);
Ctrl_status    df_read_capacity( U32 _MEM_TYPE_SLOW_ *u32_nb_sector );
Bool           df_wr_protect(void);
Bool           df_removal(void);


//---- ACCESS DATA FONCTIONS ----

// Functions read/write memory to usb interface
Ctrl_status    df_read_10( U32 addr , U16 nb_sector );
Ctrl_status    df_write_10( U32 addr , U16 nb_sector );

// Standard functions for open in read/write mode from HOST
Ctrl_status    df_host_write_10( U32 addr , U16 nb_sector );
Ctrl_status    df_host_read_10( U32 addr , U16 nb_sector );

// Functions read/write memory sector (512B) to RAM buffer
Ctrl_status    df_ram_2_df( U32 addr, U8 *ram);
Ctrl_status    df_df_2_ram( U32 addr, U8 *ram);


#endif   // _DFMEM_H_

