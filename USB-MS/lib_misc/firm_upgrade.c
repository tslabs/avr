/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the feature USB host bootloader with Udisk (see AppNote AVR916).
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

#ifndef HOST_UPGRADE_MODE
#  warning HOST_UPGRADE_MODE not defined as ENABLE or DISABLE, using DISABLE...
#  define HOST_UPGRADE_MODE  DISABLE
#endif

// File system includes required for SYNC mode
#if (HOST_UPGRADE_MODE==ENABLE)

#  ifndef __ICCAVR__
#  error HOST_UPGRADE_MODE feature run only under IAR
#  endif

#include "fat.h"
#include "fs_com.h"
#include "navigation.h"
#include "file.h"
#include "nav_utils.h"
#include "flash_lib.h"
#include "stdio.h"
#include "string.h"

//_____ M A C R O S ________________________________________________________
 
// Maximum of data by line in HEX file
#define  MAX_DATA_LINE        16

// Define the upgrade zone autorized
#define  UPGRADE_SIZE_MAX     50
#define  UPGRADE_ADDR         0x333    
   

//_____ D E C L A R A T I O N S ____________________________________________

// This array allocs a space in flash (=code)
// which will upgraded by the feature "USB host bootloader with Udisk"
code char msg_to_upgrade[UPGRADE_SIZE_MAX] At(UPGRADE_ADDR);

// Struct used to decode HEX file
typedef struct
{
   U8      u8_nb_data;  // Nb data included in line
   U16     u16_add;     // Address to store data
   U8      u8_type;     // Type of line
   U8*     datas;       // Datas included in line
}  St_hex_line;
     
  
// Internal routines used to upgrade firmware
static   void  firm_upgrade_displayzone   ( void );
static   void  firm_upgrade_status        ( char *status );
static   Bool  firm_upgrade_searchfile    ( const char* filename );
static   Bool  firm_upgrade_readhexline   ( St_hex_line* line );
static   U8    firm_upgrade_ascii2bin     ( U8 ascii );
static   U8    firm_upgrade_readbyte      ( void );


//! This function runs the upgrade process
//! 
//! @verbatim
//! The status of process is display on UART and writed in a status file on U-Disk.
//! @endverbatim
//! 
void firm_upgrade_run( void )
{
   Fs_index sav_index;
   U8 u8_i, save_int;
   St_hex_line hex_line;
   U8 datas[MAX_DATA_LINE];
   char filename[30];

   hex_line.datas    = datas; // Init data buffer used
   hex_line.u8_type  = 0;     // Autotrize only type 0

   // Save current position to resolve it before exit routine
   sav_index = nav_getindex();
   
   printf("\n\rDislpay upgrade zone BEFORE upgrade:\n\r");
   firm_upgrade_displayzone();

   printf("Search upgrade file\n\r...\r");
   if( !firm_upgrade_searchfile( "upgrade*" ) )
   {
      firm_upgrade_status("No upgrade file");
      nav_gotoindex(&sav_index);
      return;
   }
   
   nav_file_getname( filename, 50 );
   printf("Open upgrade file \"%s\"\n\r...\r", filename );
   if( !file_open(FOPEN_MODE_R) )
   {
      firm_upgrade_status("!! Error to open upgrade file");
      nav_gotoindex(&sav_index);       // Restore previous position
      return;
   }
   
   printf("Check upgrade file\n\r...\r");
   while (!file_eof())
   { 
      // For each text line, check upgrade zone
      hex_line.u8_nb_data = MAX_DATA_LINE;
      if( !firm_upgrade_readhexline( &hex_line ) )
      {
         file_close();
         firm_upgrade_status("!! Error in HEX file format");
         nav_gotoindex(&sav_index);
         return;
      }

      if( (hex_line.u16_add                      <  UPGRADE_ADDR)
      ||  (hex_line.u16_add+hex_line.u8_nb_data) > (UPGRADE_ADDR+UPGRADE_SIZE_MAX) )
      {
         file_close();
         firm_upgrade_status("!! Upgrade zone not autorized");
         nav_gotoindex(&sav_index);
         return;
      }
   }
   
   printf("Program FLASH\n\r...\r");
   // Check bootloader
   if( !flash_lib_check() )
   {
      file_close();
      firm_upgrade_status("!! The bootloder if not loaded");
      nav_gotoindex(&sav_index);
      return;
   }
   
   file_seek(0,FS_SEEK_SET);              // Restart at beginning of file
   while (!file_eof())
   { 
      // For each text line, check upgrade zone
      hex_line.u8_nb_data = MAX_DATA_LINE;
      if( !firm_upgrade_readhexline( &hex_line ) )
      {
         file_close();
         firm_upgrade_status("!! Error in HEX file format");
         nav_gotoindex(&sav_index);
         return;
      }

      // Disabling the interrupt
      save_int=Get_interrupt_state();
      Disable_interrupt();
      // Writing the flash with a check of highest flash address 
      flash_wr_block( hex_line.datas, hex_line.u16_add, hex_line.u8_nb_data );
      // Read data in flash and check programmation with buffer
      for( u8_i=0; u8_i<hex_line.u8_nb_data; u8_i++ )
      {
         if( hex_line.datas[u8_i] != flash_rd_byte((U8 farcode*)hex_line.u16_add) )
         {
            file_close();
            firm_upgrade_status("!! Programmation in flash BAD\n\r!!! Check if the bootloader is loaded in chip.");
            nav_gotoindex(&sav_index);
            return;
         }
         hex_line.u16_add++;
      }     
      // Restore interrupt state
      if(save_int) { Enable_interrupt(); }
   }

   file_close();
   firm_upgrade_status("Upgrade successfull");
   nav_gotoindex(&sav_index);
   
   printf("Dislpay upgrade zone AFTER upgrade:\n\r");
   firm_upgrade_displayzone();
}


