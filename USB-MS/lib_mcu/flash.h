/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the product defines about flash and bootloader API
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

#ifndef FLASH_H
#define FLASH_H


//_____ D E F I N I T I O N S ______________________________________________

#ifdef __ICCAVR__
   #if (defined(__AT90USB1287__) || defined(__AT90USB1286__)) 
      #define  FLASH_END         0x1FFFF  // Size of flash
      #define  FLASH_PAGE_SIZE   0x100    // Size of page flash
      #define  BOOT_SIZE         0x2000   // Size of bootloader
   #elif (defined(__AT90USB647__) || defined(__AT90USB646__))
      #define  FLASH_END         0x0FFFF  // Size of flash
      #define  FLASH_PAGE_SIZE   0x100    // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__ATmega32U4__) || defined(__ATmega32U6__)) 
      #define  FLASH_END         0x7FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif defined(__ATmega16U4__)
      #define  FLASH_END         0x3FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__AT90USB162__) ) 
      #define  FLASH_END         0x3FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__AT90USB82__) )
      #define  FLASH_END         0x1FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #else
      #error TARGET should be defined 
   #endif
#elif defined __GNUC__
   #if (defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB1286__)) 
      #define  FLASH_END         0x1FFFF  // Size of flash
      #define  FLASH_PAGE_SIZE   0x100    // Size of page flash
      #define  BOOT_SIZE         0x2000   // Size of bootloader
   #elif (defined(__AVR_AT90USB647__) || defined(__AVR_AT90USB646__))
      #define  FLASH_END         0x0FFFF  // Size of flash
      #define  FLASH_PAGE_SIZE   0x100    // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__AVR_AT90USB647__) || defined(__AVR_AT90USB646__))
      #define  FLASH_END         0x0FFFF  // Size of flash
      #define  FLASH_PAGE_SIZE   0x100    // Size of page flash
   #elif (defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega32U6__)) 
      #define  FLASH_END         0x7FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif defined(__AVR_ATmega16U4__)
      #define  FLASH_END         0x3FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__AVR_AT90USB162__) ) 
      #define  FLASH_END         0x3FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #elif (defined(__AVR_AT90USB82__) )
      #define  FLASH_END         0x1FFF   // Size of flash
      #define  FLASH_PAGE_SIZE   0x80     // Size of page flash
      #define  BOOT_SIZE         0x1000   // Size of bootloader
   #else
      #error TARGET should be defined 
   #endif
#else // Other compiler
   #error Compiler unknow
#endif

#define  LAST_BOOT_ENTRY   (((FLASH_END+1)/2)-2)   // Last boot entry is the last Word of flash (unit Word 16-bit)     
#define  FLASH_SIZE        FLASH_END-BOOT_SIZE


#endif  // FLASH_H 














