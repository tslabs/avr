/*This file has been prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the low level macros and definition for the EVK525 (AVR Mass Storage extension board)
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


// =======
// GENERAL
// =======
#define   Avr_ms_board_init()       (Led_ms_init(), MMC_io_init(), Nandflash_init())

// ====
// LEDs
// ====
#define   Led_ms_init()             (DDRD |= 0xA0, PORTD &= ~0xA0)
#define   Led_ms_write_on()         (PORTD |= 0x80)
#define   Led_ms_write_off()        (PORTD &= ~0x80)
#define   Led_ms_read_on()          (PORTD |= 0x20)
#define   Led_ms_read_off()         (PORTD &= ~0x20)


// =============
// SD/MMC memory
// =============
#define   MMC_io_init()             (DDRB &= 0xF7, DDRB |= 0x07, PORTB |= 0x09)
#ifndef   __GNUC__
  #define   MMC_CS_LINE             PORTB_Bit0
#endif

#define   MMC_CS_PORT               PORTB   // port
#define   MMC_CS_PIN                0x00    // offset
#define   Mmc_sd_select()           (MMC_CS_PORT &= ~(1<<MMC_CS_PIN))
#define   Mmc_sd_unselect()         (MMC_CS_PORT |= (1<<MMC_CS_PIN))


// ==================
// NAND Flash devices
// ==================

// Default chip soldered
#define   Nandflash_init()          (DDRC &= ~0xC0, DDRC |= 0x3F, PORTC |= 0x7C, PORTC &= ~0x03, \
                                     DDRB |= ~0x01, PORTB |= 0x01, DDRE |= 0x03, PORTE &= ~0x03, \
                                     PORTA = 0x00, DDRA |= 0xFF)
#define   Nandflash_CLE_select()     (PORTC |=  0x01)    // CLE
#define   Nandflash_CLE_unselect()   (PORTC &= ~0x01)
#define   Nandflash_ALE_select()     (PORTC |=  0x02)    // ALE
#define   Nandflash_ALE_unselect()   (PORTC &= ~0x02)
#define   Nandflash0_select()        (PORTC &= ~0x04)    // CS.0
#define   Nandflash0_unselect()      (PORTC |=  0x04)
#define   Nandflash1_select()        (PORTC &= ~0x08)    // CS.1
#define   Nandflash1_unselect()      (PORTC |=  0x08)
#define   Nandflash_powerdown()     Nandflash_unselect()
#define   Is_nandflash_ready()      (((PORTC&0x40) != 0) ? TRUE : FALSE)
#define   Is_nandflash_busy()       (((PORTC&0x40) == 0) ? TRUE : FALSE)

#define   Nandflash_wp_enable()     
#define   Nandflash_wp_disable()    

// Optional module
      // TBD (need update NF driver to support several memories)
      // don't forget R/B# internal pull-up

// CS.1 : PC3
// CS.2 : PC4
// CS.3 : PC5













