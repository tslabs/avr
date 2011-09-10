/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief Main for USB application.
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************
//!
//! @mainpage EVK525 AT90USBxxx USB Mass storage
//!
//! @section intro License
//! Use of this program is subject to Atmel's End User License Agreement.
//!
//! Please read file \ref lic_page for copyright notice.
//!
//! @section install Description
//! This embedded application source code illustrates how to implement a USB mass storage application
//! with the AT90USBxxx controller on STK525 development board, using the add-on board EVK525.
//! Please read the EVK525 User Hardware Guide before connecting the EVK525 to STK525, and before using this program.
//! The application enumerates as a Mass Storage device containing 3 Logical Units (LUN) :
//!  - SD/MMC interface, size depends on user's card  (EVK525)
//!  - NAND Flash device, 256 MBytes by default, but user can add others devices (EVK525).
//!    The NandFlash suppoted listing is availabled <A HREF="../../at90usb128/lib_mem/nf/support_NF_Driver_AVR.pdf">here</A>.
//!  - DataFlash device, 4 MBytes  (STK525)
//! 
//! @section Revision Revision
//!
//! @par V2.0.4
//!
//! USB Device:
//! - Fix bug about back drive voltage on D+ after VBus disconnect (USB Certification)
//! - Remove VBus interrupt and manage VBus state by pooling
//! - Fix bug during attach (the interrupt must be disable during attach to don't freeze clock)
//! - MassStorageClass :
//!   - Fix error in sense command to support Vista and Windows Seven
//!   - Add write Protect Management under MAC OS
//!   - Improve MSC compliance
//!   - Add MSC compliance with Linux 2.4 kernel
//!
//! SD/MMC driver:
//! - Fix SD/MMC driver initialization
//!
//! NandFlash driver:
//! - Add support NAND512W3A2C
//!
//! @par V2.0.3
//!
//! USB Stack
//! - Improve USB protocol to have a USB chap9 and MSC compliance
//!
//! Fix error on DataFlash driver
//!
//! Fix error on MMC/SD driver
//!
//! @par V2.0.2
//!
//! NandFlash driver :
//! - add feature 2xCS
//! - new CLE/ALE pin management to fix error on specific NF references
//!
//! USB Stack:
//! - Move VBus interrupt to VBus pooling
//! - Fix USB constant about Device Status
//! - Add three possibilities for SN on USB : No SN, Constant SN, Unique SN
//!
//! @par V2.0.1
//!
//! NandFlash driver :
//! - fix major bug
//! - Add listing NandFlash support
//!
//! USB Stack :
//! - Fix inquiry command
//! - add global define constante
//! - clean and fix specific request
//! - Fix endpoint array size
//! - Optimized power consumption issue in device selpowered mode:
//! -freeze clock when ubs not connected 
//! Improvement comments
//!
//! @par V2.0.0 and before
//!
//! Don't used this package, because include a major bug on NandFlash driver.
//! (e.g: old package "at90usb128-evk525-demo-storage-df-nf-sdmmc-1_0_0")
//!
//! @section arch Architecture
//! As illustrated in the figure bellow, the application entry point is located is the main.c file.
//! The main function first performs the initialization of a scheduler module and then runs it in an infinite loop.
//! The scheduler is a simple infinite loop calling all its tasks defined in the conf_scheduler.h file.
//! No real time schedule is performed, when a task ends, the scheduler calls the next task defined in
//! the configuration file (conf_scheduler.h).
//!
//! The application is based on two different tasks:
//! - The usb_task  (usb_task.c associated source file), is the task performing the USB low level
//! enumeration process in device mode or host mode.
//! - The storage task performs SCSI bulk only protocol decoding and performs flash memory access
//! for device mass storage operation calling hardware specific drivers (SD/MMC, NAND Flash, DataFlash).
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

#include "config.h"
#include "conf_access.h"
#include "modules/scheduler/scheduler.h"
#include "lib_mcu/wdt/wdt_drv.h"
#include "lib_mcu/power/power_drv.h"
#include "lib_mcu/uart/uart_lib.h"
#include "lib_mcu/debug.h"
#if (LUN_1 == ENABLE)
#include "lib_mem/nf/nf_mngt.h"
#endif

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________
       
int main(void)
{ 
   
   wdtdrv_disable();
   Clear_prescaler();

   uart_init();
#ifdef __GNUC__
   fdevopen((int (*)(char, FILE*))(uart_putchar),(int (*)(FILE*))uart_getchar); //for printf redirection used in TRACE
#endif
   trace("\x0C---  Start ---\n\r");
   
   // STK525 init
//   trace("Inits\n\r");
   Leds_init();
   Joy_init();
   Hwb_button_init();

   // Mass Storage Extension board init
   Avr_ms_board_init();


   // NAND Flash Initialization

  // Legacy
#if (LUN_1 == ENABLE && NF_RAW == FALSE)
	trace("Number of chips: ");    trace_u8(NF_N_DEVICES);    trace_nl();
	
	#if (NF_AUTO_DETECT_2KB == FALSE) && (NF_AUTO_DETECT_512B == FALSE)
    U8 nb_device;
		trace("NF check type\n\r");
		nb_device = nfc_check_type(NF_N_DEVICES);
		while( NF_N_DEVICES != nb_device );
	#else
		trace("NF detect\n\r");
	nfc_detect();
	#endif
	nf_init();

	//trace("NF test unit ready\n\r");
	nf_test_unit_ready();
#endif


  // New
#if (LUN_1 == ENABLE && NF_RAW == TRUE)

	// trace("NF_RAW Driver ON\n\r");

	// while(1)
	// {
		// trace_hex(PINC);
	// }
	
	 // while(1)
	// {
		trace_hex32(nf_raw_read_id()); trace_nl();
	// }
	 // while(1);



#endif

   scheduler();
   return 0;
}

//! \name Procedure to speed up the startup code
//! This one increment the CPU clock before RAM initialisation
//! @{
#ifdef  __GNUC__
// Locate low level init function before RAM init (init3 section)
// and remove std prologue/epilogue
char __low_level_init(void) __attribute__ ((section (".init3"),naked));
#endif

#ifdef __cplusplus
extern "C" {
#endif
char __low_level_init()
{
  // Clear_prescaler();
  return 1;
}
#ifdef __cplusplus
}
#endif
//! @}
