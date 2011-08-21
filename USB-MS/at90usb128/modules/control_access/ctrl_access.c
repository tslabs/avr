/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the interface :
//!  - between MEMORY  <-> USB (chip in USB device mode)
//!  - between MEMORY* <-> RAM (e.g. Embedded FileSystem control)
//!  *in this case the memory may be a USB MS device connected on the USB host of the chip
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

#include "config.h"
#include "ctrl_access.h"


//_____ D E F I N I T I O N S ______________________________________________

#if (ACCESS_MEM_TO_RAM == ENABLED)
   #include "modules/file_system/fat.h"
#endif

#if (LUN_0 == ENABLE)
   U8 code  lun0_name[]=LUN_0_NAME;
#endif
#if (LUN_1 == ENABLE)
   U8 code  lun1_name[]=LUN_1_NAME;
#endif
#if (LUN_2 == ENABLE)
   U8 code  lun2_name[]=LUN_2_NAME;
#endif
#if (LUN_3 == ENABLE)
   U8 code  lun3_name[]=LUN_3_NAME;
#endif
#if (LUN_4 == ENABLE)
   U8 code  lun4_name[]=LUN_4_NAME;
#endif
#if (LUN_5 == ENABLE)
   U8 code  lun5_name[]=LUN_5_NAME;
#endif
#if (LUN_6 == ENABLE)
   U8 code  lun6_name[]=LUN_6_NAME;
#endif
#if (LUN_7 == ENABLE)
   U8 code  lun7_name[]=LUN_7_NAME;
#endif
#if (LUN_USB == ENABLE)
   U8 code  lunusb_name[]=LUN_USB_NAME;
#endif

//*************************************************************************
//**** Listing of commun interface ****************************************
//*************************************************************************

//! This function return the number of logical unit
//!
//! @return U8   number of logical unit in the system
//!
U8    get_nb_lun()
{
#if   (LUN_USB == ENABLED)
   return   (MAX_LUN + Lun_usb_get_lun());
#else
   return   MAX_LUN;
#endif
}


//! This function return the current logical unit
//!
//! @return U8   number of logical unit in the system
//!
U8    get_cur_lun()
{
   return   0; //TODO a specific management
}


//! This function test the state of memory, and start the initialisation of the memory
//!
//! MORE (see SPC-3 §5.2.4) : The TEST UNIT READY command allows an application client
//! to poll a logical unit until it is ready without the need to allocate space for returned data.
//! The TEST UNIT READY command may be used to check the media status of logical units with removable media.
//!
//! @param lun        Logical unit number
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!   Not initialize ->    CTRL_BUSY
//!
Ctrl_status mem_test_unit_ready( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      return Lun_0_test_unit_ready();
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      return Lun_1_test_unit_ready();
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      return Lun_2_test_unit_ready();
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      return Lun_3_test_unit_ready();
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      return Lun_4_test_unit_ready();
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      return Lun_5_test_unit_ready();
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      return Lun_6_test_unit_ready();
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      return Lun_7_test_unit_ready();
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return Lun_usb_test_unit_ready(lun - LUN_ID_USB);
      break;
#     endif
   }
   return   CTRL_FAIL;
}


//! This function return the capacity of the memory
//!
//! @param lun        Logical unit number
//!
//! @return *u32_last_sector the last address sector
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status mem_read_capacity( U8 lun , U32 _MEM_TYPE_SLOW_ *u32_last_sector )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      return Lun_0_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      return Lun_1_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      return Lun_2_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      return Lun_3_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      return Lun_4_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      return Lun_5_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      return Lun_6_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      return Lun_7_read_capacity( u32_last_sector );
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return Lun_usb_read_capacity( lun - LUN_ID_USB, u32_last_sector );
      break;
#     endif
   }
   return   CTRL_FAIL;
}


//! This function return the sector size of the memory
//!
//! @param lun        Logical unit number
//!
//! @return           size of sector (unit 512B)
//!
U8 mem_sector_size( U8 lun )
{
# if (LUN_USB == ENABLE)
   return (lun < LUN_ID_USB) ? 1 : Lun_usb_read_sector_size(lun- LUN_ID_USB);
#else
   return 1;
#endif
}


//! This function return is the write protected mode
//!
//! @param lun        Logical unit number
//!
//! Only used by memory removal with a HARDWARE SPECIFIC write protected detection
//! !!! The customer must be unplug the card for change this write protected mode.
//!
//! @return TRUE  -> the memory is protected
//!
Bool  mem_wr_protect( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      return Lun_0_wr_protect();
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      return Lun_1_wr_protect();
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      return Lun_2_wr_protect();
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      return Lun_3_wr_protect();
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      return Lun_4_wr_protect();
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      return Lun_5_wr_protect();
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      return Lun_6_wr_protect();
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      return Lun_7_wr_protect();
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return Lun_usb_wr_protect(lun - LUN_ID_USB);
      break;
#     endif
   }
   return   CTRL_FAIL;
}


//! This function inform about the memory type
//!
//! @param lun        Logical unit number
//!
//! @return TRUE  -> The memory is removal
//!
Bool  mem_removal( U8 lun )
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      return Lun_0_removal();
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      return Lun_1_removal();
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      return Lun_2_removal();
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      return Lun_3_removal();
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      return Lun_4_removal();
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      return Lun_5_removal();
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      return Lun_6_removal();
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      return Lun_7_removal();
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return Lun_usb_removal();
      break;
#     endif
   }
   return   CTRL_FAIL;
}


