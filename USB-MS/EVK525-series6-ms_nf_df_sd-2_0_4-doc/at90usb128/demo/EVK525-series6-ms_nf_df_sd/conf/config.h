/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the system configuration definition.
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

// Compiler switch (do not change these settings)
#include "lib_mcu/compiler.h"             // Compiler definitions
#ifdef __GNUC__
   #include <avr/io.h>                    // Use AVR-GCC library
#elif __ICCAVR__
   #define ENABLE_BIT_DEFINITIONS
   #include <ioavr.h>                     // Use IAR-AVR library
#else
   #error Current COMPILER not supported
#endif


// Debug options
#define  _ASSERT_       (DISABLE)
#define  _TRACE_        (ENABLE)
 
//! @defgroup global_config Application configuration
//! @{

#include "conf/conf_scheduler.h" //!< Scheduler tasks declaration

// Board defines (do not change these settings)
#define  STK525   1
#define  USBKEY   2

//! Enable or not the ADC usage
#define USE_ADC
//! To include proper target hardware definitions, select
//! target board (USBKEY or STK525)
#define TARGET_BOARD STK525

// Data flash on card + on board df configuration
//#define   SPIDER_HAS_8MB_DF_CARD

// Data flash on card only configuration
//#define DF_ON_CARD_ONLY
#ifdef  DF_ON_CARD_ONLY
   #define DF_4_MB
#endif

#if (TARGET_BOARD==USBKEY)
   //! @warning for #define USBKEY_HAS_321_DF, only first prototypes versions have AT45DB321C memories
   //! should be undefined for std series
   //#define USBKEY_HAS_321_DF
   #include "lib_board\usb_key\usb_key.h"
#elif (TARGET_BOARD==STK525)
   #include "lib_board\stk_525\stk_525.h"
#else
   #error TARGET_BOARD must be defined somewhere
#endif

// Mass Storage Extension Board
#include "lib_board/avr_ms_board/avr_ms_board_drv.h"

//! CPU core frequency in kHz
#define FOSC 16000


// -------- END Generic Configuration -------------------------------------

#define HOST_SYNC_MODE  ENABLE

// UART Sample configuration, if we have one ... __________________________
#define BAUDRATE        57600
#define USE_UART2
#define UART_U2

#ifndef __GNUC__
#define uart_putchar putchar
#endif

#define r_uart_ptchar int
#define p_uart_ptchar int


// ADC Sample configuration, if we have one ... ___________________________


//! ADC Prescaler value
#define ADC_PRESCALER 32
//! Right adjust
#define ADC_RIGHT_ADJUST_RESULT 1
//! AVCC As reference voltage (See adc_drv.h)
#define ADC_INTERNAL_VREF  2

#define SBC_VENDOR_ID         {'A','T','M','E','L',' ',' ',' '}      // 8 Bytes only
#define SBC_PRODUCT_ID        {'A','T','9','0','U','S','B','1','2','8','7',' ','M','S',' ',' '}  // 16 Bytes only
#define SBC_REVISION_ID       {'1','.','0','0'}  // 4 Bytes only

// MMC/SD configuration ____________________________________________
// see conf_sdmmc.h

// Nand Flash configuration ____________________________________________
// see conf_nf.h

//! @}

#endif // _CONFIG_H_

