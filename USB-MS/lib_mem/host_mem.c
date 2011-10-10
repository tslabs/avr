/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief  This file contains the interface routines of virtual memory.
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
#include <string.h>
#include "conf_usb.h"
#include "host_mem.h"
#include "conf_access.h"
#include "usb_drv.h"
#include "usb_host_enum.h"
#include "usb_host_task.h"
#include "scsi_decoder.h"

//_____ D E F I N I T I O N ________________________________________________

#ifndef LOG_STR_CODE
#define LOG_STR_CODE(str)
#else
U8 code log_ms_connect[]="Mass Storage Connected";
#endif
#ifndef HOST_SECTOR_SIZE
#define HOST_SECTOR_SIZE   512   //default sector size is 512 bytes
#endif

#if (USB_HUB_SUPPORT==ENABLE)
   #ifndef USB_MAX_DMS_NUMBER
      #define USB_MAX_DMS_NUMBER    MAX_DEVICE_IN_USB_TREE-1
   #endif
#else
   #define USB_MAX_DMS_NUMBER 1         
#endif

#ifndef USB_SUPPORT_MS_SECTOR_WR_MAX
#define USB_SUPPORT_MS_SECTOR_WR_MAX 1
#endif

#define USB_MAX_LUN_PER_DMS   4

// Structure data of CBW
typedef struct
{
   U8 flag1;      //  0 - 'U'
   U8 flag2;      //  1 - 'S'
   U8 flag3;      //  2 - 'B'
   U8 flag4;      //  3 - 'C'
   U8 tag_lsb0;   //  4 - LSB0( CBW Tag )
   U8 tag_lsb1;   //  5 - LSB1( CBW Tag )
   U8 tag_lsb2;   //  6 - LSB2( CBW Tag )
   U8 tag_lsb3;   //  7 - LSB3( CBW Tag )
   U8 lgt_lsb0;   //  8 - LSB0( CBW Data Transfer Length )
   U8 lgt_lsb1;   //  9 - LSB1( CBW Data Transfer Length )
   U8 lgt_lsb2;   // 10 - LSB2( CBW Data Transfer Length )
   U8 lgt_lsb3;   // 11 - LSB3( CBW Data Transfer Length )
   U8 dir;        // 12 - CBW Flags
   U8 lun;        // 13 - CBW LUN
   U8 lgt;        // 14 - CBW length
   U8 cmd;        // 15 - CBW Operation Code (= command)
   U8 u8_data[15];// 16 - CBW Datas [15]
} S_cbw;

// Define the position of data in CBW struct
#define  HOST_MS_CBW_DATA_POS    16

// Information struct for each LUN
typedef struct
{
   Bool b_first_unitready; // to send a BUSY/CHANGE status at first Unitready, bit Field corresponding at each LUN
   Bool b_protected;       // bit Field corresponding at each LUN
   U8 u8_block_size;       // unit 512B
} S_lun_info;

// Information struct for each MS device
typedef struct
{
   U8 device_index;
   U8 pipe_in;
   U8 pipe_out;
   U8 nb_lun;
   S_lun_info lun_info[USB_MAX_LUN_PER_DMS];   
} S_dms_device;


//_____ D E C L A R A T I O N ______________________________________________


S_cbw          g_ms_cbw;                           // Global CBW struct to optimize code
S_dms_device   g_ms_devices[USB_MAX_DMS_NUMBER];   // Information about each MS devices connected
U8    g_ms_nb_connected;                           // Number of MS device connected
U8    g_ms_sel_dms=0;                              // Index of current MS device
U8    g_ms_sel_lun;                                // Index of current LUN of current MS device

#if (USB_SUPPORT_MS_SECTOR_WR_MAX != 1)
U8    g_ms_cache[(USB_SUPPORT_MS_SECTOR_WR_MAX-1)*512];
#endif


// Variables to save a read context
// (to optimize the MS device with a sector size != 512B)
U8    g_readctx_b_run = FALSE;
U8    g_readctx_u8_device;
U8    g_readctx_u8_lun;
U32   g_readctx_u32_addr;
U16   g_readctx_u16_sector;


// Macro to control MS device state
#define  Is_host_ms_configured()    ((g_ms_nb_connected && Is_host_not_suspended() && Is_usb_id_host()) ? TRUE : FALSE )

// Current pipe corresponding at MS device selected
#define  DMS_pipe_in()              (g_ms_devices[g_ms_sel_dms].pipe_in)
#define  DMS_pipe_out()             (g_ms_devices[g_ms_sel_dms].pipe_out)


// Functions medium level
Ctrl_status host_mem_request_sense     ( void );

// Functions low level
Bool        host_mem_select_lun        ( U8 lun );
void        host_mem_cbw_init          ( void );
Bool        host_mem_cbw_send          ( void );
Ctrl_status host_mem_csw_read          ( void );
void        host_mem_stall_management  ( void );


//-----------------------------------------------------------------------
//------------ Functions to manage the list of MS devices ---------------


//! This fonction must be call only once at the program startup to initialize the USB driver MS
//!
void host_mem_init( void )
{
   U8 i;
   // Reset Device MS struct
   for(i=0; i<USB_MAX_DMS_NUMBER; i++)
   {
      g_ms_devices[i].device_index=0xFF;
   }
   g_ms_nb_connected=0;
}


