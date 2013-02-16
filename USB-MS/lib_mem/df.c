/*This file is prepared for Doxygen automatic documentation generation.*/
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

//_____ I N C L U D E S ____________________________________________________

#include "config.h"                         // system configuration
#include "usb_drv.h"            // usb driver definition
#include "spi_drv.h"            // spi driver definition
#include "df.h"                             // dataflash definition


//_____ D E F I N I T I O N ________________________________________________

static   U32   gl_ptr_mem;                   // memory data pointer
static   U8    df_mem_busy;                  // memories in busy state 
static   U8    df_select;                    // current memory selected


//_____ D E C L A R A T I O N _____________________________________________

static  void    df_wait_busy(void);
static  void    df_chipselect_current(void);


//! This function initializes the SPI bus over which the DF is controlled
//!
void df_init (void)
{
   // Init the SPI communication link between the DF and the DF driver.
   Df_init_spi();
   Spi_select_master();
   Spi_set_mode(SPI_MODE_3);
   Spi_init_bus();
   Spi_set_rate(SPI_RATE_0);  // SCK freq == fosc/2.
   Spi_disable_ss();
   Spi_enable();

   // Data Flash Controller init.
   df_mem_busy = 0;           // all memories ready
   df_select = 0;
}


//! This function selects a DF memory according to the sector pointer
//!
//! @verbatim
//! The "df_select" variable contains the current memory addressed
//! Refer to the documentation of the function df_translate_addr() for more information about zones management
//! @endverbatim
//!
static void df_chipselect_memzone(U8 memzone)
{
#if (DF_NB_MEM == 1)
   df_select = 0;
#else
   #if (DF_NB_MEM == 2)
      #if (DF_PAGE_SIZE == 512)
   df_select = (memzone>>0)&0x01;
      #else
   df_select = (memzone>>1)&0x01;
      #endif
   #else
      #if (DF_PAGE_SIZE == 512)
   df_select = (memzone>>0)&0x03;
      #else
   df_select = (memzone>>1)&0x03;
      #endif
   #endif
#endif
}


//! This function physically selects the current addressed memory
//!
static void df_chipselect_current(void)
{
   Df_desel_all();
   switch (df_select)
   {
      case 0:
      Df_select_0();
      break;
#if (DF_NB_MEM > 1)
      case 1:
      Df_select_1();
      break;
#endif
#if (DF_NB_MEM > 2)
      case 2:
      Df_select_2();
      break;
      case 3:
      Df_select_3();
      break;
#endif
   }
}


//! This function translates the logical sector address to the physical byte address (1 logical sector = 512 bytes)
//! In function of the memory configuration (number 1x/2x/4x, pages 512b/1024b) :
//!
//!   MEMORIES WITH PAGES OF 512 BYTES :
//!   ==================================
//!   Consider the logical sector address as "xx..xxba", where 'b' and 'a' are the two last bits, and "xx..xx" an undefined number of more significant bits
//!      - If 1 memory is used, this logical sector address directly matches with the physical sector address
//!        So the physical sector address is "xx..xxba"
//!      - If 2 memories are used, the bit 'a' indicates the memory concerned, and the rest of the address indicates the physical address
//!        So the physical sector address is "xx...xxb"
//!      - If 4 memories are used, the bits 'ba' indicates the memory concerned, and the rest of the address indicates the physical address
//!        So the physical sector address is "xx....xx"
//!
//!   MEMORIES WITH PAGES OF 1024 BYTES :
//!   ==================================
//!   Consider the logical sector address as "xx..xxcba", where 'c', 'b' and 'a' are the three last bits, and "xx..xx" an undefined number of more significant bits
//!      - If 1 memory is used, this logical sector address directly matches with the physical sector address
//!        So the physical sector address is "xx..xxcba"
//!      - If 2 memories are used, the bit 'b' indicates the memory concerned, and the rest of the address indicates the physical address
//!        So the physical sector address is "xx...xxca"
//!      - If 4 memories are used, the bits 'cb' indicates the memory concerned, and the rest of the address indicates the physical address
//!        So the physical sector address is "xx....xxa"
//!
//!
//! @param  log_sect_addr     logical sector address
//!
//! @return U32               physical byte address
//!
static U32 df_translate_addr(Uint32 log_sect_addr)
{
#if (DF_NB_MEM == 1)
   return (log_sect_addr << 9);  // this case if only one memory...
#else
   #if (DF_NB_MEM == 2)
      #if (DF_PAGE_SIZE == 512)
   return ((log_sect_addr&0xFFFFFFFE) << 8);
      #else
   return (((log_sect_addr&0xFFFFFFFC) << 8) | ((log_sect_addr&0x00000001) << 9));
      #endif
   #else
      #if (DF_PAGE_SIZE == 512)
   return ((log_sect_addr&0xFFFFFFFC) << 7);
      #else
   return (((log_sect_addr&0xFFFFFFF8) << 7) | ((log_sect_addr&0x00000001) << 9));
      #endif
   #endif
#endif
}


