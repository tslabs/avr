/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the function declarations for usb host mass storage  task application
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

#include "config.h"
#include "conf_usb.h"

#ifndef HOST_SYNC_MODE
#  warning HOST_SYNC_MODE not defined as ENABLE or DISABLE, using DISABLE...
#  define HOST_SYNC_MODE  DISABLE
#endif

#ifndef HOST_UPGRADE_MODE
#  warning HOST_UPGRADE_MODE not defined as ENABLE or DISABLE, using DISABLE...
#  define HOST_UPGRADE_MODE  DISABLE
#endif

// File system includes required for SYNC mode
#if (HOST_SYNC_MODE==ENABLE)
   #include "fat.h"
   #include "fs_com.h"
   #include "navigation.h"
   #include "file.h"
   #include "nav_utils.h"
#endif

#if (HOST_UPGRADE_MODE==ENABLE)
   #include "firm_upgrade.h"
#endif

#include "host_ms_task.h"
#include "host_mem.h"
#include "usb_host_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E C L A R A T I O N S ____________________________________________

//! Host start of frame counter incremented under SOF interrupt
volatile U8 host_cpt_sof;

#if (HOST_SYNC_MODE==ENABLE)
   //! directory name for USB out synchro
   U8 code dir_usb_out_name[]=DIR_USB_OUT_NAME;
   //! directory name for USB in synchro
   U8 code dir_usb_in_name[]=DIR_USB_IN_NAME;
   //! directory name for LOCAL out synchro
   U8 code dir_local_out_name[]=DIR_LOCAL_OUT_NAME;
   //! directory name for LOCAL in synchro
   U8 code dir_local_in_name[]=DIR_LOCAL_IN_NAME;
   //! Flag set when sync operation is on-going
   U8 sync_on_going=0;
   //! Intermediate ram unicode file name buffer for sync operation
   U8 ms_str_ram[MAX_FILE_LENGHT];
#endif

#if (HOST_UPGRADE_MODE==ENABLE)
   Bool b_new_msc_connected = FALSE;
#endif

//! @brief This function initializes the Host Mass Storage application
//!
void host_ms_task_init(void)
{
   Leds_init();
#if (HOST_SYNC_MODE==ENABLE)
   Joy_init();
#endif
   host_mem_init();
}


//! @brief This function manages the HOST mass storage application
//!
void host_ms_task(void)
{
   if( Is_host_ready() )   
   {
      // Here, Enumeration successfull, device is operationnal
      if(Is_new_device_connection_event())
      {
         // Update MS driver in case of
         if( host_mem_install() )
         {
#if (HOST_UPGRADE_MODE==ENABLE)
            b_new_msc_connected = TRUE;
#endif
            Led1_on();
         }
      }

#if (HOST_SYNC_MODE==ENABLE)  // Sync operating mode(if available)
      if( 0 != host_mem_get_lun() )
      {
         if(Is_joy_right())   // Sync device to host stream
         {
            Led0_on();
            sync_on_going=1;
            copy_dir( (U8 code *)dir_usb_out_name, (U8 code *)dir_local_in_name, 1 );
            sync_on_going=0;
            Led3_off();
            Led0_off();
         }
         if(Is_joy_left())    // Sync host to device stream
         {
            Led0_on();
            sync_on_going=1;
            copy_dir( (U8 code *)dir_local_out_name, (U8 code *)dir_usb_in_name, 1 );
            sync_on_going=0;
            Led0_off();
            Led3_off();
         }
      }
#endif

#if (HOST_UPGRADE_MODE==ENABLE)
      if( b_new_msc_connected )
      {
         // A new MSC is connected then start upgrade routine
         b_new_msc_connected = FALSE;
         firm_upgrade_run();
      }
#endif
   }

   // Device disconnection...
   if( Is_device_disconnection_event() )
   {
      // Update MS driver in case of
      host_mem_uninstall();
      Led1_off();
   }
}


//! @brief This function is called each host start of frame, when sync operation
//! is on-going, the function toggles Led3 each 255ms.
//!
void host_sof_action(void)
{
   host_cpt_sof++;
   #if (HOST_SYNC_MODE==ENABLE)
   if(host_cpt_sof==0 &&sync_on_going) Led3_toggle();
   #endif
}