//! This fonction must be call when a device is connected
//!
Bool host_mem_install( void )
{
   U32 capacity;
   U8  device_num;
   U8  interface_num;
   U8  lun_offset;
   U8  i;
   
   // Check all devices connected
   for( device_num=0; device_num<Get_nb_device(); device_num++)
   {
      Host_select_device(device_num);
      
      // Check all interfaces of device
      for( interface_num=0; interface_num < Get_nb_supported_interface(); interface_num++ )
      {
         if( Get_class(interface_num)!= MS_CLASS )
            continue;   // It is not a MS interface
         
         LOG_STR_CODE(log_ms_connect);

         // If it is not the first MS device
         if( g_ms_nb_connected != 0 )
         {
            // Check if already installed
            for( i=0; i<g_ms_nb_connected; i++ ) 
            {
               if( g_ms_devices[i].device_index == device_num ) 
                  break;
            }
            if( i!=g_ms_nb_connected )
               break;      // Device already installed
            // Check struct space free               
            if( USB_MAX_DMS_NUMBER == g_ms_nb_connected )
               break;      // Too many MS device connected
         }            
         
         //** Start install
         g_ms_devices[g_ms_nb_connected].device_index = device_num;                     
         
         // Get correct physical pipes associated to IN/OUT Mass Storage Endpoints
         if(Is_ep_addr_in(Get_ep_addr(interface_num,0)))
         {  //Yes associate it to the MassStorage IN pipe
            g_ms_devices[g_ms_nb_connected].pipe_in=usb_tree.device[device_num].interface[interface_num].ep[0].pipe_number;
            g_ms_devices[g_ms_nb_connected].pipe_out=usb_tree.device[device_num].interface[interface_num].ep[1].pipe_number;
         }
         else
         {  //No, invert...
            g_ms_devices[g_ms_nb_connected].pipe_in=usb_tree.device[device_num].interface[interface_num].ep[1].pipe_number;
            g_ms_devices[g_ms_nb_connected].pipe_out=usb_tree.device[device_num].interface[interface_num].ep[0].pipe_number;
         }
         
         // Get last LUN number on the device mass storage connected                     
         if( CONTROL_GOOD != host_ms_get_max_lun() )
           data_stage[0] = 0; // If STALL request then last LUN number = 0
         g_ms_devices[g_ms_nb_connected].nb_lun = data_stage[0]+1;   // Save number of LUN = last LUN number + 1
         
         if( USB_MAX_LUN_PER_DMS < g_ms_devices[g_ms_nb_connected].nb_lun )
            g_ms_devices[g_ms_nb_connected].nb_lun = USB_MAX_LUN_PER_DMS;   // Limitation on the number of LUN
         
         // Compute LUN offset of this new device
         lun_offset = 0;
         for( i=0; i<g_ms_nb_connected; i++) 
         {
            lun_offset +=g_ms_devices[i].nb_lun;
         }
         
         // Add the new MS device in MS strut
         g_ms_sel_dms = g_ms_nb_connected;
         g_ms_nb_connected++;
         
         // Initialize all LUN from the new MS device
         for( i=0; i<g_ms_devices[g_ms_sel_dms].nb_lun; i++ )
         {
            // Init struct of LUN
            g_ms_devices[g_ms_sel_dms].lun_info[i].b_first_unitready = FALSE;
            g_ms_devices[g_ms_sel_dms].lun_info[i].b_protected       = TRUE;
            // Init MS corresponding at LUN
            host_mem_inquiry( lun_offset+i );
            host_mem_read_capacity( lun_offset+i, &capacity );
         }
         return TRUE;   // Install interface/device done
      }
   }
   return FALSE;   // No MSC install
}


//! This fonction must be call when a device is disconnected
//!
void host_mem_uninstall( void )
{
   U8  device_num;
   U8  i;
   Bool b_last_ms_conected = TRUE;

   for( i=g_ms_nb_connected; i!=0; i--)
   {
      device_num = g_ms_devices[i-1].device_index;
      if( 0xFF == device_num )
      {
         // Device already remove but struct no updated
         // it may be possible in case of many MS on USB HUB
         g_ms_nb_connected--;
         continue;
      }
      if( 0 == usb_tree.device[device_num].device_address )
      {
         g_ms_devices[i-1].device_index = 0xFF;
         if( b_last_ms_conected )
            g_ms_nb_connected--;
      }else{
         b_last_ms_conected = FALSE;
      }
   }
}

            
//! This fonction returns the number of LUN of the devices mass storage connected to the host
//!
//! @return   The number of LUN
//!
U8 host_mem_get_lun( void )
{
   U8 i;
   U8 host_ms_max_lun = 0;

   if(!Is_host_ms_configured())
      return 0;

   for( i=0; i<g_ms_nb_connected; i++ )
   {
      host_ms_max_lun += g_ms_devices[i].nb_lun;
   }
   return host_ms_max_lun;
}


//-----------------------------------------------------------------------
//------------ Functions to control the state of each LUN ---------------


//! This fonction test the state of memory, and start the initialisation of the memory
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_test_unit_ready(U8 lun)
{
   Ctrl_status status;
   U32   u32_nb_sector;

   if( !host_mem_select_lun(lun) )
      return CTRL_NO_PRESENT;
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }

   // Send CBW
   host_mem_cbw_init();
   // g_ms_cbw.lgt_lsb0 = 0;
   g_ms_cbw.dir               = SBC_CMD_DIR_OUT;
   g_ms_cbw.lgt               = 0x06;
   g_ms_cbw.cmd               = SBC_CMD_TEST_UNIT_READY;
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Read CSW status
   status = host_mem_csw_read();
   if( status == CTRL_GOOD )
   {
   	if( FALSE == g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].b_first_unitready )
      {
         // Signal the new diskl
         status = CTRL_BUSY;
         // U-Disk valid then update info LUN
         if( CTRL_GOOD == host_mem_read_capacity( lun , &u32_nb_sector ) )
         {
            g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].b_protected       = host_mem_wr_protect( lun );
            g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].b_first_unitready = TRUE;
         }
      }
   }
   else
   {
      g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].b_first_unitready = FALSE;
   }
   return status;
}