//! This function performs a memory check on all DF.
//!
//!
//! @return bit
//!   The memory is ready     -> OK
//!   The memory check failed -> ERR
//!
Bool df_mem_check (void)
{
   // DF memory check.
   for(df_select=0; df_select<DF_NB_MEM; df_select++)
   {
      df_chipselect_current();
      Spi_write_data(DF_RD_STATUS); // Send the read status register cmd
                                    // + 1st step to clear the SPIF bit
      Spi_write_data(0);            // dummy write that:
                                    // - finalize the clear of the SPIF bit (access to SPDR)
                                    // - get status register
                                    // - does the 1st step to clear the SPIF bit 
      // Following Spi_read_data() finalize the clear of SPIF by accessing SPDR.
      // Check the DF density.
      if ((Spi_read_data() & DF_MSK_DENSITY) != DF_DENSITY)
      {   
        // Unexpected value.
        Df_desel_all();
        return (ERR);
      }
      Df_desel_all();
   }
   return OK;
}


//! This function waits until the DataFlash is not busy.
//!
static void df_wait_busy (void)
{
   // Read the status register until the DataFlash is not busy.
   df_chipselect_current();
   Spi_write_data(DF_RD_STATUS); // Send the read status register cmd
                                 // + 1st step to clear the SPIF bit
   Spi_write_data(0);            // dummy write that:
                                 // - finalize the clear of the SPIF bit (access to SPDR)
                                 // - get status register
                                 // - does the 1st step to clear the SPIF bit
   // Following Spi_read_data() finalize the clear of SPIF by accessing SPDR
   while ((Spi_read_data() & DF_MSK_BIT_BUSY) == DF_MEM_BUSY)
   {
      Spi_write_data(0);         // dummy write to get new status
                                 // + 1st step to clear the SPIF bit
   }
   Df_desel_all();               // unselect memory to leave STATUS request mode
   Spi_ack_write();              // Final step to clear the SPIF bit.
}


