/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief  This file contains the interface routines of Data Flash memory.
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

//_____  I N C L U D E S ___________________________________________________

#include "config.h"                         // system configuration
#include "conf_sdmmc.h"
#include "mmc_sd_mem.h"
#include "mmc_sd.h"

//_____ M A C R O S ________________________________________________________


//_____ P R I V A T E    D E C L A R A T I O N _____________________________


//_____ D E F I N I T I O N ________________________________________________


//extern xdata U32 mmc_sd_mem_size;
extern            xdata U32 MMC_SD_DISK_SIZE;
extern volatile   U32 mmc_sd_last_address;
extern            bit mmc_sd_init_done;

U8                mmc_sd_presence_status = 0;
extern            bit mmc_sd_init_done;



//_____ D E C L A R A T I O N ______________________________________________

//!
//! @brief This function initializes the hw/sw ressources required to drive the MMC_SD.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return none
//!
//!/
void mmc_sd_mem_init(void)
{
   mmc_sd_init();        // Init the SPI bus and the MMC/SD card
}


//!
//! @brief This function tests the state of the MMC_SD memory and sends it to the Host.
//!        For a PC, this device is seen as a removable media
//!        Before indicating any modification of the status of the media (GOOD->NO_PRESENT or vice-versa),
//!         the function must return the BUSY data to make the PC accepting the change
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return                Ctrl_status
//!   Media is ready       ->    CTRL_GOOD
//!   Media not present    ->    CTRL_NO_PRESENT
//!   Media has changed    ->    CTRL_BUSY
//!/
Ctrl_status mmc_sd_test_unit_ready(void)
{
  Sdmmc_access_signal_on();
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }
  switch (mmc_sd_presence_status)
  {
    case MMCSD_REMOVED:
      mmc_sd_init_done = FALSE;
      if (OK == mmc_sd_mem_check())
      {
        mmc_sd_presence_status = MMCSD_INSERTED;
        Sdmmc_access_signal_off();
        return CTRL_BUSY;
      }
      Sdmmc_access_signal_off();
      return CTRL_NO_PRESENT;

    case MMCSD_INSERTED:
      if (OK != mmc_sd_mem_check())
      {
        mmc_sd_presence_status = MMCSD_REMOVING;
        mmc_sd_init_done = FALSE;
        Sdmmc_access_signal_off();
        return CTRL_BUSY;
      }
      Sdmmc_access_signal_off();
      return CTRL_GOOD;

    case MMCSD_REMOVING:
      mmc_sd_presence_status = MMCSD_REMOVED;
      Sdmmc_access_signal_off();
      return CTRL_NO_PRESENT;

    default:
      mmc_sd_presence_status = MMCSD_REMOVED;
      Sdmmc_access_signal_off();
      return CTRL_BUSY;
  }

  Sdmmc_access_signal_off();
  return CTRL_BUSY;
/*
  if (OK==mmc_sd_mem_check())
  {
    if (mmc_sd_status_changed == FALSE)
    {
      mmc_sd_status_changed = TRUE;
      return CTRL_BUSY;     // BUSY token must be returned to indicate a status change !
    }
    else
      return CTRL_GOOD;     // the 2nd time the host will ask for unit_ready, we can answer GOOD if we have returned BUSY first !
  }
  else
  {
    if (mmc_sd_status_changed == TRUE)
    {
      mmc_sd_status_changed = FALSE;
      return CTRL_BUSY;     // BUSY token must be returned to indicate a status change !
    }
    else
      return CTRL_NO_PRESENT;
  }
*/
}


//!
//! @brief This function gives the address of the last valid sector.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param *u32_nb_sector  number of sector (sector = 512B). OUT
//!
//! @return                Ctrl_status
//!   Media ready          ->  CTRL_GOOD
//!   Media not present    ->  CTRL_NO_PRESENT
//!/
Ctrl_status mmc_sd_read_capacity( U32 _MEM_TYPE_SLOW_ *u32_nb_sector )
{
//   mmc_sd_check_presence();   // ommited because creates interferences with "mmc_sd_test_unit_ready()" function
   Sdmmc_access_signal_on();
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }
   if (mmc_sd_init_done == TRUE)
   {
     *u32_nb_sector = mmc_sd_last_block_address;
     Sdmmc_access_signal_off();
     return CTRL_GOOD;
   }
   else
   {
     Sdmmc_access_signal_off();
     return CTRL_NO_PRESENT;
   }
}