//! This fonction returns the address of the last valid sector.
//!
//! @return *u16_nb_sector number of sector (sector = 512B)
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_read_capacity(U8 lun, U32 _MEM_TYPE_SLOW_ *u32_nb_sector )
{
   U16   nb;
   U8    datas[8], status;

   if( !host_mem_select_lun(lun) )
      return CTRL_NO_PRESENT;
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }

   // Send CBW
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = 0x08;
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x0A;
   g_ms_cbw.cmd               = SBC_CMD_READ_CAPACITY;
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Receiv the capacity data
   nb=8;
   status = host_get_data(DMS_pipe_in(),&nb,datas);
   if( PIPE_GOOD != status )
   {
      if( PIPE_STALL == status )
        host_mem_stall_management();
      host_mem_csw_read();
      return CTRL_FAIL;
   }

   // Get last sector address
   MSB0(*u32_nb_sector)      = datas[0];
   MSB1(*u32_nb_sector)      = datas[1];
   MSB2(*u32_nb_sector)      = datas[2];
   MSB3(*u32_nb_sector)      = datas[3];
   // Get block size (unit 512B)
   g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size = datas[6]/2; // Block size MSB

   // Read CSW status
   return host_mem_csw_read();
}


//! @brief Returns the physical sector of the memory
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @return size of physical sector of disk (unit 512B)
//!
U8    host_mem_read_sector_size( U8 lun )
{
   host_mem_test_unit_ready( lun );
   if( !host_mem_select_lun(lun) )
      return 1;
   return g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].u8_block_size;
}


//! @brief Return the write protected mode (cache used)
//! This information is in a cache to speed up command
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @return TRUE, the memory is protected
//!
Bool  host_mem_wr_protect_cache( U8 lun )
{
   if( !host_mem_select_lun(lun) )
      return TRUE;   // no present
   
   return g_ms_devices[g_ms_sel_dms].lun_info[g_ms_sel_lun].b_protected;
}


//! @brief Return the write protected mode (no cache)
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @return TRUE, the memory is protected
//!
Bool host_mem_wr_protect( U8 lun )
{
   U16 nb;
   U8 write_code[0x0C], status;
   
   if( !host_mem_select_lun(lun) )
      return TRUE;

   if( USB_SUPPORT_MS_SECTOR_WR_MAX < g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size )
      return TRUE;   // No supported this lun in write mode, because sector size != 512B

   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }

   // Send CBW
   host_mem_cbw_init();
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x06;
   g_ms_cbw.cmd               = SBC_CMD_MODE_SENSE_6;
   g_ms_cbw.lgt_lsb0          = 0x0C;
   //g_ms_cbw.u8_data[16-16]  = 0x00;                 // 16 - CBWCB1 - Option
   g_ms_cbw.u8_data[17-16]    = 0x3F;                 // 17 - CBWCB2 - Page Code 3F = return all mode pages
   //g_ms_cbw.u8_data[18-16]  = 0x00;                 // 18 - CBWCB3 - reserved
   g_ms_cbw.u8_data[19-16]    = 0x0C;                 // 19 - CBWCB4 - Allocation Length
   //g_ms_cbw.u8_data[20-16]  = 0x00;                 // 20 - CBWCB5 - Control
   if( !host_mem_cbw_send() )
      return TRUE;

   // Receiv 
   nb=0x0C;
   status = host_get_data(DMS_pipe_in(),&nb,write_code);
   if( PIPE_GOOD != status )
   {
      if( PIPE_STALL == status )
        host_mem_stall_management();
      write_code[2] = 0x00;   // error then no write protected by defaut
   }

   // Read CSW status
   host_mem_csw_read();       // ignore error on CSW (in case of specific Udisk)

   return (write_code[2] == 0x80);
}


//! This fonction inform about the memory type
//!
//! @return FASLE  -> The memory isn't removal
//!
Bool  host_mem_removal(void)
{
   return TRUE;
}

/*
//! @brief This fonction generates a read format capacity SCSI request 
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_read_format_capacity( U8 lun )
{
   U16 nb;
   U8 datas[0xFC], status;
   
   if( !host_mem_select_lun(lun) )
      return CTRL_NO_PRESENT;
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }
      
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = 0xFC;
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x0C;
   g_ms_cbw.cmd               = SBC_CMD_READ_FORMAT_CAPACITY;
   //g_ms_cbw.u8_data[16-16]  = 0x00;              // 16 - CBWCB1 - relative address
   //g_ms_cbw.u8_data[17-16]  = 0x00;              // 17 - CBWCB2 - MSB3(Logical Block Address)
   //g_ms_cbw.u8_data[18-16]  = 0x00;              // 18 - CBWCB3 - MSB2(Logical Block Address)
   //g_ms_cbw.u8_data[19-16]  = 0x00;              // 19 - CBWCB4 - MSB1(Logical Block Address)
   //g_ms_cbw.u8_data[20-16]  = 0x00;              // 20 - CBWCB5 - MSB0(Logical Block Address)
   //g_ms_cbw.u8_data[21-16]  = 0x00;              // 21 - CBWCB6 - reserved
   //g_ms_cbw.u8_data[22-16]  = 0x00;              // 22 - CBWCB7 - reserved
   g_ms_cbw.u8_data[23-16]  = 0xFC;                // 23 - CBWCB8 - PMI
   //g_ms_cbw.u8_data[24-16]  = 0x00;              // 24 - CBWCB9 - Control
   //g_ms_cbw.u8_data[25-16]  = 0x00;              // 25
   //g_ms_cbw.u8_data[26-16]  = 0x00;              // 26
   //g_ms_cbw.u8_data[27-16]  = 0x00;              // 27
   //g_ms_cbw.u8_data[28-16]  = 0x00;              // 28
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Receiv Inquiry data
   // Transfer data ...
   nb=0xFC;
   status = host_get_data(DMS_pipe_in(),&nb,datas);
   // Ignore inquiry data
   if( PIPE_STALL == status )
     host_mem_stall_management();

   // Read CSW status
   return host_mem_csw_read();
}
*/