//! Displays on UART the space used to upgrade firmware
//!
static   void firm_upgrade_displayzone( void )
{
   char ram_msg_upgrade[UPGRADE_SIZE_MAX+1];

   // Move data from FLASH to RAM, because printf don't support flash
   for( U8 u8_i=0; u8_i<UPGRADE_SIZE_MAX; u8_i++ )
   {
     ram_msg_upgrade[u8_i] = msg_to_upgrade[u8_i];
   }
   ram_msg_upgrade[UPGRADE_SIZE_MAX]=0;   // Add terminator in case of
   printf( "\"" );
   printf( ram_msg_upgrade );
   printf( "\"\r\n" );
} 
   

//! Searchs a HEX file in the U-Disks
//!
//! @param file name to search
//!
//! @return TRUE, if file found
//!
static   Bool firm_upgrade_searchfile( const char* filename )
{  
   U8 u8_drive_lun;

   //** Search "Hello.txt" file in all USB-Disk
   u8_drive_lun = 1; // 1 is USB lun in this demo (0 = DataFlash, 1 = USB Host)
   while( 1 )
   {
      if( u8_drive_lun >= nav_drive_nb() )
         return FALSE;
      // Mount USB disk
      nav_drive_set( u8_drive_lun );
      if( nav_partition_mount() )
      {
         // Mount OK then seacrh file
         if( nav_setcwd((FS_STRING)filename,TRUE,FALSE) )
            break;   // File Found
      }
      u8_drive_lun++;   // Go to next USB disk
   }
   return TRUE;
}


