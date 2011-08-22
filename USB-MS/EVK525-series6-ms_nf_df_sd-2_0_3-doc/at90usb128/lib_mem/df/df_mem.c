/*This file has been prepared for Doxygen automatic documentation generation.*/
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

//_____  I N C L U D E S ___________________________________________________

#include "config.h"                         // system configuration
#include "df_mem.h"
#include "df.h"

//_____ D E F I N I T I O N ________________________________________________

// Global value to manage the write protection on DataFlash
Bool g_b_df_protected = FALSE;
Bool g_b_df_protected_last = FALSE;

void  df_check_init( void );


//_____ D E C L A R A T I O N ______________________________________________


//! This function initializes the hw/sw ressources required to drive the DF.
//!
void df_mem_init(void)
{
   df_init();        // Init the DF driver and its communication link.
}


//! This function tests the state of the DF memory.
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Not initialize ->    CTRL_BUSY
//!   Else           ->    CTRL_NO_PRESENT
//!
Ctrl_status df_test_unit_ready(void)
{
   if( g_b_df_protected != g_b_df_protected_last )
   {
      g_b_df_protected_last = g_b_df_protected;
      return CTRL_BUSY;
   }
   return( (OK==df_mem_check()) ? CTRL_GOOD : CTRL_NO_PRESENT);
}


//! @brief This function gives the address of the last valid sector.
//!
//! @param *u32_nb_sector  number of sector (sector = 512B)
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Not initialize ->    CTRL_BUSY
//!   Else           ->    CTRL_NO_PRESENT
//!
Ctrl_status df_read_capacity( U32 _MEM_TYPE_SLOW_ *u32_nb_sector )
{
#ifdef DF_4_MB              // AT45DB321 memories
   *u32_nb_sector = ((DF_NB_MEM*4*1024L*1024L)/512)-1;
#endif
#ifdef DF_8_MB              // AT45DB642 memories
   *u32_nb_sector = ((DF_NB_MEM*8*1024L*1024L)/512)-1;
#endif
   return df_test_unit_ready();
}


//! This function returns the write protected status of the memory.
//!
//! Only used by memory removal with a HARDWARE SPECIFIC write protected detection
//! !!! The customer must unplug the memory to change this write protected status,
//! which cannot be for a DF.
//!
//! @return FALSE, the memory is not write-protected
//!
Bool  df_wr_protect(void)
{
   return g_b_df_protected;
}


//! This function tells if the memory has been removed or not.
//!
//! @return FALSE, The memory isn't removed
//!
Bool  df_removal(void)
{
   return FALSE;
}



//------------ STANDARD FUNCTIONS to read/write the memory --------------------

//! This function performs a read operation of n sectors from a given address to USB
//!
//! @param addr         Sector address to start the read from
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status df_read_10( U32 addr , U16 nb_sector )
{
   U8 status = OK;
#if   (DF_NB_MEM == 1)     // 1 DATAFLASH
   df_read_open(addr);                    // wait device is not busy, then send command & address
   status = df_read_sector(nb_sector);             // transfer data from memory to USB
   
#else                      // 2 or 4 DATAFLASH
   U32   next_sector_addr = addr;
   U16   nb_sectors_remaining = nb_sector;
   
   #ifdef DF_4_MB             // 512B PAGES
   while( (nb_sectors_remaining != 0) && (status == OK))
   {
      df_read_open(next_sector_addr);     // wait device is not busy, then send command & address
      status = df_read_sector(1);                  // transfer the page from memory to USB
      df_read_close();
      nb_sectors_remaining--;
      next_sector_addr++;
   }
   #else                      // 1024B PAGES
   while( (nb_sectors_remaining != 0) && (status == OK))
   {
      df_read_open(next_sector_addr);     // wait device is not busy, then send command & address
      if ((LSB0(next_sector_addr)&0x01) == 0)
      {
        if (nb_sectors_remaining == 1)
        {
           status = df_read_sector(1);
           df_read_close();
           nb_sectors_remaining--;
           next_sector_addr++;
        }
        else
        {
          status = df_read_sector(2);
          df_read_close();
          nb_sectors_remaining -= 2;
          next_sector_addr += 2;
        }
      }
      else
      {
        status = df_read_sector(1);
        df_read_close();
        nb_sectors_remaining--;
        next_sector_addr++;
      }
   }
   #endif
#endif
   
   df_read_close();
   if(status == KO)
      return CTRL_FAIL;
   return CTRL_GOOD;
}


//! This function performs a write operation of n sectors to a given address from USB
//!
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status df_write_10( U32 addr , U16 nb_sector )
{
#if   (DF_NB_MEM != 1)                 // if more than 1 memory, variables are needed for zones mangement
   U32   next_sector_addr = addr;
   U16   nb_sectors_remaining = nb_sector;
#endif

   if( g_b_df_protected ) return CTRL_FAIL;

#if      (DF_NB_MEM == 1)  /* 1 DATAFLASH */
   df_write_open(addr);                    // wait device is not busy, then send command & address
   if( KO == df_write_sector(nb_sector) )  // transfer data from memory to USB
   {
      df_mem_init();
      return CTRL_FAIL;
   }
#else                      /* 2 or 4 DATAFLASH */
   #ifdef DF_4_MB       // 512B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_write_open(next_sector_addr);     // wait device is not busy, then send command & address
      if( KO == df_write_sector(1))        // transfer the page from memory to USB
      {
         df_mem_init();
         return CTRL_FAIL;
      }
      df_write_close();
      nb_sectors_remaining--;
      next_sector_addr++;
   }
   #else                // 1024B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_write_open(next_sector_addr);     // wait device is not busy, then send command & address
      if ((LSB0(next_sector_addr)&0x01) == 0)
      {
        if (nb_sectors_remaining == 1)
        {
          if( KO == df_write_sector(1))    // transfer the page from memory to USB
          {
             df_mem_init();
             return CTRL_FAIL;
          }
          df_write_close();
          nb_sectors_remaining--;
          next_sector_addr++;
        }
        else
        {
          if( KO == df_write_sector(2))    // transfer the page from memory to USB
          {
             df_mem_init();
             return CTRL_FAIL;
          }
          df_write_close();
          nb_sectors_remaining -= 2;
          next_sector_addr += 2;
        }
      }
      else
      {
        if( KO == df_write_sector(1))      // transfer the page from memory to USB
        {
           df_mem_init();
           return CTRL_FAIL;
        }
        df_write_close();
        nb_sectors_remaining--;
        next_sector_addr++;
      }
   }
   #endif