//! @brief Read inquiry datas
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_inquiry( U8 lun )
{
   U16 nb;
   U8 datas[31], status;
   
   if( !host_mem_select_lun(lun) )
      return CTRL_NO_PRESENT;
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }
      
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = 0x24;
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x06;
   g_ms_cbw.cmd               = SBC_CMD_INQUIRY;
   //g_ms_cbw.u8_data[16-16]  = 0x00;              // 16 - CBWCB1 - Option
   //g_ms_cbw.u8_data[17-16]  = 0x00;              // 17 - CBWCB2 - Page or operation code
   //g_ms_cbw.u8_data[18-16]  = 0x00;              // 18 - CBWCB3 - reserved
   g_ms_cbw.u8_data[19-16]    = 0x24;              // 19 - CBWCB4 - Allocation Length
   //g_ms_cbw.u8_data[20-16]  = 0x00;              // 20 - CBWCB5 - Control
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Receiv Inquiry data
   // Transfer data ...
   nb=31;
   status = host_get_data(DMS_pipe_in(),&nb,datas);
   // Ignore inquiry data
   if( PIPE_STALL == status )
     host_mem_stall_management();

   // Read CSW status
   return host_mem_csw_read();
}


//! @brief Read the error informations about last command FAIL
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_request_sense( void )
{
   U16 nb;
   U8 datas[17], status;
   U8 sense_key, sense_key_add;

   // Send CBW
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = 0x12;
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x06;
   g_ms_cbw.cmd               = SBC_CMD_REQUEST_SENSE;
   //g_ms_cbw.u8_data[16-HOST_MS_CBW_DATA_POS]  = 0x00;  // 16 - CBWCB1 - reserved
   //g_ms_cbw.u8_data[17-HOST_MS_CBW_DATA_POS]  = 0x00;  // 17 - CBWCB2 - reserved
   //g_ms_cbw.u8_data[18-HOST_MS_CBW_DATA_POS]  = 0x00;  // 18 - CBWCB3 - reserved
   g_ms_cbw.u8_data[19-HOST_MS_CBW_DATA_POS]    = 0x12;  // 19 - CBWCB4 - Allocation Length
   //g_ms_cbw.u8_data[20-HOST_MS_CBW_DATA_POS]  = 0x00;  // 20 - CBWCW5 - Control
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Receiv Sense Code
   nb=17;
   status = host_get_data(DMS_pipe_in(),&nb,datas);
   if( PIPE_GOOD != status )
   {
      if( PIPE_STALL == status )
        host_mem_stall_management();
      return CTRL_FAIL;
   }

   //  0 - Response Code
   //  1 - obsolete
   //  2 - Sense Key
   sense_key = (0x0F & datas[2]);
   //  3 - MSB3(Information)
   //  4 - MSB2(Information)
   //  5 - MSB1(Information)
   //  6 - MSB0(Information)
   //  7 - Additional Sense Length
   //  8 - MSB3(Command Specific Information)
   //  9 - MSB2(Command Specific Information)
   // 10 - MSB1(Command Specific Information)
   // 11 - MSB0(Command Specific Information)
   // 12 - Additional Sense Code
   sense_key_add = datas[12];
   // 13 - Additional Sense Code Qualifier
   // 14 - Field Replaceable Unit Code
   // 15 - MSB2(Sense Key Specific)
   // 16 - MSB1(Sense Key Specific)
   // 17 - MSB0(Sense Key Specific)

   // Receiv CSW, but ignore status
   nb=13;
   status = host_get_data(DMS_pipe_in(),&nb,datas);
   if( PIPE_GOOD != status )
   {
      if( PIPE_STALL == status )
        host_mem_stall_management();
      return CTRL_FAIL;
   }

   // Translate Sense Code to device status
   switch( sense_key )
   {
      case SBC_SENSE_KEY_NOT_READY:
      if( SBC_ASC_MEDIUM_NOT_PRESENT == sense_key_add )
         return CTRL_NO_PRESENT;
      break;

      case SBC_SENSE_KEY_UNIT_ATTENTION:
      if( SBC_ASC_NOT_READY_TO_READY_CHANGE == sense_key_add )
         return CTRL_BUSY;
      if( SBC_ASC_MEDIUM_NOT_PRESENT == sense_key_add )
         return CTRL_NO_PRESENT;
      break;
/*
      case SBC_SENSE_KEY_NO_SENSE:
      case SBC_SENSE_KEY_HARDWARE_ERROR:
      case SBC_SENSE_KEY_DATA_PROTECT:
      break;
*/      
   }
   return CTRL_FAIL; // By default FAIL
}


//-----------------------------------------------------------------------
//-------- Functions to control read/write operations -------------------