//! This function opens a DF memory in read mode at a given sector address.
//!
//! NOTE: Address may not be synchronized on the beginning of a page (depending on the DF page size).
//!
//! @param  pos   Logical sector address
//!
//! @return bit
//!   The open succeeded      -> OK
//!
Bool df_read_open (Uint32 pos)
{
   // Set the global memory ptr at a Byte address.
   gl_ptr_mem = df_translate_addr(pos);
   
   // Select the DF that memory "pos" points to (the "df_select" variable will be updated)
   df_chipselect_memzone(LSB0(pos));
   
   // If the DF memory is busy, wait until it's not.
   if (is_df_busy(df_select))
   {
    df_release_busy(df_select);
    df_wait_busy();                              // wait end of programming
   }
   
   // Physically assert the selected dataflash
   df_chipselect_current();
   
   //#
   //# Initiate a page read at a given sector address.
   //#
   // Send read main command, + first step to clear the SPIF bit
   Spi_write_data(DF_RD_MAIN);
   // Final step to clear the SPIF bit will be done on the next write
   
   // Send the three address Bytes made of:
   // - the page-address(first xbits),
   // - the Byte-address within the page(last ybits).
   // (x and y depending on the DF type).
   // NOTE: the bits of gl_ptr_mem above the 24bits are not useful for the local
   // DF addressing. They are used for DF discrimination when there are several
   // DFs.
   Spi_write_data((MSB1(gl_ptr_mem) << DF_SHFT_B1) | (MSB2(gl_ptr_mem) >> DF_SHFT_B2));
   Spi_write_data(((MSB2(gl_ptr_mem) & ~DF_PAGE_MASK) << DF_SHFT_B1) | (MSB2(gl_ptr_mem) & DF_PAGE_MASK));
   Spi_write_data(MSB3(gl_ptr_mem));
   // Final step to clear the SPIF bit will be done on the next write
   
   // 4 dummy writes for reading delay
   Spi_write_data(0xFF);
   Spi_write_data(0xFF);
   Spi_write_data(0xFF);
   Spi_write_data(0xFF); // Tx 0xFF, first step to clear the SPIF bit
   Spi_ack_write();      // Final step to clear the SPIF bit.
   
   return OK;
}


//! This function unselects the current DF memory.
//!
void df_read_close (void)
{
  Df_desel_all();   // Unselect memory
}


//! This function is optimized and writes nb-sector * 512 Bytes from DataFlash memory to USB controller
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_read_open() function,
//!   - The USB EPIN must have been previously selected,
//!   - USB ping-pong buffers are free,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @param nb_sector    number of contiguous sectors to read [IN]
//!
//! @return The read succeeded   -> OK
//!
Bool df_read_sector (Uint16 nb_sector)
{
   U8 i;
#ifdef DF_CODE_SIZE_OPTIMIZATION   
   U8 j;
#endif
   do
   {
      for (i = 8; i != 0; i--)
      {
         Disable_interrupt();    // Global disable.
         
         // Principle: send any Byte to get a Byte.
         // Spi_write_data(0): send any Byte + 1st step to clear the SPIF bit.
         // Spi_read_data(): get the Byte + final step to clear the SPIF bit.         
#ifdef DF_CODE_SIZE_OPTIMIZATION
         for(j=0;j<64;j++)
         {
            Spi_write_data(0); Usb_write_byte(Spi_read_data());
         }
#else         
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
         Spi_write_data(0); Usb_write_byte(Spi_read_data());
#endif
         Usb_send_in();          // Send the FIFO IN content to the USB Host.

         Enable_interrupt();     // Global interrupt re-enable.

         // Wait until the tx is done so that we may write to the FIFO IN again.
         while(Is_usb_write_enabled()==FALSE)
         {
            if(!Is_usb_endpoint_enabled())
               return ERR; // USB Reset
         }         
      }
      gl_ptr_mem += 512;         // increment global address pointer
      nb_sector--;               // 1 more sector read
      #if (DF_NB_MEM == 1)       // end of page ?
         #if (DF_PAGE_SIZE == 512)
            Df_desel_all();
            if (nb_sector != 0)
              df_read_open(gl_ptr_mem>>9);
         #else
            if ((MSB2(gl_ptr_mem) & DF_PAGE_MASK) == 0x00)
            {
               Df_desel_all();
               if (nb_sector != 0)
                 df_read_open(gl_ptr_mem>>9);
            }
         #endif
      #endif
   }
   while (nb_sector != 0);

   return OK;
}