//!
//! @brief This function returns the write protected status of the memory.
//!
//! Only used by memory removal with a HARDWARE SPECIFIC write protected detection
//! !!! The customer must unplug the memory to change this write protected status,
//! which cannot be for a MMC_SD.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @return FALSE  -> the memory is not write-protected (always)
//!/
Bool  mmc_sd_wr_protect(void)
{
   return FALSE;
}


//!
//! @brief This function tells if the memory has been removed or not.
//!
//! @param none
//!
//! @warning Code:?? bytes (function code length)
//!
//! @return FALSE  -> The memory isn't removed
//!/
Bool  mmc_sd_removal(void)
{
  return FALSE;
//  return ((OK == mmc_sd_check_presence()) ? FALSE : TRUE);
}



//------------ STANDARD FUNCTIONS to read/write the memory --------------------

//!
//! @brief This function performs a read operation of n sectors from a given address on.
//! (sector = 512B)
//!
//!         DATA FLOW is: MMC_SD => USB
//!
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param addr         Sector address to start the read from
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status mmc_sd_read_10( U32 addr , U16 nb_sector )
{
bit status;
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }
   if (mmc_sd_init_done == TRUE)
   {
     Sdmmc_access_signal_on();
     mmc_sd_read_open(addr);
     status = mmc_sd_read_sector(nb_sector);
     mmc_sd_read_close();
     Sdmmc_access_signal_off();
     if (status == OK)
        return CTRL_GOOD;
     else
        return CTRL_NO_PRESENT;
   }
   else
     return CTRL_NO_PRESENT;
}


//! This fonction initialise the memory for a write operation
//!
//!         DATA FLOW is: USB => MMC_SD
//!
//!
//! (sector = 512B)
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status mmc_sd_write_10( U32 addr , U16 nb_sector )
{
  bit status;
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }
   if (mmc_sd_init_done == TRUE)
   {
     Sdmmc_access_signal_on();
     mmc_sd_write_open(addr);
     status = mmc_sd_write_sector(nb_sector);
     mmc_sd_write_close();
     Sdmmc_access_signal_off();
     if (status == OK)
       return CTRL_GOOD;
     else
       return CTRL_NO_PRESENT;
   }
   else
     return CTRL_NO_PRESENT;
}


//------------ Standard functions for read/write 1 sector to 1 sector ram buffer -----------------

//! This fonction initialise the memory for a write operation
//! from ram buffer to MMC/SD (1 sector)
//!
//!         DATA FLOW is: RAM => MMC
//!
//! (sector = 512B)
//! @param addr         Sector address to write
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    mmc_ram_2_mmc(U32 addr, U8 *ram)
{
#if (MMC_SD_RAM == ENABLE)
   Sdmmc_access_signal_on();
   mmc_sd_check_presence();
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }

   if (mmc_sd_init_done == TRUE)
   {
     mmc_sd_write_open(addr);
     if (KO == mmc_sd_write_sector_from_ram(ram))
     {
       mmc_sd_write_close();
       Sdmmc_access_signal_off();
       return CTRL_NO_PRESENT;
     }
     mmc_sd_write_close();
     Sdmmc_access_signal_off();
     return CTRL_GOOD;
   }
   Sdmmc_access_signal_off();
#endif   // (MMC_SD_RAM == ENABLE)
     return CTRL_NO_PRESENT;
}

//! This fonction read 1 sector from MMC/SD to ram buffer
//!
//!         DATA FLOW is: MMC => RAM
//!
//! (sector = 512B)
//! @param addr         Sector address to read
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    mmc_mmc_2_ram( U32 addr, U8 *ram)
{
#if (MMC_SD_RAM == ENABLE)
   Sdmmc_access_signal_on();
   mmc_sd_check_presence();
   if (mmc_sd_init_done == FALSE)
   {
      mmc_sd_mem_init();
   }

   if (mmc_sd_init_done == TRUE)
   {
     mmc_sd_read_open(addr);
     if (KO == mmc_sd_read_sector_to_ram(ram))
     {
       mmc_sd_write_close();
       Sdmmc_access_signal_off();
       return CTRL_NO_PRESENT;
     }
     mmc_sd_read_close();
     Sdmmc_access_signal_off();
     return CTRL_GOOD;
   }
   Sdmmc_access_signal_off();
#endif   // (MMC_SD_RAM == ENABLE)
   return CTRL_NO_PRESENT;
}