//! @brief Read sectors on lun
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @param addr            Sector address to start read (unit 512B)
//! @param *ram            buffer 512B to store the sector readed
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_mem_2_ram( U8 lun, U32 addr, U8 *ram )
{
   U8    status;
   U16   nb;
   U8    u8_nb_sec_ignore_at_beg, u8_sector_size;
   U32   u32_address;

   if( !host_mem_select_lun(lun) )
      return CTRL_FAIL; // no present

   u8_nb_sec_ignore_at_beg    = 0;
  
   if( g_readctx_b_run )
   {
      // Check if the sector requested is the following of previous read command
      if( ( g_ms_devices[g_ms_sel_dms].device_index != g_readctx_u8_device)
      ||  ( g_readctx_u8_lun != g_ms_sel_lun)
      ||  ( g_readctx_u32_addr > addr)
      ||  ( (g_readctx_u32_addr+g_readctx_u16_sector) <= addr ) )
      {
         host_mem_mem_2_ram_stop();
         host_mem_select_lun(lun);
      }else{
         u8_nb_sec_ignore_at_beg = addr-g_readctx_u32_addr;
      }
   }

   if( !g_readctx_b_run )
   {
      u8_sector_size = g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size;

      // Compute the address in physical sector units
      u32_address                = addr / u8_sector_size;
      // Compute the number of sector to ignore from the beginning of the physical sector
      u8_nb_sec_ignore_at_beg    = addr % u8_sector_size;
      // Update context
      g_readctx_u16_sector       = u8_sector_size;
      g_readctx_u32_addr         = addr - u8_nb_sec_ignore_at_beg;
    
      // Send CBW
      host_mem_cbw_init();
      g_ms_cbw.lgt_lsb0          = 0x00;
      g_ms_cbw.lgt_lsb1          = u8_sector_size * (512/256);
      g_ms_cbw.lgt_lsb2          = 0x00;
      g_ms_cbw.lgt_lsb3          = 0x00;
      g_ms_cbw.dir               = SBC_CMD_DIR_IN;
      g_ms_cbw.lgt               = 0x0A;
      g_ms_cbw.cmd               = SBC_CMD_READ_10;
      //g_ms_cbw.u8_data[16-HOST_MS_CBW_DATA_POS]  = 0x00;              // 16 - CBWCB1 - Option
      g_ms_cbw.u8_data[17-HOST_MS_CBW_DATA_POS]    = MSB0(u32_address); // 17 - CBWCB2 - MSB3(Logical Block Address)
      g_ms_cbw.u8_data[18-HOST_MS_CBW_DATA_POS]    = MSB1(u32_address); // 18 - CBWCB3 - MSB2(Logical Block Address)
      g_ms_cbw.u8_data[19-HOST_MS_CBW_DATA_POS]    = MSB2(u32_address); // 19 - CBWCB4 - MSB1(Logical Block Address)
      g_ms_cbw.u8_data[20-HOST_MS_CBW_DATA_POS]    = MSB3(u32_address); // 20 - CBWCB5 - MSB0(Logical Block Address)
      //g_ms_cbw.u8_data[21-HOST_MS_CBW_DATA_POS]  = 0x00;              // 21 - CBWCB6 - reserved
      //g_ms_cbw.u8_data[22-HOST_MS_CBW_DATA_POS]  = 0x00;              // 22 - CBWCB7 - MSB1(Transfer Length)
      g_ms_cbw.u8_data[23-HOST_MS_CBW_DATA_POS]    = 0x01;              // 23 - CBWCB8 - MSB0(Transfer Length)
      //g_ms_cbw.u8_data[24-HOST_MS_CBW_DATA_POS]  = 0x00;              // 24 - CBWCB9 - Control
      if( !host_mem_cbw_send() )
         return CTRL_FAIL;
   }

   if( 0 != u8_nb_sec_ignore_at_beg )
   {
      // Ignore eventualy first sector of command (in case of device with sector >512B)
      // Update sector remaining
      g_readctx_u16_sector -= u8_nb_sec_ignore_at_beg;
      g_readctx_u32_addr   += u8_nb_sec_ignore_at_beg;
      while( 0 != u8_nb_sec_ignore_at_beg )
      {
         // Read one sector
         nb=HOST_SECTOR_SIZE;
         status = host_get_data(DMS_pipe_in(),&nb,NULL);
         if(PIPE_GOOD != status)
         {
            if(PIPE_STALL==status)
               host_mem_stall_management();
            host_mem_csw_read();
            return CTRL_FAIL;
         }
         u8_nb_sec_ignore_at_beg--;
      }
   }

   // Transfer data ...
   g_readctx_u16_sector--;
   g_readctx_u32_addr++;
   nb=HOST_SECTOR_SIZE;
   status = host_get_data(DMS_pipe_in(),&nb,ram);
   if(PIPE_GOOD != status)
   {
      if(PIPE_STALL==status)
         host_mem_stall_management();
      host_mem_csw_read();
      return CTRL_FAIL;
   }
   
   if( 0 != g_readctx_u16_sector )
   {
      //  Update context
      g_readctx_u8_device = g_ms_devices[g_ms_sel_dms].device_index;
      g_readctx_u8_lun = g_ms_sel_lun;
      g_readctx_b_run = TRUE;
      return CTRL_GOOD;
   }

   // Read CSW status
   g_readctx_b_run = FALSE;
   return host_mem_csw_read();
}


//! This function terminates the tranfer of a physical sector from memory
//!
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
//!
Ctrl_status host_mem_mem_2_ram_stop( void )
{
   U16 nb;
   U8  status;

   g_readctx_b_run = FALSE;

   g_ms_sel_lun = g_readctx_u8_lun;
   g_ms_sel_dms = g_readctx_u8_device;
   Host_select_device( g_ms_devices[g_ms_sel_dms].device_index );
    
   while( 0 != g_readctx_u16_sector )
   {
      // Read one sector
      nb=HOST_SECTOR_SIZE;
      status = host_get_data(DMS_pipe_in(),&nb,NULL);
      if(PIPE_GOOD != status)
      {
         if(PIPE_STALL==status)
            host_mem_stall_management();
         host_mem_csw_read();
         return CTRL_FAIL;
      }
      g_readctx_u16_sector--;
   }

   // Read CSW status
   return host_mem_csw_read();
}

