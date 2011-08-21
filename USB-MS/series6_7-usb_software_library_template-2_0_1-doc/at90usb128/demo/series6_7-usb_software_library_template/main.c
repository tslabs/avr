/*This file has been prepared for Doxygen automatic documentation generation.*/
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
//! @mainpage AT90USBxxx USB software library for dual role devices
//!
//! @section intro License
//! Use of this program is subject to Atmel's End User License Agreement.
//!
//! Please read file  \ref lic_page for copyright notice.
//!
//! @section install Description
//! This embedded application source code illustrates how to implement a USB interfaced application
//! over the AT90USBxxx controller.
//!
//! As the AT90USBxxx implements  a device/host usb controller, the embedded application can operates
//! in one of the following usb operating modes:
//! - USB device
//! - USB reduced host controller
//! - USB dual role device (depending on the ID pin)
//!
//! To optimize embedded code/ram size and reduce the number of source modules, the application can be
//! configured to use one and only one of these operating modes.
//!
//! This sample application can be configured for both STK525 or AT90USBKey hardware, see #TARGET_BOARD
//! define value in config.h file.
//!
//! @section src_code About the source code
//! This source code is usable with the following compilers:
//! - IAR Embedded Workbench (5.11A and higher)
//! - AVRGCC (WinAVR 20080411 and higher).
//!
//! Support for other compilers may required modifications or attention for:
//! - compiler.h file 
//! - special registers declaration file
//! - interrupt subroutines declarations
//!
//! @section Revision Revision
//!
//! @par V2.0.1
//!
//! USB Stack
//! - Add three possibilities for SN on USB
//! - Move VBus interrupt to VBus pooling
//! - Fix USB constant about Device Status
//! - Improve USB protocol to have a USB chap9 compliance
//!
//! New watchdog driver
//!
//! @par V2.0.0 and before
//! Old versions
//!
//! @section sample About the sample application
//! By default the sample code is delivered with a simple preconfigured dual role USB application.
//! It means that the code generated allows to operate as a device or a host depending on the USB ID pin:
//! - Attached to a mini B plug (ID pin unconnected) the application will be used in the device operating mode
//! - Attached to a mini A plug (ID pin tied to ground) the application will operate in reduced host mode
//!
//! \image html appli.gif
//! Thus two instances of this application can be connected together with a miniA-miniB cable.
//! - The host operating mode of the application (that we call A device) can manage the connection
//! and the enumeration of a device application connected in device operating mode (that we call B device).
//! Once the device has been enumerated, the host high level application can operate USB applicative exchanges
//! with the B device. Here the sample host application writes 64 byte through a USB OUT pipe and read back
//! these data with an IN pipe.
//! - The device operating mode of the application (B device) answers to the enumeration requests (setup requests)
//! of the host controller. Once it has been properly enumerated the high level device application receives 64 data
//! through its OUT enpoint and when the host controller request for an IN exchange on its IN endpoints,
//! sends it back (loopback application).
//!
//! @section arch Architecture
//! As illustrated in the figure bellow, the application entry point is located is the main.c file.
//! The main function first performs the initialization of a scheduler module and then runs it in an infinite loop.
//! The scheduler is a simple infinite loop calling all its tasks defined in the conf_scheduler.h file.
//! No real time schedule is performed, when a task ends, the scheduler calls the next task defined in
//! the configuration file (conf_scheduler.h).
//!
//! The sample dual role application is based on three different tasks:
//! - The usb_task  (usb_task.c associated source file), is the task performing the USB low level
//! enumeration process in device or host mode.
//! Once this task has detected that the usb connection is fully operationnal, it updates different status flags
//! that can be check within the high level application tasks.
//! - The device template task (device_template_task.c associated source file) performs the high level device
//! application operation. Once the device is fully enumerated (DEVICE SETUP_SET_CONFIGURATION request received), the task
//! checks for received data on its OUT endpoint and transmit these data on its IN endpoint.
//! - The host template tak (host_template_task.c associated file) performs the high level host application operation.
//! Ih the "B device" is correctly connected and enumerated, the task sends and receives data with the USB bus.
//!
//! \image html arch_full.gif
//!
//! @section config Configuration
//! The sample application is configured to implements both host and device functionnalities.
//! Of course it can also be configured to be used in device or reduced host only mode (See conf_usb.h file).
//! Depending on the USB operating mode selected, the USB task will call either the usb_host_task (usb_host_task.c),
//!  either the usb device
//! task (usb_device_task.c) to manage chapter 9 requests. In such case, the corresponding template_device_task
//! or template_host_task can be removed from the scheduled tasks (see conf_scheduler.h).
//! @note
//! The B device descriptors used for this sample application are not directly usable for enumeration
//! with a standard Pc host system. Please refers to <A HREF="http://www.atmel.com">ATMEL website
//! </A> for real devices applications  examples (HID mouse, HID keyboard, MassStorage, CDC ... )
//!
//! @todo Implements the required mechanism for On the Go (OTG) compliance:
//! SRP, HNP and role exchange detection/requests.
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

#include "config.h"
#include "modules/scheduler/scheduler.h"
#include "lib_mcu/wdt/wdt_drv.h"
#include "lib_mcu/power/power_drv.h"

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

int main(void)
{
   wdtdrv_disable();
   Clear_prescaler();
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
  Clear_prescaler();
  return 1;
}
#ifdef __cplusplus
}
#endif
//! @}