//! This function opens a DF memory in write mode at a given sector address.
//!
//! NOTE: If page buffer > 512 bytes, page content is first loaded in buffer to
//! be partially updated by write_byte or write64 functions.
//!
//! @param  pos   Sector address
//!
//! @return The open succeeded      -> OK
//!
Bool df_write_open (Uint32 pos)
{
   // Set the global memory ptr at a Byte address.
   gl_ptr_mem = df_translate_addr(pos);
   
   // Select the DF that memory "pos" points to
   df_chipselect_memzone(LSB0(pos));
   
   // If the DF memory is busy, wait until it's not.
   if (is_df_busy(df_select))
   {
    df_release_busy(df_select);
    df_wait_busy();                              // wait end of programming
   }
   
#if DF_PAGE_SIZE > 512
   // Physically assert the selected dataflash
   df_chipselect_current();
   
   //#
   //# Transfer the current page content in buffer1.
   //#
   // Send Main Mem page to Buffer1 command, + first step to clear the SPIF bit
   Spi_write_data(DF_TF_BUF_1);
   // Final step to clear the SPIF bit will be done on the next write
   
   // Send the three address Bytes made of:
   // - the page-address(first xbits),
   // - remaining don't care bits(last ybits).
   // (x and y depending on the DF type).
   // NOTE: the bits of gl_ptr_mem above the 24bits are not useful for the local
   // DF addressing. They are used for DF discrimination when there are several
   // DFs.
   Spi_write_data((MSB1(gl_ptr_mem) << DF_SHFT_B1) | (MSB2(gl_ptr_mem) >> DF_SHFT_B2));
   Spi_write_data(MSB2(gl_ptr_mem) << DF_SHFT_B1);
   Spi_write_data(0);       // Remaining don't care bits.
   Spi_ack_write();         // Final step to clear the SPIF bit.
   
   Df_desel_all();          // Unselect memory to validate the command
   
   df_wait_busy();               // Wait end of page transfer
#endif
   
   // Physically assert the selected dataflash
   df_chipselect_current();
   
   //#
   //# Initiate a page write at a given sector address.
   //#
   // Send Main Memory Page Program Through Buffer1 command,
   // + first step to clear the SPIF bit
   Spi_write_data(DF_PG_BUF_1);
   // Final step to clear the SPIF bit will be done on the next write
   
   // Send the three address Bytes made of:
   //  (.) the page-address(first xbits),
   //  (.) the Byte-address within the page(last ybits).
   // (x and y depending on the DF type).
   // NOTE: the bits of gl_ptr_mem above the 24bits are not useful for the local
   // DF addressing. They are used for DF discrimination when there are several
   // DFs.
   Spi_write_data((MSB1(gl_ptr_mem) << DF_SHFT_B1) | (MSB2(gl_ptr_mem) >> DF_SHFT_B2));
   Spi_write_data(((MSB2(gl_ptr_mem) & ~DF_PAGE_MASK) << DF_SHFT_B1) | (MSB2(gl_ptr_mem) & DF_PAGE_MASK));
   Spi_write_data(MSB3(gl_ptr_mem));
   Spi_ack_write();         // Final step to clear the SPIF bit.
   
   return OK;
}


//! This function fills the end of the logical sector (512B) and launch page programming.
//!
void df_write_close (void)
{
   //#
   //# While end of logical sector (512B) not reached, zero-fill the remaining
   //# memory Bytes.
   //#
   while ((MSB3(gl_ptr_mem) != 0x00) || ((MSB2(gl_ptr_mem) & 0x01) != 0x00))
   {
      Spi_write_data(0x00);   // - Final step to clear the SPIF bit,
                              // - Write 0x00
                              // - first step to clear the SPIF bit.
      gl_ptr_mem++;
   }
   Spi_ack_write();           // Final step to clear the SPIF bit.
   
   Df_desel_all();            // Launch page programming (or simply unselect memory
                              // if the while loop was not performed).
   df_set_busy(df_select);    // Current memory is busy
}


