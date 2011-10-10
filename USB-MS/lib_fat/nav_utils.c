/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file provides other related file name management for file system navigation
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
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
#include <stdio.h>
#include "nav_utils.h"


//_____ M A C R O S ________________________________________________________
#define  SIZE_NAME_COPY    50

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________


//! Intermediate ram unicode file name buffer
U8 str_ram[MAX_FILE_LENGHT];


//! @brief set the explorer navigator to the corresponding file or directory name stored in flash.
//!
//! @param *str: the file or directory name to found
//! @param case_sensitive
//!
//! @return status: TRUE for ok, FALSE if the name is not found
#ifdef __GNUC__
U8 goto_code_name(U8 *str, Bool b_case_sensitive, Bool b_create)
#else
U8 goto_code_name(U8 code *str, Bool b_case_sensitive, Bool b_create)
#endif
{
   str_code_to_str_ram( str, str_ram );
   return nav_setcwd( (FS_STRING)str_ram, b_case_sensitive, b_create);
}


//! @brief copy an string code to a ram buffer
//!
//! @param U8 code string_code: the string code pointer
//! @param U8 *str_ram: the ram buffer pointer
//!
//! @return none
#ifdef __GNUC__
void str_code_to_str_ram(U8 string_code[],U8 str_ram[MAX_FILE_LENGHT])
#else
void str_code_to_str_ram(U8 code string_code[],U8 str_ram[MAX_FILE_LENGHT])
#endif
{
   U8 i;
   for(i=0;i<MAX_FILE_LENGHT;i++)
#ifndef __GNUC__
   {   str_ram[i]=string_code[i];}
#else
   {   str_ram[i]=pgm_read_byte_near((unsigned int)string_code+i); }
#endif   
}


//! @brief This function copys a source directory to an other directory
//!
#ifdef __GNUC__
Bool copy_dir( U8 string_src[], U8 string_dst[], Bool b_print  )
#else
Bool copy_dir( U8 code string_src[], U8 code string_dst[], Bool b_print )
#endif
{
   Fs_index sav_index;
   char s_name[SIZE_NAME_COPY];
   U8 u8_folder_level = 0;

   if( b_print ) printf("\n\r");
   // Save the position
   sav_index = nav_getindex();

   //** Use three navigators (to explore source, to explore destination, to copy file routine)
   // Select source directory
   if( b_print ) printf("Select source directory\n\r");
   nav_select( FS_NAV_ID_COPY_SRC_DIR );
   if( !goto_code_name( string_src, FALSE, FALSE ) )
      goto copy_dir_error;
   nav_filelist_reset();
   // Select destination directory
   if( b_print ) printf("Select destination directory\n\r");
   nav_select( FS_NAV_ID_COPY_DST_DIR );
   if( !goto_code_name( string_dst, FALSE, TRUE ) )
      goto copy_dir_error;
   nav_filelist_reset();


   // loop to scan and create ALL folders and files
   while(1)
   {
      // No dir in current dir then go to parent dir on SD and NandFlash disk
      while(1)
      {
         //printf("Search files or dir\n\r");
         // Reselect SD
         nav_select( FS_NAV_ID_COPY_SRC_DIR );
         if( nav_filelist_set( 0 , FS_FIND_NEXT ) )
            break;   // a next file and directory is found
   
         // No other dir or file in current dir then go to parent dir on SD and NandFlash disk
         if( 0 == u8_folder_level )
         {
            // end of update folder
            //********* END OF COPY **************
            goto copy_dir_finish;
         }

         if( b_print ) printf("Go to parent\n\r");
         // Remark, nav_dir_gotoparent() routine go to in parent dir and select the children dir in list
         u8_folder_level--;
         if( !nav_dir_gotoparent() )
            goto copy_dir_error;
         // Select NandFlash navigator and go to the same dir of SD
         nav_select( FS_NAV_ID_COPY_DST_DIR );
         if( !nav_dir_gotoparent() )
            goto copy_dir_error;
      } // end of while (1)
      
      if( nav_file_isdir())
      {
         if( b_print ) printf("Dir found - create dir: ");
         //** here, a new directory is found and is selected
         // Get name of current selection (= dir name on SD)
         if( !nav_file_name( (FS_STRING)s_name, MAX_FILE_LENGHT, FS_NAME_GET, FALSE ))
            goto copy_dir_error;
         // Enter in dir (on SD)
         if( !nav_dir_cd())
            goto copy_dir_error;
         u8_folder_level++;
         // Select NandFlash disk
         nav_select( FS_NAV_ID_COPY_DST_DIR );
         // Create folder in NandFlash disk
         if( b_print ) printf((char*)s_name);
         if( b_print ) printf("\n\r");
         if( !nav_dir_make( (FS_STRING )s_name ))
         {
            if( FS_ERR_FILE_EXIST != fs_g_status )
               goto copy_dir_error;
            // here, error the name exist
         }
         // Here the navigator have selected the folder on NandFlash
         if( !nav_dir_cd())
         {
            if( FS_ERR_NO_DIR == fs_g_status )
            {
               // FYC -> Copy impossible, because a file have the same name of folder
            }
            goto copy_dir_error;
         }
         // here, the folder is created and the navigatorS is entered in this dir
      }
      else
      {
         if( b_print ) printf("File found - copy file: ");
         //** here, a new file is found and is selected
         // Get name of current selection (= file name on SD)
         if( !nav_file_name( (FS_STRING)s_name, MAX_FILE_LENGHT, FS_NAME_GET, FALSE ))
            goto copy_dir_error;
         if( b_print ) printf((char*)s_name);
         if( b_print ) printf("\n\r");
         if( !nav_file_copy())
            goto copy_dir_error;

         // Paste file in current dir of NandFlash disk
         nav_select( FS_NAV_ID_COPY_DST_DIR );
         while( !nav_file_paste_start( (FS_STRING)s_name ) )
         {
            // Error
            if( fs_g_status != FS_ERR_FILE_EXIST )
               goto copy_dir_error;
            // File exists then deletes this one
            if( b_print ) printf("File exists then deletes this one.\n\r");
            if( !nav_file_del( TRUE ) )
               goto copy_dir_error;
            // here, retry PASTE                   
         }
         // Copy running
         {
         U8 status;
         do{
            status = nav_file_paste_state(FALSE);
         }while( COPY_BUSY == status );

         if( COPY_FINISH != status )
            goto copy_dir_error;
         }
      } // if dir OR file
   } // end of first while(1)
 
copy_dir_error:
   // Restore the position
   nav_select( FS_NAV_ID_USHELL_CMD );
   nav_gotoindex(&sav_index);
   if( b_print ) printf("!!!copy fail\n\r");
   return FALSE;
   
copy_dir_finish:
   // Restore the position
   nav_select( FS_NAV_ID_USHELL_CMD );
   nav_gotoindex(&sav_index);
   if( b_print ) printf("End of copy\n\r");
   return TRUE;
}