//! @brief Write a sectors
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @param addr            Sector address to start write (unit 512B)
//! @param *ram            buffer 512B with the data to store in the sector writed
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_ram_2_mem( U8 lun, U32 addr, U8 *ram )
{
   U8    status;
   U16   nb, u16_sector;
   U8    u8_nb_sec_ignore_at_beg, u8_sector_size;
   U32   u32_address;
#if (USB_SUPPORT_MS_SECTOR_WR_MAX != 1)
   Ctrl_status read_status;
   U8    u8_i,u8_j;
   U32   u32_addr;
#endif
   
   if( !host_mem_select_lun(lun) )
      return CTRL_FAIL; // no present

   u8_sector_size = g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size;

   if( USB_SUPPORT_MS_SECTOR_WR_MAX < u8_sector_size )
      return CTRL_FAIL; // No supported this lun in write mode, because sector size != 512B


   // Compute the address in physical sector units
   u32_address                = addr / u8_sector_size;
   // Compute the number of sector to ignore from the beginning of the physical sector
   u8_nb_sec_ignore_at_beg    = addr % u8_sector_size;
   // Update context
   u16_sector                 = u8_sector_size;
    
#if (USB_SUPPORT_MS_SECTOR_WR_MAX != 1)
   // If the sector size > 512B then read sector and store it in a buffer
   if( 1 != u8_sector_size )
   {
      u32_addr = addr - u8_nb_sec_ignore_at_beg;
      for( u8_i=0,u8_j=0 ; u8_i<u8_sector_size; u8_i++,u32_addr++ )
      {
         if( u8_i == u8_nb_sec_ignore_at_beg )
            continue;
         read_status = host_mem_mem_2_ram( lun, u32_addr, &g_ms_cache[u8_j*512] );
         if( CTRL_GOOD != read_status )
            return CTRL_GOOD;
         u8_j++;
      }
   }
#endif
   
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }
   
   // Send CBW
   host_mem_cbw_init();
   //g_ms_cbw.lgt_lsb0        = 0x00;
   g_ms_cbw.lgt_lsb1          = u8_sector_size * (512/256);
   //g_ms_cbw.lgt_lsb2        = 0x00;
   //g_ms_cbw.lgt_lsb3        = 0x00;
   g_ms_cbw.dir               = SBC_CMD_DIR_OUT;
   g_ms_cbw.lgt               = 0x0A;
   g_ms_cbw.cmd               = SBC_CMD_WRITE_10;
   //g_ms_cbw.u8_data[16-HOST_MS_CBW_DATA_POS]  = 0x00;        // 16 - CBWCB1 - Option
   g_ms_cbw.u8_data[17-HOST_MS_CBW_DATA_POS]    = MSB0(u32_address);  // 17 - CBWCB2 - MSB3(Logical Block Address)
   g_ms_cbw.u8_data[18-HOST_MS_CBW_DATA_POS]    = MSB1(u32_address);  // 18 - CBWCB3 - MSB2(Logical Block Address)
   g_ms_cbw.u8_data[19-HOST_MS_CBW_DATA_POS]    = MSB2(u32_address);  // 19 - CBWCB4 - MSB1(Logical Block Address)
   g_ms_cbw.u8_data[20-HOST_MS_CBW_DATA_POS]    = MSB3(u32_address);  // 20 - CBWCB5 - MSB0(Logical Block Address)
   //g_ms_cbw.u8_data[21-HOST_MS_CBW_DATA_POS]  = 0x00;        // 21 - CBWCB6 - reserved
   //g_ms_cbw.u8_data[22-HOST_MS_CBW_DATA_POS]  = 0x00;        // 22 - CBWCB7 - MSB1(Transfer Length)
   g_ms_cbw.u8_data[23-HOST_MS_CBW_DATA_POS]    = 0x01;        // 23 - CBWCB8 - MSB0(Transfer Length)
   //g_ms_cbw.u8_data[24-HOST_MS_CBW_DATA_POS]  = 0x00;        // 24 - CBWCB9 - Control
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

#if (USB_SUPPORT_MS_SECTOR_WR_MAX != 1)
   if( 0 != u8_nb_sec_ignore_at_beg )
   {
      // Ignore eventualy first sector of command (in case of device with sector >512B)
      // Update sector remaining
      u16_sector -= u8_nb_sec_ignore_at_beg;
      for( u8_i=0; u8_i < u8_nb_sec_ignore_at_beg; u8_i++ )
      {
         // Transfer data ...
         nb=HOST_SECTOR_SIZE;
         status = host_send_data(DMS_pipe_out(),nb,&g_ms_cache[u8_i*512]);
         if(status==PIPE_STALL)
         {
            host_mem_stall_management();
            host_mem_csw_read();
            return CTRL_FAIL;
         }
      }
   }
#endif

   // Transfer data ...
   u16_sector--;
   nb=HOST_SECTOR_SIZE;
   status = host_send_data(DMS_pipe_out(),nb,ram);
   if(status==PIPE_STALL)
   {
      host_mem_stall_management();
      host_mem_csw_read();
      return CTRL_FAIL;
   }
   
#if (USB_SUPPORT_MS_SECTOR_WR_MAX != 1)
   while( 0 != u16_sector )
   {
      // Transfer data ...
      nb=HOST_SECTOR_SIZE;
      status = host_send_data(DMS_pipe_out(),nb,&g_ms_cache[u8_nb_sec_ignore_at_beg*512]);
      if(status==PIPE_STALL)
      {
         host_mem_stall_management();
         host_mem_csw_read();
         return CTRL_FAIL;
      }
      u8_nb_sec_ignore_at_beg++;
      u16_sector--;
   }
#endif
   
   // Read CSW status
   return host_mem_csw_read();
}


