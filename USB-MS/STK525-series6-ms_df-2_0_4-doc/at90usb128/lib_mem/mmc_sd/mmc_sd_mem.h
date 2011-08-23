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
#ifndef _MMC_SD_MEM_H_
#define _MMC_SD_MEM_H_

#include "conf/conf_access.h"
#include "modules/control_access/ctrl_status.h"



//_____ D E F I N I T I O N S ______________________________________________

#define   MMCSD_REMOVED       0
#define   MMCSD_INSERTED      1
#define   MMCSD_REMOVING      2


//---- CONTROL FONCTIONS ----

// those fonctions are declared in mmc_sd_mem.h
void           mmc_sd_spi_init(void);
void           mmc_sd_mem_init(void);
Ctrl_status    mmc_sd_test_unit_ready(void);
Ctrl_status    mmc_sd_read_capacity( U32 _MEM_TYPE_SLOW_ *u32_nb_sector );
Bool           mmc_sd_wr_protect(void);
Bool           mmc_sd_removal(void);


//---- ACCESS DATA FONCTIONS ----

// Standard functions for open in read/write mode the device
Ctrl_status    mmc_sd_read_10( U32 addr , U16 nb_sector );
Ctrl_status    mmc_sd_write_10( U32 addr , U16 nb_sector );

// Standard functions for read/write 1 sector to 1 sector ram buffer
Ctrl_status    mmc_ram_2_mmc( U32 addr, U8 *ram);
Ctrl_status    mmc_mmc_2_ram( U32 addr, U8 *ram);

#endif   // _MMC_SD_MEM_H_

