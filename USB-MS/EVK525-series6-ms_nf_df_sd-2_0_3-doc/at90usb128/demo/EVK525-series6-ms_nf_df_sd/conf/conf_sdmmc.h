/*This file has been prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the possible external configuration of the SD/MMC interface
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


//_____ D E F I N I T I O N ________________________________________________


//!  USB <-> SD/MMC Access Control
//!
//! Values: ENABLE : enable the SD/MMD functions relatives to USB transfer
//!         DISABLE : disable the SD/MMD functions relatives to USB transfer
//!
#define     MMC_SD_USB              ENABLE



//! RAM <-> SD/MMC Access Control
//!
//! Values: ENABLE : enable the SD/MMD functions relatives to RAM transfer (/!\ a 512 bytes buffer is created in RAM)
//!         DISABLE : disable the SD/MMC functions relatives to RAM transfer (internal memory)
//!
#define     MMC_SD_RAM              ENABLE



//! SD/MMC CID Register Management
//!
//! Values: ENABLE : the CID register is transfered from the card and stored into a special array
//!         DISABLE : CID register unused
//!
#define     MMC_SD_READ_CID         DISABLE



//! Function linker for NF access indications
//!
//! Values :   Sdmmc_access_signal_on()      The linked function is called when a read/write operation to SD/MMC starts
//!            Sdmmc_access_signal_off()     The linked function is called when the read/write operation to SD/MMC ends
//!
#define  Sdmmc_access_signal_on()
#define  Sdmmc_access_signal_off()