//! @brief Read sectors on lun
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @param addr            Sector address to start read (unit 512B)
//! @param *ram            buffer 512B to store the sector readed
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_mem_2_ram_ext( U8 lun, U32 addr, U8 *ram , U8 u8_nb_sector)
{
   Ctrl_status status;
   U8    u8_status;
   U8    u8_sector_size;
   U16   nb;
   U32   dCBWDataTransferLength;

   if( !host_mem_select_lun(lun) )
      return CTRL_FAIL; // no present

   u8_sector_size = g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size;

   if( 1 < u8_sector_size )
   {
      // For U-Disk with large block then use standard routine
      while( u8_nb_sector != 0 ) {
         status = host_mem_mem_2_ram(lun, addr, ram);
         if (status != CTRL_GOOD) return status;
         u8_nb_sector--;
         addr++;
         ram = (U8*)ram + HOST_SECTOR_SIZE;
      }
      return CTRL_GOOD;      
   }
      
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }
   
   dCBWDataTransferLength = (U32)u8_nb_sector * 512;

   // Send CBW
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = LSB0(dCBWDataTransferLength);     //  8 - LSB0W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb1          = LSB1(dCBWDataTransferLength);     //  9 - LSB1W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb2          = LSB2(dCBWDataTransferLength);     // 10 - LSB2W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb3          = LSB3(dCBWDataTransferLength);     // 11 - LSB3W(dCBWDataTransferLength)
   g_ms_cbw.dir               = SBC_CMD_DIR_IN;
   g_ms_cbw.lgt               = 0x0A;
   g_ms_cbw.cmd               = SBC_CMD_READ_10;
   //g_ms_cbw.u8_data[16-HOST_MS_CBW_DATA_POS]  = 0x00;           // 16 - CBWCB1 - Option
   g_ms_cbw.u8_data[17-HOST_MS_CBW_DATA_POS]    = MSB0(addr);     // 17 - CBWCB2 - MSB3(Logical Block Address)
   g_ms_cbw.u8_data[18-HOST_MS_CBW_DATA_POS]    = MSB1(addr);     // 18 - CBWCB3 - MSB2(Logical Block Address)
   g_ms_cbw.u8_data[19-HOST_MS_CBW_DATA_POS]    = MSB2(addr);     // 19 - CBWCB4 - MSB1(Logical Block Address)
   g_ms_cbw.u8_data[20-HOST_MS_CBW_DATA_POS]    = MSB3(addr);     // 20 - CBWCB5 - MSB0(Logical Block Address)
   //g_ms_cbw.u8_data[21-HOST_MS_CBW_DATA_POS]  = 0x00;           // 21 - CBWCB6 - reserved
   //g_ms_cbw.u8_data[22-HOST_MS_CBW_DATA_POS]  = 0x00;           // 22 - CBWCB7 - MSB1(Transfer Length)
   g_ms_cbw.u8_data[23-HOST_MS_CBW_DATA_POS]    = u8_nb_sector;   // 23 - CBWCB8 - MSB0(Transfer Length)
   //g_ms_cbw.u8_data[24-HOST_MS_CBW_DATA_POS]  = 0x00;           // 24 - CBWCB9 - Control
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Transfer data ...
   nb=HOST_SECTOR_SIZE;
   while( u8_nb_sector!= 0 )
   {
      u8_status = host_get_data(DMS_pipe_in(),&nb,ram);
      if(PIPE_GOOD != u8_status)
      {
         if(PIPE_STALL==u8_status)
            host_mem_stall_management();
         host_mem_csw_read();
         return CTRL_FAIL;
      }
      u8_nb_sector--;
      ram = (U8*)ram + HOST_SECTOR_SIZE;
   }

   // Read CSW status
   g_readctx_b_run = FALSE;
   return host_mem_csw_read();
}


//! @brief Write a sectors
//!
//! @param  lun,  global lun selected (global!=lun of specific USB device)
//!
//! @param addr            Sector address to start write (unit 512B)
//! @param u8_nb_sector    Number of sector to write
//! @param *ram            buffer with the data to store in the sector writed
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_ram_2_mem_ext( U8 lun, U32 addr, U8 *ram , U8 u8_nb_sector)
{
   U8    status;
   U16   nb;
   U8    u8_sector_size;
   U32   dCBWDataTransferLength;

   if( !host_mem_select_lun(lun) )
      return CTRL_FAIL; // no present

   u8_sector_size = g_ms_devices[ g_ms_sel_dms ].lun_info[ g_ms_sel_lun ].u8_block_size;

   if( 1 < u8_sector_size )
      return CTRL_FAIL; // No supported this lun in write mode, because sector size != 512B

      
   if (g_readctx_b_run)
   {
      host_mem_mem_2_ram_stop();
      host_mem_select_lun(lun);
   }
   
   dCBWDataTransferLength = (U32)u8_nb_sector * 512;

   // Send CBW
   host_mem_cbw_init();
   g_ms_cbw.lgt_lsb0          = LSB0(dCBWDataTransferLength);  //  8 - LSB0W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb1          = LSB1(dCBWDataTransferLength);  //  9 - LSB1W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb2          = LSB2(dCBWDataTransferLength);  // 10 - LSB2W(dCBWDataTransferLength)
   g_ms_cbw.lgt_lsb3          = LSB3(dCBWDataTransferLength);  // 11 - LSB3W(dCBWDataTransferLength)
   g_ms_cbw.dir               = SBC_CMD_DIR_OUT;
   g_ms_cbw.lgt               = 0x0A;
   g_ms_cbw.cmd               = SBC_CMD_WRITE_10;
   //g_ms_cbw.u8_data[16-HOST_MS_CBW_DATA_POS]  = 0x00;        // 16 - CBWCB1 - Option
   g_ms_cbw.u8_data[17-HOST_MS_CBW_DATA_POS]    = MSB0(addr);  // 17 - CBWCB2 - MSB3(Logical Block Address)
   g_ms_cbw.u8_data[18-HOST_MS_CBW_DATA_POS]    = MSB1(addr);  // 18 - CBWCB3 - MSB2(Logical Block Address)
   g_ms_cbw.u8_data[19-HOST_MS_CBW_DATA_POS]    = MSB2(addr);  // 19 - CBWCB4 - MSB1(Logical Block Address)
   g_ms_cbw.u8_data[20-HOST_MS_CBW_DATA_POS]    = MSB3(addr);  // 20 - CBWCB5 - MSB0(Logical Block Address)
   //g_ms_cbw.u8_data[21-HOST_MS_CBW_DATA_POS]  = 0x00;        // 21 - CBWCB6 - reserved
   //g_ms_cbw.u8_data[22-HOST_MS_CBW_DATA_POS]  = 0x00;        // 22 - CBWCB7 - MSB1(Transfer Length)
   g_ms_cbw.u8_data[23-HOST_MS_CBW_DATA_POS]    = u8_nb_sector;// 23 - CBWCB8 - MSB0(Transfer Length)
   //g_ms_cbw.u8_data[24-HOST_MS_CBW_DATA_POS]  = 0x00;        // 24 - CBWCB9 - Control
   if( !host_mem_cbw_send() )
      return CTRL_FAIL;

   // Transfer data ...
   nb=HOST_SECTOR_SIZE;
   while( u8_nb_sector!= 0 )
   {
      status = host_send_data(DMS_pipe_out(),nb,ram);
      if(status==PIPE_STALL)
      {
         host_mem_stall_management();
         host_mem_csw_read();
         return CTRL_FAIL;
      }
      u8_nb_sector--;
      ram = (U8*)ram + HOST_SECTOR_SIZE;
   }
     
   // Read CSW status
   return host_mem_csw_read();
}