//! This function is optimized and writes nb-sector * 512 Bytes from USB controller to DataFlash memory
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_write_open() function,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - The USB EPOUT must have been previously selected,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @param nb_sector    number of contiguous sectors to write [IN]
//!
//! @return The write succeeded  -> OK
//!
Bool df_write_sector (Uint16 nb_sector)
{
   U8 i;
#ifdef DF_CODE_SIZE_OPTIMIZATION   
   U8 j;
#endif
   do
   {
      //# Write 8x64b = 512b from the USB FIFO OUT.
      for (i = 8; i != 0; i--)
      {
         // Wait end of rx in USB EPOUT.
         while(!Is_usb_read_enabled())
         {
            if(!Is_usb_endpoint_enabled())
              return ERR; // USB Reset
         }
   
         Disable_interrupt();    // Global disable.

         // SPI write principle: send a Byte then clear the SPIF flag.
         // Spi_write_data(Usb_read_byte()): (.) Final step to clear the SPIF bit,
         //                                  (.) send a Byte read from USB,
         //                                  (.) 1st step to clear the SPIF bit.
#ifdef DF_CODE_SIZE_OPTIMIZATION
         for(j=0;j<64;j++)
         {
            Spi_write_data(Usb_read_byte());
         }
#else
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
         Spi_write_data(Usb_read_byte());
#endif
         Spi_ack_write();        // Final step to clear the SPIF bit.
   
         Usb_ack_receive_out();  // USB EPOUT read acknowledgement.
   
         Enable_interrupt();     // Global re-enable.
      } // for (i = 8; i != 0; i--)

      gl_ptr_mem += 512;         // Update the memory pointer.
      nb_sector--;               // 1 more sector written

      //# Launch page programming if end of page.
      //#
      #if DF_PAGE_SIZE > 512
         // Check if end of 1024b page.
         if ((MSB2(gl_ptr_mem) & DF_PAGE_MASK) == 0x00)
         {
            Df_desel_all();               // Launch page programming
            df_set_busy(df_select);       // memory is busy
            #if (DF_NB_MEM == 1)
               if (nb_sector != 0)
                 df_write_open(gl_ptr_mem>>9);
            #endif
         }
      #else
         // Always end of page.
         Df_desel_all();                  // Launch page programming
         df_set_busy(df_select);          // memory is busy
         #if (DF_NB_MEM == 1)
            if (nb_sector != 0)
              df_write_open(gl_ptr_mem>>9);
         #endif
      #endif
   }
   while (nb_sector != 0);

   return OK;                  // Write done
}


#if( USB_HOST_FEATURE==ENABLE )
//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! USB HOST controller to DataFlash memory
//!
//!         DATA FLOW is: USB => DF
//!
//!
//! NOTE:
//!   - This function should be used only when using the USB controller in HOST mode
//!   - First call must be preceded by a call to the df_write_open() function,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - The USB PIPE OUT must have been previously selected,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to write [IN]
//!
//! @return bit
//!   The write succeeded  -> OK
//!/
bit df_host_write_sector (Uint16 nb_sector)
{
  Byte i;

   do
   {
    //# Write 8x64b = 512b from the USB FIFO OUT.
    for (i = 8; i != 0; i--)
    {
      // Wait end of rx in USB PIPE IN.
      Host_unfreeze_pipe();
      while(Is_host_read_enabled()==FALSE);

      Disable_interrupt();    // Global disable.

      // SPI write principle: send a Byte then clear the SPIF flag.
      // Spi_write_data(Usb_read_byte()): (.) Final step to clear the SPIF bit,
      //                                  (.) send a Byte read from USB,
      //                                  (.) 1st step to clear the SPIF bit.
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_ack_write();        // Final step to clear the SPIF bit.
      Host_ack_in_received();  // USB PIPE IN read acknowledgement.

      Enable_interrupt();     // Global re-enable.
    } // for (i = 8; i != 0; i--)

      gl_ptr_mem += 512;        // Update the memory pointer.
      nb_sector--;              // 1 more sector written

      //# Launch page programming if end of page.
      //#
      #if DF_PAGE_SIZE > 512
         // Check if end of 1024b page.
         if ((MSB2(gl_ptr_mem) & DF_PAGE_MASK) == 0x00)
         {
            Df_desel_all();         // Launch page programming
            df_set_busy(df_select);     // memory is busy
            #if (DF_NB_MEM == 1)
               if (nb_sector != 0)
                 df_write_open(gl_ptr_mem>>9);
            #endif
         }
      #else
         // Always end of page.
         Df_desel_all();           // Launch page programming
         df_set_busy(df_select);     // memory is busy
         #if (DF_NB_MEM == 1)
            if (nb_sector != 0)
              df_write_open(gl_ptr_mem>>9);
         #endif
      #endif
   }
   while (nb_sector != 0);

  return OK;                  // Write done
}