#endif
   df_write_close();                    // unselect memory
   return CTRL_GOOD;
}



//------------ FUNCTIONS FOR USED WITH USB HOST MODE (host mass storage)-----------------

#if( USB_HOST_FEATURE==ENABLE )
//! This fonction initialise the memory for a write operation
//! in usb host mode
//!
//!         DATA FLOW is: DF => USB
//!
//! (sector = 512B)
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status df_host_write_10( U32 addr , U16 nb_sector ) //! Write_10 in host mode means reading the DF
{
#if   (DF_NB_MEM != 1)                 // if more than 1 memory, variables are needed for zones mangement
   U32   next_sector_addr = addr;
   U16   nb_sectors_remaining = nb_sector;
#endif

#if   (DF_NB_MEM == 1)     /* 1 DATAFLASH */
   df_read_open(addr);                    // wait device is not busy, then send command & address
   df_host_read_sector(nb_sector);             // transfer data from memory to USB
#else                      /* 2 or 4 DATAFLASH */
   #ifdef DF_4_MB             // 512B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_read_open(next_sector_addr);     // wait device is not busy, then send command & address
      df_host_read_sector(1);                  // transfer the page from memory to USB
      df_read_close();
      nb_sectors_remaining--;
      next_sector_addr++;
   }
   #else                      // 1024B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_read_open(next_sector_addr);     // wait device is not busy, then send command & address
      if ((LSB0(next_sector_addr)&0x01) == 0)
      {
        if (nb_sectors_remaining == 1)
        {
           df_host_read_sector(1);
           df_read_close();
           nb_sectors_remaining--;
           next_sector_addr++;
        }
        else
        {
          df_host_read_sector(2);
          df_read_close();
          nb_sectors_remaining -= 2;
          next_sector_addr += 2;
        }
      }
      else
      {
        df_host_read_sector(1);
        df_read_close();
        nb_sectors_remaining--;
        next_sector_addr++;
      }
   }
   #endif
#endif
   df_read_close();                    // unselect memory
   return CTRL_GOOD;
}


//! This fonction initialise the memory for a read operation
//! in usb host mode
//!
//!         DATA FLOW is: USB => DF
//!
//! (sector = 512B)
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status df_host_read_10( U32 addr , U16 nb_sector )  //! Read_10 in host mode means writing the DF
{
#if   (DF_NB_MEM != 1)                 // if more than 1 memory, variables are needed for zones mangement
   U32   next_sector_addr = addr;
   U16   nb_sectors_remaining = nb_sector;
#endif

#if      (DF_NB_MEM == 1)  /* 1 DATAFLASH */
   df_write_open(addr);                    // wait device is not busy, then send command & address
   df_host_write_sector(nb_sector);             // transfer data from memory to USB
#else                      /* 2 or 4 DATAFLASH */
   #ifdef DF_4_MB       // 512B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_write_open(next_sector_addr);     // wait device is not busy, then send command & address
      df_host_write_sector(1);                  // transfer the page from memory to USB
      df_write_close();
      nb_sectors_remaining--;
      next_sector_addr++;
   }
   #else                // 1024B PAGES
   while (nb_sectors_remaining != 0)
   {
      df_write_open(next_sector_addr);     // wait device is not busy, then send command & address
      if ((LSB0(next_sector_addr)&0x01) == 0)
      {
        if (nb_sectors_remaining == 1)
        {
          df_host_write_sector(1);
          df_write_close();
          nb_sectors_remaining--;
          next_sector_addr++;
        }
        else
        {
          df_host_write_sector(2);
          df_write_close();
          nb_sectors_remaining -= 2;
          next_sector_addr += 2;
        }
      }
      else
      {
        df_host_write_sector(1);
        df_write_close();
        nb_sectors_remaining--;
        next_sector_addr++;
      }
   }
   #endif
#endif
   df_write_close();                    // unselect memory
   return CTRL_GOOD;
}
#endif   // USB_HOST_FEATURE==ENABLE


//------------ Standard functions for read/write 1 sector to 1 sector ram buffer -----------------


//! This function performs a read operation of 1 sector from a given address to RAM buffer
//!
//! @param addr         Sector address to read
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    df_df_2_ram( U32 addr, U8 *ram)
{
   df_read_open(addr);
   df_read_sector_2_ram(ram);
   df_read_close();
   return CTRL_GOOD;
}


//! This function performs a write operation of 1 sector to a given address from RAM buffer
//!
//! @param addr         Sector address to write
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    df_ram_2_df(U32 addr, U8 *ram)
{
   df_write_open(addr);
   df_write_sector_from_ram(ram);
   df_write_close();
   return CTRL_GOOD;
}