//! Reads and decodes a text line with HEX format
//!
//! @param line   Specify in struct:
//!               - u8_nb_data, number max. of data supported
//!               - datas, init. buffer to fill 
//!
//! @return TRUE, if line supported and line struct filled
//!
static   Bool firm_upgrade_readhexline( St_hex_line* line )
{  
   U8 u8_i, u8_nb_data, u8_crc=0;
   U8 *ptr_data;
   
   // Check header line
   if( ':' != file_getc())
      return FALSE;
   
   // Get data size
   u8_nb_data = firm_upgrade_readbyte();
   u8_crc += u8_nb_data;
   if( u8_nb_data > line->u8_nb_data )
      return FALSE;
   line->u8_nb_data = u8_nb_data;
   
   // Get data address 
   line->u16_add = (((U16)firm_upgrade_readbyte())<<8) | firm_upgrade_readbyte();
   u8_crc += LSB(line->u16_add);
   u8_crc += MSB(line->u16_add);
   
   // Read record type
   // - 00, data record, contains data and 16-bit address. The format described above. 
   // - 01, End Of File record, a file termination record. No data. Has to be the last line of the file, only one per file permitted. Usually ':00000001FF'. Originally the End Of File record could contain a start address for the program being loaded, e.g. :00AB2F0125 would make a jump to address AB2F. This was convenient when programs were loaded from punched paper tape. 
   // - 02, Extended Segment Address Record, segment-base address. Used when 16 bits are not enough, identical to 80x86 real mode addressing. The address specified by the 02 record is multiplied by 16 (shifted 4 bits left) and added to the subsequent 00 record addresses. This allows addressing of up to a megabyte of address space. The address field of this record has to be 0000, the byte count is 02 (the segment is 16-bit). The least significant hex digit of the segment address is always 0. 
   // - 03, Start Segment Address Record. For 80x86 processors, it specifies the initial content of the CS:IP registers. The address field is 0000, the byte count is 04, the first two bytes are the CS value, the latter two are the IP value. 
   // - 04, Extended Linear Address Record, allowing for fully 32 bit addressing. The address field is 0000, the byte count is 02. The two data bytes represent the upper 16 bits of the 32 bit address, when combined with the address of the 00 type record. 
   // - 05, Start Linear Address Record. The address field is 0000, the byte count is 04. The 4 data bytes represent the 32-bit value loaded into the EIP register of the 80386 and higher CPU.
   line->u8_type = firm_upgrade_readbyte();
   u8_crc += line->u8_type;
   
   // Get data
   ptr_data = line->datas;
   for( u8_i=0; u8_i<u8_nb_data; u8_i++ )
   {
      *ptr_data = firm_upgrade_readbyte();
      u8_crc += *ptr_data;
      ptr_data++;
   }
   
   // Check CRC
   // If the checksum is correct, adding all the bytes (the Byte count, both bytes in Address, the Record type, each Data byte and the Checksum)
   // together will always result in a value wherein the least significant byte is zero (0x00). 
   u8_crc += firm_upgrade_readbyte();
   
   // Read end of line = '\r' and '\n'
   file_getc();file_getc();
   
   return( 0 == u8_crc );
}

      
//! This function creates a file on disk with the status of upgrade
//! 
//! @param status to write
//!
static   void firm_upgrade_status( char *status )
{
   char file_status_name[] = "status.txt";

   printf( status );
   printf( "\n\r" );
   
   // Create a status file if possible
   if( !nav_file_create((FS_STRING)file_status_name) )
   {
      if( fs_g_status != FS_ERR_FILE_EXIST )
         return; // Error during creation file
      // File exist then continue
   }
   if( file_open(FOPEN_MODE_W) )
   {
      // Write status in file
      file_write_buf( (unsigned char*) status, strlen(status) );
      file_close();
   }
}


//! This function is used to convert a ascii character into 4 bit number
//! '5' => 5       'A' => 10
//! 
//! @param ASCII characters to convert
//!
//! @return converted byte
//!
static   U8 firm_upgrade_ascii2bin(U8 ascii)
{
   U8 byte=0;
   if( ('0'<=ascii) && (ascii<='9') )
      byte = ascii-'0';
   if( ('a'<=ascii) && (ascii<='f') )
      byte = (ascii-'a'+10);
   if( ('A'<=ascii) && (ascii<='F') )
      byte = (ascii-'A'+10);
   return byte;
}

//! This function reads and converts two ascii characters into byte number
//! '51' => 0x51       'A1' => 0xA1
//! 
//! @return converted byte
//!
static   U8 firm_upgrade_readbyte( void )
{
   U8 ascii_msb ,ascii_lsb;
   ascii_msb = file_getc();
   ascii_lsb = file_getc();
   return (firm_upgrade_ascii2bin(ascii_msb)<<4) | firm_upgrade_ascii2bin(ascii_lsb);
}

#endif // (HOST_UPGRADE_MODE==ENABLE)

