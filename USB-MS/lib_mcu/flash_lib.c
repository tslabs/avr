/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains a set of routines to perform flash access.
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USBx, ATmega16U4, ATmega32U4, ATmega32U6
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

#include "config.h"
#include "flash.h"
#include "flash_lib.h"


//_____ D E F I N I T I O N S ______________________________________________

// For a futur new management
#define Enable_flash()
#define Disable_flash()


// These functions pointers are used to call functions entry points in bootloader
void  (*boot_flash_page_erase_and_write)  (unsigned long adr)=(void (*)(unsigned long))(LAST_BOOT_ENTRY-12);
U8    (*boot_flash_read_sig)              (unsigned long adr)=(U8 (*)(unsigned long))(LAST_BOOT_ENTRY-10);
U8    (*boot_flash_read_fuse)             (unsigned long adr)=(U8 (*)(unsigned long))(LAST_BOOT_ENTRY-8);
void  (*boot_flash_fill_temp_buffer)      (unsigned int data,unsigned int adr)=(void (*)(unsigned int, unsigned int))(LAST_BOOT_ENTRY-6);
void  (*boot_flash_prg_page)              (unsigned long adr)=(void (*)(unsigned long))(LAST_BOOT_ENTRY-4);
void  (*boot_flash_page_erase)            (unsigned long adr)=(void (*)(unsigned long))(LAST_BOOT_ENTRY-2);
void  (*boot_lock_wr_bits)                (unsigned char val)=(void (*)(unsigned char))(LAST_BOOT_ENTRY);


//_____ D E C L A R A T I O N S ____________________________________________


//! This function checks the presence of bootloader
//!
//! @return FALSE, if no code loaded in bootloader area
//!
Bool flash_lib_check( void )
{
   return (*((code U16*)((U32)boot_flash_page_erase_and_write*2)) != 0xFFFF);
}


//! This function allows to write a byte in the flash memory.
//!
//! @param addr_byte   Address in flash memory to write the byte.
//! @param value    Value to write in the flash memory
//!
void flash_wr_byte(Uint32 addr_byte, Uchar value)
{
   Enable_flash();
   flash_wr_block(&value, addr_byte, 1);
   Disable_flash();
}


//! This function allows to write up to 65535 bytes in the flash memory.
//! This function manages alignement issue.
//!
//! @param src    Address of data to write.
//! @param dst    Start address in flash memory where write data
//! @param n      number of byte to write
//!
Uchar flash_wr_block(Byte _MemType_* src, Uint32 dst, U16 n)
{
   U16 nb_word, temp16;
   U32 address;
   U32 save_page_adr;
   U8 page_is_blank;

   while(n)                                     // While there is data to load from src buffer
   {
      page_is_blank=TRUE;
      address=dst-(LOW(dst)%FLASH_PAGE_SIZE);   // Compute the start of the page to be modified
      save_page_adr=address;                    // Memorize page addr

      // For each word in this page
      for(nb_word=0;nb_word<FLASH_PAGE_SIZE/2;nb_word++)
      {
         if(n)                                  // Still some data to load from src
         {
            if(address>=dst)                    // Current address is inside the target range adr
            {
               MSB(temp16)=(*(U8*)src);         // Load MSB of word from buffer src
               src++; n--;
               if(n)                            // Still some data to load ?
               {
                  LSB(temp16)=(*(U8*)src);      // Load LSB of word from buffer src
                  src++; n--;
               }
               else                             // Only the MSB of the working belong to src buffer
               {                                // Load LSB form exisying flash
                  LSB(temp16)=flash_rd_byte((U8 farcode*)address+1);
               }
            }
            else                                // Current word addr out of dst target
            {                                   // Load MSB from existing flash
               MSB(temp16)=flash_rd_byte((U8 farcode*)address);
               if(address+1==dst)               // Is LSB word addr in dst range ?
               {
                  LSB(temp16)=(*(U8*)src);
                  src++; n--;
               }
               else                             // LSB read from existing flash
               {
                  LSB(temp16)=flash_rd_byte((U8 farcode*)address+1);
               }
            }
         }
         else                                   // Complete page with words from existing flash
         {
            temp16=flash_rd_word((U16 farcode*)address);
         }
         //Load temp buffer
         (*boot_flash_fill_temp_buffer)(temp16,address);
         address+=2;
      }
      address=save_page_adr;
      for(nb_word=0;nb_word<FLASH_PAGE_SIZE/2;nb_word++)
      {
         if(flash_rd_word((U16 farcode*)address)!=0xFFFF)   // Check for Blank page
         {
            page_is_blank=FALSE;
            break;
         }
         address+=2;
      }
      // Now launch prog sequence (with or without page erase)
      address=save_page_adr;
      if(page_is_blank)  { (*boot_flash_prg_page)(save_page_adr); }
      else{(*boot_flash_page_erase_and_write)(save_page_adr);}
      //- First Flash address update for the next page
      address = save_page_adr + FLASH_PAGE_SIZE;
   }
   return TRUE;
}


//! This function allows to read a byte in the flash memory.
//!
//! @param *add   Address of flash memory to read.
//! @return byte  Read value
//!
U8 flash_rd_byte(U8 farcode* addr)
{
   unsigned char temp;
   Enable_flash();
   temp = *addr;
   Disable_flash();
   return temp;
}


//! This function allows to read a word in the flash memory.
//!
//! @param *add   Address of flash memory to read.
//! @return word  Read value
//!
U16 flash_rd_word(U16 farcode* addr)
{
   Union16 temp;
   Enable_flash();
   temp.b[1] = flash_rd_byte ((Uchar farcode*) addr);
   temp.b[0] = flash_rd_byte ((Uchar farcode*)addr+1);
   Disable_flash();
   return temp.w;
}