//-----------------------------------------------------------------------
//-------------------- Functions low level ------------------------------

//! Select the USB device corresponding at the LUN
//! 
//! @param lun LUN number to select
//!
//! @return TRUE if the LUN number exist else FALSE
//!
Bool host_mem_select_lun(U8 lun)
{
   if( !Is_host_ms_configured() )
      return FALSE;

   for( g_ms_sel_dms=0; g_ms_sel_dms < g_ms_nb_connected; g_ms_sel_dms++ )
   {
      if(lun < g_ms_devices[g_ms_sel_dms].nb_lun )
      {
         if( 0xFF == g_ms_devices[g_ms_sel_dms].device_index )
            return FALSE;                          // Device disconnected
         g_ms_sel_lun = lun;
         Host_select_device( g_ms_devices[g_ms_sel_dms].device_index );
         return TRUE;
      }
      lun -= g_ms_devices[g_ms_sel_dms].nb_lun;
   }
   return FALSE;
}


//! @brief Initialize at 0 the data of cbw command
//!
void host_mem_cbw_init( void )
{
   U8 tmp;

   tmp = g_ms_cbw.tag_lsb0;
   // Clear all structure
   memset( &g_ms_cbw , 0, sizeof(g_ms_cbw) );
   // Write Flag
   g_ms_cbw.flag1 = 'U';
   g_ms_cbw.flag2 = 'S';
   g_ms_cbw.flag3 = 'B';
   g_ms_cbw.flag4 = 'C';
   // Increment tag
   g_ms_cbw.tag_lsb0 = tmp+1;
   // Update LUN
   g_ms_cbw.lun = g_ms_sel_lun;
}


//! @brief Send the CBW command
//!
//! @return FALSE in case of error
//!
Bool host_mem_cbw_send( void )
{
   return (PIPE_GOOD == host_send_data( DMS_pipe_out(), sizeof( g_ms_cbw ), (U8*) &g_ms_cbw ));
}


//! @brief Wait and read CSW status
//!
//! @return                          Ctrl_status
//!   It is ready              ->    CTRL_GOOD
//!   Memory unplug            ->    CTRL_NO_PRESENT
//!   Not initialize or change ->    CTRL_BUSY
//!   A error occur            ->    CTRL_FAIL
//!
Ctrl_status host_mem_csw_read( void )
{
   U16   nb;
   U8    status;
   U8    u8_datas[13];
   Bool  b_status = TRUE;   // By default no error
   Bool  b_stall  = TRUE;   // Accept one stall on CSW

   while(1)
   {
      nb = 13;
      status = host_get_data(DMS_pipe_in(),&nb,u8_datas);
      if( PIPE_GOOD == status )
         break;
      if( PIPE_STALL == status )
      {
         host_mem_stall_management();
         if( b_stall )
         {
            b_stall = FALSE;
            continue;   // Error STALL but retry
         }
      }
      return CTRL_FAIL;   
   }
  
   if( PIPE_GOOD == status )
   {
      // CSW receiv then check CSW
      // Check "USBS" string (0 to 3) 
      if(('U' != u8_datas[0] )
      || ('S' != u8_datas[1] )
      || ('B' != u8_datas[2] )
      || ('S' != u8_datas[3] ) )
      {
         b_status = FALSE;    // Flag error
      }
      // Check CBW Tag (4 to 7) 
      if((g_ms_cbw.tag_lsb0 != u8_datas[4] )
      || (0 != u8_datas[5] )
      || (0 != u8_datas[6] )
      || (0 != u8_datas[7] ) )
      {
         b_status = FALSE;    // Tag error
      }
      // CSW Data Residue (8 to 11)
      // Check CSW status (12)
      if ( 0 != u8_datas[12] )
         b_status = FALSE;    // Status FAIL
   }
   else
   {
      b_status = FALSE;       // Error PIPE IN, then status FAIL
   }

   if( !b_status )
      return host_mem_request_sense();

   return CTRL_GOOD;
}


//! This function writes data from the usb memory
//!
void host_mem_stall_management( void )
{
   U8 dummy;

   dummy = Host_get_endpoint_number() | ~MSK_EP_DIR;
   host_clear_endpoint_feature(dummy);
   Host_select_pipe(DMS_pipe_in());
   Host_ack_stall();
   Host_reset_pipe_data_toggle();
}