//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! DataFlash memory to USB HOST controller
//!
//!         DATA FLOW is: DF => USB
//!
//!
//! NOTE:
//!   - This function should ne used only when using the USB controller in HOST mode
//!   - First call must be preceded by a call to the df_read_open() function,
//!   - The USB PIPE IN must have been previously selected,
//!   - USB ping-pong buffers are free,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to read [IN]
//!
//! @return bit
//!   The read succeeded   -> OK
//!/
bit df_host_read_sector (Uint16 nb_sector)
{
   U8 i;
   do
   {
      for (i = 8; i != 0; i--)
    {
      Disable_interrupt();    // Global disable.

      // Principle: send any Byte to get a Byte.
      // Spi_write_data(0): send any Byte + 1st step to clear the SPIF bit.
      // Spi_read_data(): get the Byte + final step to clear the SPIF bit.
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());
      Spi_write_data(0); Usb_write_byte(Spi_read_data());

      Enable_interrupt();     // Global re-enable.

      //#
      //# Send the USB FIFO IN content to the USB Host.
      //#
      Host_send_out();       // Send the FIFO from the USB Host.
      Host_unfreeze_pipe();
      // Wait until the tx is done so that we may write to the FIFO IN again.
      while(Is_host_out_sent()==FALSE);
      Host_ack_out_sent();
      }
      gl_ptr_mem += 512;      // increment global address pointer
      nb_sector--;            // 1 more sector read
      #if (DF_NB_MEM == 1)    // end of page ?
         #if (DF_PAGE_SIZE == 512)
            Df_desel_all();
            if (nb_sector != 0)
              df_read_open(gl_ptr_mem>>9);
         #else
            if ((MSB2(gl_ptr_mem) & DF_PAGE_MASK) == 0x00)
            {
               Df_desel_all();
               if (nb_sector != 0)
                 df_read_open(gl_ptr_mem>>9);
            }
         #endif
      #endif
   }
   while (nb_sector != 0);

  return OK;   // Read done.
}
#endif // USB_HOST_FEATURE==ENABLE

//! This function read one DF sector and load it into a ram buffer
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_read_open() function,
//!
//! @param *ram         pointer to ram buffer
//!
//! @return The read succeeded -> OK
//!
Bool df_read_sector_2_ram(U8 *ram)
{
   U16 i;
   for(i=0;i<512;i++)
   {
      Spi_write_data(0);
      *ram=Spi_read_data();
      ram++;
   }
   gl_ptr_mem += 512;     // Update the memory pointer.
   return OK;
}


//! This function write one DF sector from a ram buffer
//!
//! NOTE:
//!   - First call must be preceded by a call to the df_write_open() function,
//!
//! @param *ram         pointer to ram buffer
//!
//! @return The read succeeded   -> OK
//!
Bool df_write_sector_from_ram(U8 *ram)
{
   U16 i;
   for(i=0;i<512;i++)
   {
      Spi_write_data(*ram);
      ram++;
   }
   Spi_ack_write();        // Final step to clear the SPIF bit.
   gl_ptr_mem += 512;      // Update the memory pointer.
   return OK;
}