//! This function returns a pointer to the LUN name
//!
//! @param lun        Logical unit number
//!
//! @return pointer to code string
//!
#ifdef __GNUC__
U8* mem_name( U8 lun )
#else
U8 code*  mem_name( U8 lun )
#endif
{
   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      return (U8 code*)lun0_name;
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      return (U8 code*)lun1_name;
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      return (U8 code*)lun2_name;
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      return (U8 code*)lun3_name;
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      return (U8 code*)lun4_name;
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      return (U8 code*)lun5_name;
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      return (U8 code*)lun6_name;
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      return (U8 code*)lun7_name;
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return (U8 code*)lunusb_name;
      break;
#     endif
   }
   return 0;   // Remove compiler warning
}



//*************************************************************************
//**** Listing of READ/WRITE interface ************************************
//*************************************************************************


//! This function tranfer a data from memory to usb
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start read (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    memory_2_usb( U8 lun , U32 addr , U16 nb_sector )
{
   Ctrl_status status=CTRL_FAIL;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      status = Lun_0_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      status = Lun_1_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      status = Lun_2_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      status = Lun_3_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      status = Lun_4_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      status = Lun_5_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      status = Lun_6_read_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      status = Lun_7_read_10(addr , nb_sector);
      break;
#     endif
   }
   return   status;
}

//! This function transfer a data from usb to memory
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start write (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    usb_2_memory( U8 lun , U32 addr , U16 nb_sector )
{
   Ctrl_status status=CTRL_FAIL;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      status = Lun_0_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      status = Lun_1_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      status = Lun_2_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      status = Lun_3_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      status = Lun_4_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      status = Lun_5_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      status = Lun_6_write_10(addr , nb_sector);
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      status = Lun_7_write_10(addr , nb_sector);
      break;
#     endif
   }
   return   status;
}

//! Interface for RAM
#if (ACCESS_MEM_TO_RAM == ENABLED)

//! This function tranfer one sector data from memory to ram
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start read (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//! @param ram          Adresse of ram buffer (only xdata)
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    memory_2_ram( U8 lun , const U32 _MEM_TYPE_SLOW_ addr , U8 _MEM_TYPE_SLOW_ *ram )
{

   Ctrl_status status=CTRL_FAIL;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      status = Lun_0_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      status = Lun_1_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      status = Lun_2_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      status = Lun_3_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      status = Lun_4_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      status = Lun_5_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      status = Lun_6_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      status = Lun_7_mem_2_ram(addr , ram);
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return  Lun_usb_mem_2_ram(lun - LUN_ID_USB,addr , ram);
#     endif
   }
   return   status;
}
#endif // ACCESS_MEM_TO_RAM == ENABLED



#if (ACCESS_MEM_TO_RAM==ENABLE)

//! This function transfer a data from ram to memory
//!
//! @param lun          Logical unit number
//! @param addr         Sector address to start write (sector = 512B)
//! @param nb_sector    Number of sectors to transfer
//! @param ram          Adresse of ram buffer (only xdata)
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!   Memory unplug  ->    CTRL_NO_PRESENT
//!
Ctrl_status    ram_2_memory( U8 lun , const U32 _MEM_TYPE_SLOW_ addr , U8 _MEM_TYPE_SLOW_ * ram )
{
   Ctrl_status status=CTRL_FAIL;

   switch( lun )
   {
#     if (LUN_0 == ENABLE)
      case LUN_ID_0:
      status = Lun_0_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_1 == ENABLE)
      case LUN_ID_1:
      status = Lun_1_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_2 == ENABLE)
      case LUN_ID_2:
      status = Lun_2_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_3 == ENABLE)
      case LUN_ID_3:
      status = Lun_3_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_4 == ENABLE)
      case LUN_ID_4:
      status = Lun_4_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_5 == ENABLE)
      case LUN_ID_5:
      status = Lun_5_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_6 == ENABLE)
      case LUN_ID_6:
      status = Lun_6_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_7 == ENABLE)
      case LUN_ID_7:
      status = Lun_7_ram_2_mem(addr , ram);
      break;
#     endif
#     if (LUN_USB == ENABLE)
      default:
      return  Lun_usb_ram_2_mem(lun - LUN_ID_USB,addr , ram);
      break;
#     endif
   }
   return   status;
}


//! This function copy a data from memory to other memory
//!
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer
//!
U8    stream_mem_to_mem( U8 src_lun , U32 src_addr , U8 dest_lun , U32 dest_addr , U16 nb_sector )
{
   Ctrl_status status=CTRL_FAIL;
   fat_cache_flush();
   while(nb_sector)
   {
      status = memory_2_ram( src_lun , src_addr , fs_g_sector );
      if( status != CTRL_GOOD )
         break;
      status = ram_2_memory( dest_lun , dest_addr , fs_g_sector );
      if( status != CTRL_GOOD )
         break;
      src_addr++;
      dest_addr++;
      nb_sector--;
   }
   fat_cache_reset();
   return (status != CTRL_GOOD)? ID_STREAM_ERR:0;
}


//! Returns the state on a data transfer
//!
//! @param id           transfer id
//!
//! @return the state of the transfer
//!          CTRL_GOOD              It is finish
//!          CTRL_BUSY              It is running
//!          CTRL_FAIL              It is fail
//!
Ctrl_status stream_state( U8 Id )
{
   return CTRL_GOOD;
}

//! Stop the data transfer
//!
//! @param id  Transfer id
//!
//! @return the number of sector remainder
//!
U16 stream_stop( U8 Id )
{
   return 0;
}

#endif   // ACCESS_RAM_TO_MEM == ENABLED

