/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file manages the µshell task for file system.
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
#include "ushell_task.h"
#include <stdio.h>
#include "df_mem.h"
#include "wdt_drv.h"
#include "uart_lib.h"
#include "usb_drv.h"
#include "timer16_drv.h"
#if (TARGET_BOARD==SPIDER)
#include "spider_drv.h"
#include "intermcu_spi_drv.h"
#endif
#include "fat.h"
#include "fs_com.h"
#include "navigation.h"
#include "file.h"
#include "nav_utils.h"
#include "usb_standard_request.h"
#include "ctrl_access.h"

#ifndef USHELL_RF
   #define USHELL_RF DISABLE
   #define ushell_putchar  uart_putchar
   #define ushell_test_hit uart_test_hit
   #define ushell_get_char uart_getchar
#endif

#if (USHELL_RF==ENABLE)
   #include "rf_task.h"
   void rf_task_init(void);
#endif


#if (USHELL_DFU==ENABLE)
#include "usb_host_task.h"
#include "usb_host_enum.h"
#include "host_dfu_task.h"
#endif

#if (USHELL_USB==ENABLE)
#include "conf_usb.h"
#include "usb_host_task.h"
#include "usb_host_enum.h"
#include "usb_standard_request.h"
#  if(USB_HUB_SUPPORT==ENABLE)
#include "ãsb_host_hub.h"
#  endif
#endif

#if (USHELL_HID==ENABLE)
#include "host_hid_task.h"
#endif


//_____ M A C R O S ________________________________________________________

#ifndef USHELL_DFU
#warning USHELL_DFU is not defined as ENABLE or DISABLE, using DISABLE by default...
#define  USHELL_DFU DISABLE
#endif

#ifndef USHELL_USB
#warning USHELL_USB is not defined as ENABLE or DISABLE, using DISABLE by default...
#define  USHELL_USB DISABLE
#endif

#ifndef USHELL_HID
#warning USHELL_HID is not defined as ENABLE or DISABLE, using DISABLE by default...
#define  USHELL_HID DISABLE
#endif

#ifndef USHELL_HISTORY
#warning USHELL_HISTORY value is not defined using 10 as default value
#define  USHELL_HISTORY 10
#endif

#ifndef USHELL_NB_LINE
#warning USHELL_NB_LINE value is not defined using 20 as default value
#define USHELL_NB_LINE  20
#endif


#ifndef USHELL_SIZE_CMD_LINE
#warning USHELL_SIZE_CMD_LINE value is not defined using 30 as default value
#define USHELL_SIZE_CMD_LINE  30
#endif

#define USHELL_MAX_NB_ARG     2

//_____ D E C L A R A T I O N S ____________________________________________


// Manage task
static Bool g_b_ushell_task_run = FALSE;

// To manage command line
static U8   g_u8_escape_sequence=0;
static U8   g_u8_cmd_size=0;
static U8   g_u8_history_pos=0;
static U8   g_u8_history_pos_search=0;
static char g_s_cmd_his[USHELL_HISTORY][USHELL_SIZE_CMD_LINE];
static char g_s_cmd[USHELL_SIZE_CMD_LINE];
static char g_s_arg[USHELL_MAX_NB_ARG][USHELL_SIZE_CMD_LINE];

// To manage a file system shortcut
static Fs_index g_mark_index;


//* To store all shell messages
U8 code str_cd[]=STR_CD;
U8 code str_mount[]=STR_MOUNT;
U8 code str_cp[]=STR_CP;
U8 code str_ls[]=STR_LS;
U8 code str_ls_more[]=STR_LS_MORE;
U8 code str_rm[]=STR_RM;
U8 code str_df[]=STR_DF;
U8 code str_space[]=STR_SPACE;
U8 code str_mkdir[]=STR_MKDIR;
U8 code str_touch[]=STR_TOUCH;
U8 code str_append[]=STR_APPEND;
U8 code str_cat[]=STR_CAT;
U8 code str_cat_more[]=STR_CAT_MORE;
U8 code str_up[]=STR_UP;
U8 code str_disk[]=STR_DISK;
U8 code str_mark[]=STR_MARK;
U8 code str_goto[]=STR_GOTO;
U8 code str_mv[]=STR_MV;
U8 code str_help[]=STR_HELP;
U8 code str_format[]=STR_FORMAT;
U8 code str_sync[]=STR_SYNC;
U8 code str_perform[]=STR_PERFORM;
U8 code str_reboot[]=STR_REBOOT;
#if (USHELL_USB==ENABLE)
U8 code str_ls_usb[]=STR_LS_USB;
U8 code str_usb_suspend[]=STR_USB_SUSPEND;
U8 code str_usb_resume[]=STR_USB_RESUME;
U8 code str_usb_force_enum[]=STR_USB_FORCE_ENUM;
#endif
#if (USHELL_DFU==ENABLE)
U8 code str_dfu_erase[]=STR_DFU_ERASE;
U8 code str_dfu_load[]=STR_DFU_LOAD;
U8 code str_dfu_start[]=STR_DFU_START;
#endif
#if (USHELL_HID==ENABLE)
U8 code str_hid_enter_dfu[]=STR_HID_ENTER_DFU;
U8 code str_hid_get_info[]=STR_HID_GET_INFO;
#endif
U8 code msg_paste_fail[]=MSG_ER_PASTE;
U8 code msg_prompt[]=MSG_PROMPT;
U8 code msg_welcome[]=MSG_WELCOME;
U8 code msg_exit[]=MSG_EXIT;
U8 code msg_er_mount[]=MSG_ER_MOUNT;
U8 code msg_er_drive[]=MSG_ER_DRIVE;
U8 code msg_er_rm[]=MSG_ER_RM;
U8 code msg_er_unknown_file[]=MSG_ER_UNKNOWN_FILE;
U8 code msg_er_cmd_not_found[]=MSG_ER_CMD_NOT_FOUND;
U8 code msg_er_format[]=MSG_ER_FORMAT;
U8 code msg_append_welcome[]=MSG_APPEND_WELCOME;
U8 code msg_help[]=MSG_HELP;
U8 code msg_no_device[]=MSG_NO_DEVICE;
U8 code msg_ok[]=MSG_OK;
U8 code msg_ko[]=MSG_KO;
#if (USHELL_USB==ENABLE)
U8 code msg_remote_wake_up_ok[]=MSG_REMOTE_WAKEUP_OK;
U8 code msg_remote_wake_up_ko[]=MSG_REMOTE_WAKEUP_KO;
U8 code msg_device_self_powered[]=MSG_SELF_POWERED;
U8 code msg_device_bus_powered[]=MSG_BUS_POWERED;
U8 code msg_usb_suspended[]=MSG_USB_SUSPENDED;
U8 code msg_device_full_speed[]=MSG_DEVICE_FULL_SPEED;
U8 code msg_device_low_speed[]=MSG_DEVICE_LOW_SPEED;
#endif


// Internal sub routines
Bool  ushell_cmd_scan         ( void );
U8    ushell_cmd_decode       ( void );
void  ushell_clean_cmd_line   ( void );
void  ushell_history_up       ( void );
void  ushell_history_down     ( void );
void  ushell_history_display  ( void );
#ifdef __GNUC__
   U8    mystrncmp      (char *str1,U8 *str2,U8 i);
   void  print_msg      (U8 *str);
#else
   U8    mystrncmp      (char *str1,U8 code *str2,U8 i);
   void  print_msg      (U8 code *str);
#endif
Bool  ushell_more_wait        ( void );
// Internal sub routines for file system commands
void  ushell_cmd_nb_drive     ( void );
void  ushell_cmd_free_space   ( void );
void  ushell_cmd_format       ( void );
void  ushell_cmd_mount        ( void );
void  ushell_cmd_space        ( void );
void  ushell_cmd_ls           ( Bool b_more );
void  ushell_cmd_cd           ( void );
void  ushell_cmd_gotoparent   ( void );
void  ushell_cmd_cat          ( Bool b_more);
void  ushell_cmd_help         ( void );
void  ushell_cmd_mkdir        ( void );
void  ushell_cmd_touch        ( void );
void  ushell_cmd_rm           ( void );
void  ushell_cmd_append_file  ( void );
void  ushell_cmd_copy         ( void );
void  ushell_cmd_rename       ( void );
void  ushell_cmd_reboot       ( void );
Bool  ushell_cmd_sync         ( void );
void  ushell_cmd_perform      ( void );
void  ushell_path_valid_syntac( char *path );
// Internal sub routines for USB commands
void  ushell_cmdusb_ls        ( void );
void  ushell_cmdusb_suspend   ( void );
void  ushell_cmdusb_resume    ( void );
void  ushell_cmdusb_force_enum( void );
// Internal sub routines for USB DFU commands
void  ushell_cmddfu_erase     ( void );
void  ushell_cmddfu_load      ( void );
void  ushell_cmddfu_start     ( void );
// Internal sub routines for USB HID commands
void  ushell_cmdhid_enter_dfu ( void );
void  ushell_cmdhid_getinfo   ( void );



//! @brief This function initializes the hardware/software ressources required for ushell task.
//!
void ushell_task_init(void)
{
   U8 u8_i;
#if (USHELL_RF==ENABLE)
   rf_task_init();
#else
   uart_init();
#endif

#ifdef __GNUC__
   fdevopen((int (*)(char, FILE*))(ushell_putchar),(int (*)(FILE*))ushell_get_char); //for printf redirection
#endif
   g_b_ushell_task_run = FALSE;
   for( u8_i=0; u8_i<USHELL_HISTORY; u8_i++ ) {
      g_s_cmd_his[u8_i][0] = 0;  // Set end of line for all cmd line history
   }
  
}


//! @brief Entry point of the explorer task management
//!
//! This function performs ushell task decoding to access file system functions.
//!
void ushell_task(void)
{
   //** Check the USB mode and autorize/unautorize ushell
   if(!g_b_ushell_task_run)
   {
      if( Is_usb_id_device() )
         return;
      g_b_ushell_task_run = TRUE;
      print_msg((U8 code *)msg_welcome);
      ushell_cmd_nb_drive();
      print_msg((U8 code *)msg_prompt);
      // Reset the embedded FS on ushell navigator and on first drive
      nav_reset();
      nav_select( FS_NAV_ID_USHELL_CMD );
   }else{
      if( Is_usb_id_device() )
      {
         g_b_ushell_task_run = FALSE;
         print_msg((U8 code *)msg_exit);
         nav_exit();
         return;
      }
   }

   if( !ushell_cmd_scan() )
      return;

   //** Command ready then decode and execute this one
   switch( ushell_cmd_decode() )
   {
      // Displays number of  drives
      case CMD_NB_DRIVE:
      ushell_cmd_nb_drive();
      break;

      // Displays free space information for all connected drives
      case CMD_DF:
      ushell_cmd_free_space();
      break;

      // Formats disk
      case CMD_FORMAT:
      ushell_cmd_format();
      break;
      
      // Mounts a drive (e.g. "b:")
      case CMD_MOUNT:
      ushell_cmd_mount();
      break;

      // Displays the space information for current drive
      case CMD_SPACE:
      ushell_cmd_space();
      break;
      
      // Lists the files present in current directory (e.g. "ls")
      case CMD_LS:
      ushell_cmd_ls(FALSE);
      break;
      case CMD_LS_MORE:
      ushell_cmd_ls(TRUE);
      break;

      // Enters in a directory (e.g. "cd folder_toto")
      case CMD_CD:
      ushell_cmd_cd();
      break;

      // Enters in parent directory ("cd..")
      case CMD_UP:
      ushell_cmd_gotoparent();
      break;

      // Displays a text file
      case CMD_CAT:
      ushell_cmd_cat(FALSE);
      break;
      case CMD_CAT_MORE:
      ushell_cmd_cat(TRUE);
      break;
         
      // Displays the help
      case CMD_HELP:
      ushell_cmd_help();
      break;

      // Creates directory
      case CMD_MKDIR:
      ushell_cmd_mkdir();
      break;

      // Creates file
      case CMD_TOUCH:
      ushell_cmd_touch();
      break;
      
      // Deletes files or directories
      case CMD_RM:
      ushell_cmd_rm();
      break;

      // Appends char to selected file
      case CMD_APPEND:
      ushell_cmd_append_file();
      break;

      // Index routines (= specific shortcut from ATMEL FileSystem)
      case CMD_SET_ID:
      g_mark_index = nav_getindex();
      break;
      case CMD_GOTO_ID:
      nav_gotoindex( &g_mark_index );
      break;
      
      // Copys file to other location
      case CMD_CP:
      ushell_cmd_copy();
      break;

      // Renames file
      case CMD_MV:
      ushell_cmd_rename();
      break;

      // Synchronize folders
      case CMD_SYNC:
      ushell_cmd_sync();
      break;

      // Perform transfer
      case CMD_PERFORM:
      ushell_cmd_perform();
      break;

      // Reboot target
      case CMD_REBOOT:
      ushell_cmd_reboot();
      break;

      // USB commands
#if (USHELL_USB==ENABLE)
      case CMD_LS_USB:
      ushell_cmdusb_ls();
      break;
      case CMD_USB_SUSPEND:
      ushell_cmdusb_suspend();
      break;
      case CMD_USB_RESUME:
      ushell_cmdusb_resume();
      break;
      case CMD_USB_FORCE_ENUM:
      ushell_cmdusb_force_enum();
      break;
#endif

      // DFU commands
#if (USHELL_DFU==ENABLE)
      case CMD_DFU_ERASE:
      ushell_cmddfu_erase();
      break;
      case CMD_DFU_LOAD:
      ushell_cmddfu_load();
      break;
      case CMD_DFU_START:
      ushell_cmddfu_start();
      break;
#endif

      // HID commands
#if (USHELL_HID==ENABLE)
      case CMD_HID_ENTER_DFU:
      ushell_cmdhid_enter_dfu();
      break;
      case CMD_HID_GET_INFO:
      ushell_cmdhid_getinfo();
      break;
#endif

      // Unknown command
      default:
      print_msg((U8 code *)msg_er_cmd_not_found);
      break;
   }

   print_msg((U8 code *)msg_prompt);
}


//! @brief Get the full command line to be interpreted.
//!
//! @return TRUE, if a command is ready
//!
Bool ushell_cmd_scan(void)
{
   char c_key;

   // Something new of the UART ?
   if(!ushell_test_hit())   
      return FALSE;

   c_key=ushell_get_char();
   
   if( 0 != g_u8_escape_sequence )
   {
      //** Decode escape sequence
      if( 1 == g_u8_escape_sequence )
      {
         if( 0x5B != c_key )
         {
            g_u8_escape_sequence=0;
            return FALSE;  // Escape sequence cancel
         }
         g_u8_escape_sequence=2;
      }
      else
      {
         // Decode value of the sequence
         switch (c_key)
         {
/*
Note: OVERRUN error on USART with an RTOS and USART without interrupt management
If you want support "Escape sequence", then you have to implement USART interrupt management
            case 0x41:     // UP command
            ushell_clean_cmd_line();
            ushell_history_up();
            ushell_history_display();
            break;
            case 0x42:     // DOWN command           
            ushell_clean_cmd_line();
            ushell_history_down();
            ushell_history_display();
            break;
*/            
            default:       // Ignore other command
            break;
         }
         g_u8_escape_sequence=0; // End of Escape sequence 
      }
      return FALSE;
   }
   
   //** Normal sequence
   switch (c_key)
   {
      //** Command validation
      case ASCII_CR:
      ushell_putchar(ASCII_CR);     // Echo
      ushell_putchar(ASCII_LF);     // Add new line flag
      g_s_cmd_his[g_u8_history_pos][g_u8_cmd_size]=0;  // Add NULL terminator at the end of command line
      return TRUE;
      
      //** Enter in escape sequence
      case ASCII_ESCAPE:
      g_u8_escape_sequence=1;
      break;
      
      //** backspace
      case ASCII_BKSPACE:
      if(g_u8_cmd_size>0)        // Beginning of line ?
      {
         // Remove the last character on terminal
         ushell_putchar(ASCII_BKSPACE);   // Send a backspace to go in previous character
         ushell_putchar(' ');             // Send a space to erase previous character
         ushell_putchar(ASCII_BKSPACE);   // Send a backspace to go in new end position (=previous character position)
         // Remove the last character on cmd line buffer
         g_u8_cmd_size--;
      }
      break;
      
      // History management
      case '!':
      ushell_clean_cmd_line();
      ushell_history_up();
      ushell_history_display();
      break;
      case '$':
      ushell_clean_cmd_line();
      ushell_history_down();
      ushell_history_display();
      break;

      //** Other char
      default:
      if( (0x1F<c_key) && (c_key<0x7F) && (USHELL_SIZE_CMD_LINE!=g_u8_cmd_size) )
      {
         // Accept char
         ushell_putchar(c_key);                                   // Echo
         g_s_cmd_his[g_u8_history_pos][g_u8_cmd_size++] = c_key;  // append to cmd line
      }
      break;
   }
   return FALSE;
}


//! @brief decodes full command line into command type and arguments
//!
//!
//! @return the command type decoded
//!
//! @verbatim
//! The arguments are storage in g_s_arg global array
//! @endverbatim
//!
U8 ushell_cmd_decode( void )
{
   U8 cmd_type;
   U8 u8_i,u8_j,u8_k;
   U8 u8_cmd_size;
   Bool b_arg_include_space;
   
   if(0==g_u8_cmd_size)
   {
      // Command line empty
      print_msg((U8 code *)msg_prompt);
      return CMD_NONE;
   }

   // Get command string and Change command to lower case
   for( u8_i=0; (g_s_cmd_his[g_u8_history_pos][u8_i]!=' ') && (u8_i<=g_u8_cmd_size); u8_i++)
   {
      g_s_cmd[u8_i] = g_s_cmd_his[g_u8_history_pos][u8_i];
      if( ('A'<=g_s_cmd[u8_i]) && (g_s_cmd[u8_i]<='Z') )
         g_s_cmd[u8_i] += ('a'-'A');
   }
   g_s_cmd[u8_i]=0;
   u8_cmd_size = u8_i-1;

   // Get arguments strings
   for( u8_j=0; u8_j<USHELL_MAX_NB_ARG; u8_j++ )
   {
      u8_i++;     // Jump space character
      // Check "
      b_arg_include_space = ( g_s_cmd_his[g_u8_history_pos][u8_i] == '"' );
      if( b_arg_include_space ) {
        u8_i++;
      }
      for( u8_k=0;
           (b_arg_include_space || (g_s_cmd_his[g_u8_history_pos][u8_i] != ' '))
           && ((!b_arg_include_space) || (g_s_cmd_his[g_u8_history_pos][u8_i] != '"'))
           && (u8_i<=g_u8_cmd_size);
           u8_i++, u8_k++ )
      {
         g_s_arg[u8_j][u8_k] = g_s_cmd_his[g_u8_history_pos][u8_i];
      }
      if( b_arg_include_space ) {
        u8_i++;   // Jump last "
      }
      g_s_arg[u8_j][u8_k] = 0;
   }
           
   // Reset command size and update history       
   g_u8_cmd_size=0;
   g_u8_history_pos++;
   if( g_u8_history_pos == USHELL_HISTORY)
      g_u8_history_pos = 0;
   g_u8_history_pos_search = g_u8_history_pos;
   
   // Decode command type
   if ( mystrncmp(g_s_cmd,(U8 code *)str_disk,u8_cmd_size))
   {  cmd_type=CMD_NB_DRIVE; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_df,u8_cmd_size))
   {  cmd_type=CMD_DF; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_format,u8_cmd_size))
   {  cmd_type=CMD_FORMAT; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_mount,u8_cmd_size))
   {  cmd_type=CMD_MOUNT; }
   else if ( g_s_cmd[1]==':' )
   {  cmd_type=CMD_MOUNT; g_s_arg[0][0]=g_s_cmd[0];g_s_arg[0][1]='0'; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_space,u8_cmd_size))
   {  cmd_type=CMD_SPACE; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_ls,u8_cmd_size))
   {  cmd_type=CMD_LS; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_ls_more,u8_cmd_size))
   {  cmd_type=CMD_LS_MORE; }   
   else if (mystrncmp(g_s_cmd,(U8 code *)str_cd,u8_cmd_size))
   {  cmd_type=CMD_CD; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_up,u8_cmd_size))
   {  cmd_type=CMD_UP; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_cat,u8_cmd_size))
   {  cmd_type=CMD_CAT; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_cat_more,u8_cmd_size))
   {  cmd_type=CMD_CAT_MORE; }   
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_help,u8_cmd_size))
   {  cmd_type=CMD_HELP; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_mkdir,u8_cmd_size))
   {  cmd_type=CMD_MKDIR; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_touch,u8_cmd_size))
   {  cmd_type=CMD_TOUCH; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_rm,u8_cmd_size))
   {  cmd_type=CMD_RM; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_append,u8_cmd_size))
   {  cmd_type=CMD_APPEND; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_mark,u8_cmd_size))
   {  cmd_type=CMD_SET_ID; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_goto,u8_cmd_size))
   {  cmd_type=CMD_GOTO_ID; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_cp,u8_cmd_size))
   {  cmd_type=CMD_CP; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_mv,u8_cmd_size))
   {  cmd_type=CMD_MV; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_sync,u8_cmd_size))
   {  cmd_type=CMD_SYNC; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_perform,u8_cmd_size))
   {  cmd_type=CMD_PERFORM; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_reboot,u8_cmd_size))
   {  cmd_type=CMD_REBOOT; }
#if (USHELL_USB==ENABLE)
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_ls_usb,u8_cmd_size))
   {  cmd_type=CMD_LS_USB; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_usb_suspend,u8_cmd_size))
   {  cmd_type=CMD_USB_SUSPEND; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_usb_resume,u8_cmd_size))
   {  cmd_type=CMD_USB_RESUME; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_usb_force_enum,u8_cmd_size))
   {  cmd_type=CMD_USB_FORCE_ENUM; }
#endif
#if (USHELL_DFU==ENABLE)
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_dfu_erase,u8_cmd_size))
   {  cmd_type=CMD_DFU_ERASE; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_dfu_load,u8_cmd_size))
   {  cmd_type=CMD_DFU_LOAD; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_dfu_start,u8_cmd_size))
   {  cmd_type=CMD_DFU_START; }
#endif
#if (USHELL_HID==ENABLE)
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_hid_enter_dfu,u8_cmd_size))
   {  cmd_type=CMD_HID_ENTER_DFU; }
   else if ( mystrncmp(g_s_cmd,(U8 code *)str_hid_get_info,u8_cmd_size))
   {  cmd_type=CMD_HID_GET_INFO; }
#endif
   else
   {
      print_msg((U8 code *)msg_er_cmd_not_found);
      print_msg((U8 code *)msg_prompt);
      return CMD_NONE;
   }
   return cmd_type;
}


//! @brief Cleans the command line on the display
//!
void ushell_clean_cmd_line( void )
{
   // Clean command line display
   while( 0 != g_u8_cmd_size )
   {
      // Remove the last character on cmd line buffer
      putchar(ASCII_BKSPACE); // Send a backspace to go in previous character
      putchar(' ');           // Send a space to erase previous character
      putchar(ASCII_BKSPACE); // Send a backspace to go in new end position (=previous character position)
      g_u8_cmd_size--;
   }
}


//! @brief Selects the previous command in history list
//!
void ushell_history_up( void )
{
   if( g_u8_history_pos_search == 0 )
   {
      if( (USHELL_HISTORY-1) == g_u8_history_pos )
         return;  // End of history list
      g_u8_history_pos_search = USHELL_HISTORY-1;
   }else{
      if( (g_u8_history_pos_search-1) == g_u8_history_pos )
         return;  // End of history list
      g_u8_history_pos_search--;
   }
   if( 0 == g_s_cmd_his[g_u8_history_pos_search][0] )
   {
      // History empty then go to previous selection
      ushell_history_down();
   }
}


//! @brief Selects the next command in history list
//!
void ushell_history_down( void )
{
   if( g_u8_history_pos_search == g_u8_history_pos )
      return;  // End of history list
   if( g_u8_history_pos == 0 )
   {
      if( (USHELL_HISTORY-1) == g_u8_history_pos_search )
         return;  // End of history list
      g_u8_history_pos_search++;
   }else{
      if( (g_u8_history_pos_search+1) == g_u8_history_pos )
         return;  // End of history list
   }
   g_u8_history_pos_search++;
   if( USHELL_HISTORY == g_u8_history_pos_search )
      g_u8_history_pos_search = 0;
}
           

//! @brief Displays the current history
//!
void ushell_history_display( void )
{
   g_u8_cmd_size=0;
   while( g_s_cmd_his[g_u8_history_pos_search][g_u8_cmd_size] != 0 )
   {
      putchar( g_s_cmd_his[g_u8_history_pos_search][g_u8_cmd_size] );
      g_s_cmd_his[g_u8_history_pos][g_u8_cmd_size] = g_s_cmd_his[g_u8_history_pos_search][g_u8_cmd_size];
      g_u8_cmd_size++;
   }
   g_s_cmd_his[g_u8_history_pos][g_u8_cmd_size] = 0;
}           

   

//! @brief compares two strings located in flash code area.
//!
//! @param *str1
//! @param *str2
//!
//! @return status: TRUE for ok, FALSE if strings are not equal
#ifdef __GNUC__
U8 mystrncmp(char *str1,U8 *str2,U8 u8_i)
#else
U8 mystrncmp(char *str1,U8 code *str2,U8 u8_i)
#endif
{
   U8 j;
   for(j=0;j<=u8_i;j++)
   {
#ifndef __GNUC__
      if(*str1!=*str2)
#else
      if( *str1 != pgm_read_byte_near((unsigned int)str2))
#endif
      {
         return FALSE;
      }
      str1++;str2++;
   }
   return TRUE;
}


//! @brief Display an ASCII code string.
//!
//! @param *str: pointer to string located in flash area
//!
#ifdef __GNUC__
void print_msg(U8 *str)
#else
void print_msg(U8 code *str)
#endif
{
   char c;
#ifndef __GNUC__
   c=*str++;
   while(c!=0)
   {
      ushell_putchar(c);
      c=*str++;
   }
#else    // AVRGCC does not support point to PGM space
   c=pgm_read_byte_near((unsigned int)str++);
   while(c!=0)
   {
      ushell_putchar(c);
      c=pgm_read_byte_near((unsigned int)str++);
   }
#endif

}


//! @brief This function wait a key press
//!
//! @return TRUE, if the action must be continue
//!
Bool ushell_more_wait( void )
{
   char c_key;
   printf("\n\r-- space for more--"); 
   c_key=0;
   while( (c_key!='q') && (c_key!=' ') )
   {
     c_key=ushell_get_char();
   }
   printf("\r                 \r");
   return (c_key==' ');
}


//! @brief This function display all drives present
//!
void ushell_cmd_nb_drive( void )
{
   U8 u8_tmp;
   
   printf("Memory interface available:\r\n");
   for( u8_tmp=0; u8_tmp<nav_drive_nb(); u8_tmp++ )
   {
      // Display drive letter name (a, b...)
      ushell_putchar(u8_tmp+'a');
      ushell_putchar(':');
      ushell_putchar(' ');
      print_msg((U8 code *)mem_name(u8_tmp));
      ushell_putchar(ASCII_CR); ushell_putchar(ASCII_LF);
   }
}


//! @brief This function displays the free space of each drive present
//!
void ushell_cmd_free_space( void )
{
   U8 u8_tmp;
   Fs_index sav_index = nav_getindex();      // Save current position
   for( u8_tmp=0; u8_tmp<nav_drive_nb(); u8_tmp++ )
   {
      nav_drive_set( u8_tmp );      // Select drive
      if( !nav_partition_mount() )  // Mount drive
         continue;
      
      // Display drive letter name (a, b...)
      ushell_putchar( u8_tmp+'a' );
      ushell_putchar(':');
      ushell_putchar(' ');
      print_msg((U8 code *)mem_name(u8_tmp));
      printf("\n\r");
      
      if( g_s_arg[0][0]=='l' )        // Choose command option
      {
         // Long and exact fonction
         printf("Free space: %lu KBytes / %lu KBytes\n\r", 
                   ((unsigned long)nav_partition_freespace()>>1),
                   ((unsigned long)nav_partition_space()>>1));
      }
      else                    
      {
         // Otherwise use fast command
         printf("Free space: %u %%\n\r", nav_partition_freespace_percent() );
      }
   }
   nav_gotoindex(&sav_index);       // Restore position
}


//! @brief This function formats a drive
//!
void ushell_cmd_format( void )
{
   if( g_s_arg[0][0] == 0 )
      return;
   
   // Select drive to format
   nav_drive_set( g_s_arg[0][0]-'a');
   if( !nav_drive_format(FS_FORMAT_DEFAULT) )
   {
      print_msg((U8 code *)msg_er_format);
      return;
   }
}


//! @brief This function mount a drive
//!
void ushell_cmd_mount( void )
{
   U8 u8_drive_lun;
   Fs_index sav_index;

   if( g_s_arg[0][0] == 0 )
      return;
   
   // Compute the logical unit number of drive
   u8_drive_lun=g_s_arg[0][0]-'a';
   // Check lun number
   if( u8_drive_lun >= nav_drive_nb() )
   {
      print_msg((U8 code *)msg_er_drive);
      return;
   }

   // Mount drive
   sav_index = nav_getindex();      // Save previous position
   if( nav_drive_set(u8_drive_lun))
   {
      if( nav_partition_mount() )
         return;                    // Here, drive mounted
   }
   print_msg((U8 code *)msg_er_mount);
   nav_gotoindex(&sav_index);       // Restore previous position
}


//! @brief This function displays the disk space of current drive
//!
void ushell_cmd_space( void )
{
   U32 u32_space;
   // Display drive letter name (a, b...)
   print_msg((U8 code *)mem_name(nav_drive_get()));
   ushell_putchar(' ');
   ushell_putchar( nav_drive_get()+'a');
   // Otherwise use fast command
   u32_space = nav_partition_space();
   if( 1024 >(u32_space % (2*1024)) )
   {
      u32_space = u32_space/(2*1024);
   }else{
      u32_space = (u32_space/(2*1024))+1;
   }
   printf(": space: %luMB \n\r", u32_space );
}


//! @brief This function manages the ls command
//!
//! @param more: enable the '|more' management when TRUE otherwize no '|more' management
//!
void ushell_cmd_ls( Bool b_more )
{
   U8 str_char[MAX_FILE_LENGHT];
   U16 u16_i,u16_nb_file,u16_nb_dir,last_i;
   U8 ext_filter=FALSE;

   //** Print drive name
   printf("%c: volume is ", 'a'+nav_drive_get() );
   print_msg((U8 code *)mem_name(nav_drive_get()));
   printf("\n\rDrive uses ");
   switch (nav_partition_type())
   {
      case FS_TYPE_FAT_12:
      printf("FAT12\n\r");
      break;
      
      case FS_TYPE_FAT_16:
      printf("FAT16\n\r");
      break;
      
      case FS_TYPE_FAT_32:
      printf("FAT32\n\r");
      break;
      
      default:
      printf("an unknown partition type\r\n");
      return;
   }
   
   //** Print directory name
   if( !nav_dir_name( (FS_STRING)str_char, MAX_FILE_LENGHT ) )
   {
      printf("ROOT directory\n\r");
   }else{
      printf("Dir name is %s\n\r",str_char);
   }

   //** Check extension filter in extra parameters
   if(g_s_arg[0][0]!=0)
   {
      if(g_s_arg[0][0] == '*' && g_s_arg[0][1]=='.')
      {
         ext_filter=TRUE;
         for(u16_i=2; u16_i<USHELL_SIZE_CMD_LINE; u16_i++)
         {
            g_s_arg[0][u16_i-2]=g_s_arg[0][u16_i];
         }
      }
   }
   
   //** Print files list
   printf("          Size  Name\n\r");
   // Init loop at the begining of directory
   nav_filelist_reset();
   u16_nb_file=0;
   u16_nb_dir=0;
   last_i=0;
   // For each file in list
   while( nav_filelist_set(0,FS_FIND_NEXT) )
   {  
      if(!ext_filter)
      {
         // No extension filter
         if( nav_file_isdir() )
         {
            printf("Dir ");
            u16_nb_dir++;              // count the number of directory
         }else{
            printf("    ");
         }
      }
      else
      {
         // If extension filter then ignore directories
         if(nav_file_isdir())
            continue;
         // Check extension
         if(!nav_file_checkext((FS_STRING)g_s_arg[0]))
            continue;
      }
      u16_nb_file++;                   // count the total of files (directories and files)

      // Check 'more' step
      if( b_more && ((u16_nb_file%USHELL_NB_LINE)==0) && (u16_nb_file!=0) && (last_i != u16_nb_file) )
      {
         last_i=u16_nb_file;
         if( !ushell_more_wait() )
            return;  // Exit LS command
      }
      
      // Display file
      nav_file_name((FS_STRING)str_char, MAX_FILE_LENGHT, FS_NAME_GET, TRUE);
      printf("%10lu  %s\n\r", nav_file_lgt(), str_char);
   }
   // Display total number
   printf(" %4i Files\r\n", u16_nb_file-u16_nb_dir );
   printf(" %4i Dir\r\n", u16_nb_dir );
}


//! @brief This function enter in a directory
//!
void ushell_cmd_cd( void )
{ 
   if( g_s_arg[0][0] == 0 )
      return;
   
   // Add '\' at the end of path, else the nav_setcwd select the directory but don't enter into.
   ushell_path_valid_syntac( g_s_arg[0] );
   
   // Call file system routine
   if( nav_setcwd((FS_STRING)g_s_arg[0],TRUE,FALSE) == FALSE )
   {
      print_msg((U8 code *)msg_er_unknown_file);
   }
}


//! @brief This function go back to parent directory
//!
void ushell_cmd_gotoparent( void )
{
   nav_dir_gotoparent();
}


//! @brief Manage cat command
//!
//! @param more: enable the '|more' management
//!
//! @todo more management not fully functionnal with file without CR
//!
void ushell_cmd_cat( Bool b_more)
{
   char c_file_character;
   U8 n_line=0;
   
   if( g_s_arg[0][0] == 0 )
      return;
   
   // Select file
   if( !nav_setcwd((FS_STRING)g_s_arg[0],TRUE,FALSE) )
   {
      print_msg((U8 code *)msg_er_unknown_file);
      return;
   }
     
   // Open file
   file_open(FOPEN_MODE_R);
   while (file_eof()==FALSE)
   {
      // Check 'more' option
      if( b_more && (n_line >= USHELL_NB_LINE))
      {
         n_line = 0;
         if( !ushell_more_wait() )
            break;   // Stop cat command
      }

      // Display a character
      c_file_character = file_getc();
      ushell_putchar( c_file_character );

      // Count the line number
      if (c_file_character==ASCII_LF)
         n_line++;
   }
   file_close();

   // Jump in a new line
   ushell_putchar(ASCII_CR);ushell_putchar(ASCII_LF);
}


//! @brief This function display the help
//!
void ushell_cmd_help( void )
{
   print_msg((U8 code *)msg_help);
}


//! @brief This function create a directory
//!
void ushell_cmd_mkdir( void )
{
   if( g_s_arg[0][0] == 0 )
      return;
   
   if( !nav_dir_make((FS_STRING)g_s_arg[0]) )
      print_msg((U8 code *)msg_ko);      
}


//! @brief This function create a file
//!
void ushell_cmd_touch( void )
{
   if( g_s_arg[0][0] == 0 )
      return;
   
   nav_file_create((FS_STRING)g_s_arg[0]);
}


//! @brief This function delete a file or directory
//!
void ushell_cmd_rm( void )
{
   U8 u8_i = 0;
   Fs_index sav_index;

   if( g_s_arg[0][0] == 0 )
      return;
   
   // Save the position
   sav_index = nav_getindex();
   
   while( 1 )
   {
      // Restore the position
      nav_gotoindex(&sav_index);
      // Select file or directory
      if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
         break;
      // Delete file or directory
      if( !nav_file_del( FALSE ) )
      {
         print_msg((U8 code *)msg_ko);      
         break;
      }
      u8_i++;
   }
   printf( "%u file(s) deleted\n\r", u8_i );
}


//! @brief Minimalist file editor to append char to a file
//!
//! @verbatim
//! hit ^q to exit and save file
//! @endverbatim
//!
void ushell_cmd_append_file( void )
{
   char c_key;
  
   if( g_s_arg[0][0] == 0 )
      return;
   
   // Select file or directory
   if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
   {
      print_msg((U8 code *)msg_er_unknown_file);
      return;
   }
   // Open file
   if( !file_open(FOPEN_MODE_APPEND) )
   {
      print_msg((U8 code *)msg_ko);
      return;
   }

   // Append file
   print_msg((U8 code *)msg_append_welcome);
   while( 1 )
   {
      c_key = ushell_get_char();

      if( c_key == ASCII_CTRL_Q )
         break;   // ^q to quit
      
      ushell_putchar( c_key );
      file_putc( c_key );
      if( c_key == ASCII_CR )
      {
         ushell_putchar(ASCII_LF);
         file_putc(ASCII_LF);
      }
   }

   // Close file
   file_close();
   ushell_putchar(ASCII_CR); ushell_putchar(ASCII_LF);
}


//! @brief This function copys a file to other location
//!
void ushell_cmd_copy( void )
{
   Fs_index sav_index;
   U8 u8_status_copy;

   if( g_s_arg[0][0] == 0 )
      return;
   
   // Save the position
   sav_index = nav_getindex();
   
   // Select source file
   if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
   {
      print_msg((U8 code *)msg_er_unknown_file);
      return;
   }
   // Get name of source to be used as same destination name
   nav_file_name( (FS_STRING)g_s_arg[0], MAX_FILE_LENGHT, FS_NAME_GET, TRUE );              
   // Mark this selected file like source file
   if( !nav_file_copy())
   {
      print_msg((U8 code *)msg_ko);
      goto cp_end;
   }

   // Select destination
   if( g_s_arg[1][0]==0 )
   {
      // g_s_arg[1] is NULL, using mark
      if( !nav_gotoindex(&g_mark_index) )
         goto cp_end;
   }
   else
   {
      // g_s_arg[1] exists, then go to this destination
      if( !nav_setcwd( (FS_STRING)g_s_arg[1], TRUE, FALSE ) )
      {
         print_msg((U8 code *)msg_er_unknown_file);
         goto cp_end;
      }
   }
   
   // Set the name destination and start paste
   if( !nav_file_paste_start((FS_STRING)g_s_arg[0]) )
   {
      print_msg((U8 code *)msg_paste_fail);
      goto cp_end;
   }
   
   // Performs copy
   do
   {
      u8_status_copy = nav_file_paste_state( FALSE );
   }while( u8_status_copy == COPY_BUSY );

   // Check status of copy action
   if( u8_status_copy == COPY_FAIL )
   {
      print_msg((U8 code *)msg_paste_fail);
      goto cp_end;
   }

cp_end:
   // Restore the position
   nav_gotoindex(&sav_index);
}


//! @brief This function renames a file or a directory
//!
void ushell_cmd_rename( void )
{
   if( g_s_arg[0][0] == 0 )
      return;
   if( g_s_arg[1][0] == 0 )
      return;
   
   // Select source file
   if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
   {
      print_msg((U8 code *)msg_er_unknown_file);
      return;
   }
   // Rename file or directory
   if( !nav_file_rename( (FS_STRING)g_s_arg[1] ) )
   {
      print_msg((U8 code *)msg_ko);
      return;
   }
}


//! @brief Synchronize a path with an other path
//!
//! @return TRUE if success
//!
Bool ushell_cmd_sync( void )
{
   Fs_index sav_index;
   U8 u8_folder_level = 0;

   if( g_s_arg[0][0] == 0 )
      return FALSE;
   if( g_s_arg[1][0] == 0 )
      return FALSE;
   // Add '\' at the end of path, else the nav_setcwd select the directory but don't enter into.
   ushell_path_valid_syntac( g_s_arg[0] );
   ushell_path_valid_syntac( g_s_arg[1] );
   
   printf("Synchronize folders:\n\r");
   sav_index = nav_getindex();   // Save the position
   
   // Select source directory in COPYFILE navigator handle
   nav_select( FS_NAV_ID_COPYFILE );
   printf("Select source directory\n\r");
   if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
      goto ushell_cmd_sync_error;
   nav_filelist_reset();

   // Select destination directory in USHELL navigator handle
   nav_select( FS_NAV_ID_USHELL_CMD );
   printf("Select destination directory\n\r");
   if( !nav_setcwd( (FS_STRING)g_s_arg[1], TRUE, TRUE ) )
      goto ushell_cmd_sync_error;
   nav_filelist_reset();

   // loop to scan and create ALL folders and files
   while(1)
   {
      while(1)
      {
         // Loop to Search files or directories
         // Reselect Source
         nav_select( FS_NAV_ID_COPYFILE );
         if( nav_filelist_set( 0 , FS_FIND_NEXT ) )
            break;   // a next file and directory is found
   
         // No other dir or file in current dir then go to parent dir on Source and Destination disk
         if( 0 == u8_folder_level )
         {
            // end of update folder
            //********* END OF COPY **************
            goto ushell_cmd_sync_finish;
         }

         printf("Go to parent\n\r");
         // Remark, nav_dir_gotoparent() routine go to in parent dir and select the children dir in list
         u8_folder_level--;
         if( !nav_dir_gotoparent() )
            goto ushell_cmd_sync_error;
         // Select Destination navigator and go to the same dir of Source
         nav_select( FS_NAV_ID_USHELL_CMD );
         if( !nav_dir_gotoparent() )
            goto ushell_cmd_sync_error;
      } // end of while (1)
      
      if( nav_file_isdir())
      {
         printf("Dir found - create dir: ");
         //** here, a new directory is found and is selected
         // Get name of current selection (= dir name on Source)
         if( !nav_file_name( (FS_STRING)g_s_arg[0], USHELL_SIZE_CMD_LINE, FS_NAME_GET, FALSE ))
            goto ushell_cmd_sync_error;
         // Enter in dir (on Source)
         if( !nav_dir_cd())
            goto ushell_cmd_sync_error;
         u8_folder_level++;
         // Select Destination disk
         nav_select( FS_NAV_ID_USHELL_CMD );
         // Create folder in Destination disk
         printf((char*)g_s_arg[0]);
         printf("\n\r");
         if( !nav_dir_make( (FS_STRING )g_s_arg[0] ))
         {
            if( FS_ERR_FILE_EXIST != fs_g_status )
               goto ushell_cmd_sync_error;
            // here, error the name exist
         }
         // Here the navigator have selected the folder on Destination
         if( !nav_dir_cd())
         {
            if( FS_ERR_NO_DIR == fs_g_status )
            {
               // FYC -> Copy impossible, because a file have the same name of folder
            }
            goto ushell_cmd_sync_error;
         }
         // here, the folder is created and the navigatorS is entered in this dir
      }
      else
      {
         printf("File found - copy file: ");
         //** here, a new file is found and is selected
         // Get name of current selection (= file name on Source)
         if( !nav_file_name( (FS_STRING)g_s_arg[0], USHELL_SIZE_CMD_LINE, FS_NAME_GET, FALSE ))
            goto ushell_cmd_sync_error;
         printf((char*)g_s_arg[0]);
         printf("\n\r");
         if( !nav_file_copy())
            goto ushell_cmd_sync_error;

         // Paste file in current dir of Destination disk
         nav_select( FS_NAV_ID_USHELL_CMD );
         while( !nav_file_paste_start( (FS_STRING)g_s_arg[0] ) )
         {
            // Error
            if( fs_g_status != FS_ERR_FILE_EXIST )
               goto ushell_cmd_sync_error;
            // File exists then deletes this one
            printf("File exists then deletes this one.\n\r");
            if( !nav_file_del( TRUE ) )
               goto ushell_cmd_sync_error;
            // here, retry PASTE                   
         }
         // Copy running
         {
         U8 status;
         do{
            status = nav_file_paste_state(FALSE);
         }while( COPY_BUSY == status );

         if( COPY_FINISH != status )
            goto ushell_cmd_sync_error;
         }
      } // if dir OR file
   } // end of first while(1)
 
ushell_cmd_sync_error:
   // Restore the position
   nav_select( FS_NAV_ID_USHELL_CMD );
   nav_gotoindex(&sav_index);
   printf("!!!Copy fail\n\r");
   return FALSE;
   
ushell_cmd_sync_finish:
   // Restore the position
   nav_select( FS_NAV_ID_USHELL_CMD );
   nav_gotoindex(&sav_index);
   printf("End of copy\n\r");
   return TRUE;
}

// File alloc space (unit sector 512B)
#define  FILE_ALLOC_SIZE      ((1024*1024L)/512L)      // 1MB

Fs_file_segment ushell_cmd_perform_alloc( U8 lun, U16 size_alloc )
{
   const FS_STRING file_tmp_name = "tmp.bin";
   Fs_file_segment g_recorder_seg;   
   g_recorder_seg.u16_size = 0;   

   if( !nav_drive_set(lun))
      return g_recorder_seg;

   if( !nav_partition_mount() )
      return g_recorder_seg;

   if( !nav_file_create((FS_STRING)file_tmp_name))
   {
      if( FS_ERR_FILE_EXIST != fs_g_status)
         return g_recorder_seg;
      nav_file_del(FALSE);
      if( !nav_file_create((FS_STRING)file_tmp_name))
         return g_recorder_seg;
   }
   // Open file
   if( !file_open(FOPEN_MODE_W) )
   {
      nav_file_del(FALSE);
      return g_recorder_seg;
   }
   // Define the size of segment to alloc (unit 512B)
   // Note: you can alloc more in case of you don't know total size
   g_recorder_seg.u16_size = size_alloc;   
   // Alloc in FAT a cluster list equal or inferior at segment size
   if( !file_write( &g_recorder_seg ))
   {
      g_recorder_seg.u16_size = 0;   
      file_close();
      nav_file_del(FALSE);
   }
   return g_recorder_seg;   //** File open and FAT allocated
}

void ushell_cmd_perform_transfer( Fs_file_segment seg_src, Fs_file_segment seg_dest )
{
   U8 id_trans_memtomem;
   Ctrl_status status_stream;
   U16 u16_i, u16_ctn, u16_trans_max;
   U32 u32_tmp;
   
   Timer16_set_waveform_mode(TIMER16_COMP_MODE_NORMAL);
   Timer16_set_clock(TIMER16_CLKIO_BY_1024); // 8MHz / 1024
   u16_trans_max = ( seg_src.u16_size < seg_dest.u16_size )?  seg_src.u16_size : seg_dest.u16_size;
   for( u16_ctn=0,u16_i=2; u16_i<=u16_trans_max; u16_i*=10 )
   {
      Timer16_clear_overflow_it();
      Timer16_set_counter(0);
      id_trans_memtomem = stream_mem_to_mem( seg_src.u8_lun , seg_src.u32_addr , seg_dest.u8_lun , seg_dest.u32_addr , u16_i );
      if( ID_STREAM_ERR == id_trans_memtomem )
      {
         printf( "Transfert error\r\n");
         return;
      }
      while(1)
      {
         status_stream = stream_state( id_trans_memtomem );
         if( CTRL_BUSY == status_stream ) continue;
         if( CTRL_GOOD == status_stream ) break;
         if( CTRL_FAIL == status_stream ) {
            printf( "Transfert error\r\n");
            return;
         }
      }
      u16_ctn = Timer16_get_counter();
      if( Timer16_get_overflow_it() )
      {
         // Counter too small to get time
         if( 1 == u16_i )
            printf( "Transfert too slow\r\n");
         break;
      }
      u32_tmp = ((U32)u16_i*1000)/((2*(U32)u16_ctn*1024)/8000);
      printf( "Transfert rate %4luKB/s - stream size %4iKB\r\n", u32_tmp, u16_i/2 );
   }
}

void ushell_cmd_perform_access( Bool b_sens_write, Fs_file_segment seg )
{
   U32 u32_tmp;
   U16 u16_trans,u16_ctn;
   
   fat_cache_flush();
   fat_cache_reset();
   Timer16_set_waveform_mode(TIMER16_COMP_MODE_NORMAL);
   Timer16_set_clock(TIMER16_CLKIO_BY_1024); // 8MHz / 1024
   Timer16_clear_overflow_it();
   Timer16_set_counter(0);
   for( u16_ctn=0,u16_trans=0; u16_trans<seg.u16_size; u16_trans++ )
   {         
      if( b_sens_write )
      {
         if( CTRL_GOOD != ram_2_memory( seg.u8_lun , seg.u32_addr , fs_g_sector )) {
            printf( "Transfert error\r\n");
            return;
         }
      }else{
         if( CTRL_GOOD != memory_2_ram( seg.u8_lun , seg.u32_addr , fs_g_sector )) {
            printf( "Transfert error\r\n");
            return;
         }
      }
      seg.u32_addr++;
      if( Timer16_get_overflow_it() )
         break;
      u16_ctn = Timer16_get_counter();
   }
   u32_tmp = ((U32)u16_trans*1000)/((2*(U32)u16_ctn*1024)/8000);
   if( b_sens_write )
      printf( "Transfert rate - WRITE %4luKB/s\r\n", u32_tmp );
   else
      printf( "Transfert rate - READ %4luKB/s\r\n", u32_tmp );
}

#ifndef USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD
#warning Define USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD in config.h file
#endif

static   U8  u8_ext_buffer[512*USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD];

void ushell_cmd_perform_extaccess( Bool b_sens_write, Fs_file_segment seg )
{
   U16 u16_trans,u16_ctn;
   U32 u32_tmp;
   U8  u8_nb_trans_usb=0;
   
   fat_cache_flush();
   fat_cache_reset();

   Timer16_set_waveform_mode(TIMER16_COMP_MODE_NORMAL);
   Timer16_set_clock(TIMER16_CLKIO_BY_1024); // 8MHz / 1024
   Timer16_clear_overflow_it();
   Timer16_set_counter(0);

   u16_trans=u16_ctn=0;
   while( seg.u16_size!=0 )
   {         
      if( 0 == (seg.u32_addr % USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD) )
      {
         u8_nb_trans_usb = USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD;
      }else{
         u8_nb_trans_usb = USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD - (seg.u32_addr % USHELL_SIZE_OF_EXT_BUFFER_TO_PERF_CMD);  // to align access with usual memory mapping
      }
      if (u8_nb_trans_usb > seg.u16_size)
        u8_nb_trans_usb = seg.u16_size;
      
      if( b_sens_write )
      {
         if( CTRL_GOOD != host_mem_ram_2_mem_ext( seg.u8_lun-LUN_ID_USB, seg.u32_addr , u8_ext_buffer, u8_nb_trans_usb )) {
            printf( "Transfert error\r\n");
            return;
         }
      }else{        
         if( CTRL_GOOD != host_mem_mem_2_ram_ext( seg.u8_lun-LUN_ID_USB, seg.u32_addr , u8_ext_buffer, u8_nb_trans_usb )) {
            printf( "Transfert error\r\n");
            return;
         }
      }
      
      if( Timer16_get_overflow_it() )
      {
         break;
      }

      u16_ctn = Timer16_get_counter();
      seg.u16_size -= u8_nb_trans_usb;
      u16_trans    += u8_nb_trans_usb;
      seg.u32_addr += u8_nb_trans_usb;
   }
   u32_tmp = ((U32)u16_trans*1000)/((2*(U32)u16_ctn*1024)/8000);
   if( b_sens_write )
      printf( "Transfert rate - WRITE %4luKB/s\r\n", u32_tmp);
   else
      printf( "Transfert rate - READ %4luKB/s\r\n", u32_tmp );
}


//! @brief Perform transfer between two devices
//!
void ushell_cmd_perform( void )
{
   Fs_index sav_index;
   Fs_file_segment seg1, seg2;

   if( g_s_arg[0][0] == 0 )
      return;
   
   sav_index = nav_getindex();   // Save the position
   
   // Alloc a file on each devices
   printf("Alloc a file on each devices\n\r");
   seg1 = ushell_cmd_perform_alloc( (g_s_arg[0][0]-'a') , FILE_ALLOC_SIZE );
   if( seg1.u16_size == 0 )
   {
      printf("!!!Error allocation on device 1\n\r");
      // Restore the position
      nav_gotoindex(&sav_index);
      return;
   }
   if( g_s_arg[1][0] != 0 )
   {
      nav_select( FS_NAV_ID_COPYFILE );
      seg2 = ushell_cmd_perform_alloc( (g_s_arg[1][0]-'a') , FILE_ALLOC_SIZE );
      if( seg2.u16_size == 0 )
      {
         nav_select( FS_NAV_ID_USHELL_CMD );   
         file_close();
         nav_file_del(FALSE);
         printf("!!!Error allocation on device 2\n\r");
         // Restore the position
         nav_gotoindex(&sav_index);
         return;
      }   
      
      // Transfert data from device 1 to device 2
      printf("Transfert data from device 1 to device 2\r\n");
      ushell_cmd_perform_transfer(seg1,seg2);
      printf("Transfert data from device 2 to device 1\r\n");
      ushell_cmd_perform_transfer(seg2,seg1);
      // Delete files allocated
      nav_select( FS_NAV_ID_COPYFILE );   
      file_close();
      nav_file_del(FALSE);
      nav_select( FS_NAV_ID_USHELL_CMD );
   }
   else
   {
      ushell_cmd_perform_access( FALSE, seg1 );
      ushell_cmd_perform_access( TRUE, seg1 );
      if( LUN_ID_USB <= nav_drive_get() )
      {
         printf("Transfert large buffer on USB\r\n");
         ushell_cmd_perform_extaccess( FALSE, seg1 );
         ushell_cmd_perform_extaccess( TRUE, seg1 );
      }
      
   }
   
   file_close();
   nav_file_del(FALSE);
   // Restore the position
   nav_gotoindex(&sav_index);
   printf("End of test\n\r");
   return;   
}




//! @brief Appends the '\' char at the end of path
//!
void ushell_path_valid_syntac( char *path )
{
   U8 u8_tmp;
   
   // Compute size of substitute
   for( u8_tmp=0; u8_tmp<MAX_FILE_LENGHT; u8_tmp++ )
   {
      if( path[u8_tmp]==0)
         break;
   }
   // Append the '\' char for the nav_setcwd to enter the found directory
   if ( path[u8_tmp-1] != '\\')
   {
      path[u8_tmp]='\\';
      path[u8_tmp+1]=0;
   }
}


//! @brief This function reboot the target
//!
void ushell_cmd_reboot( void )
{
   // Reset SPI interface
#ifdef SPIDER_ADC_BRIDGE
   #if (TARGET_BOARD==SPIDER)
   SPI_InterMCU_Ready();
   Spider_bridge_reset();
   SPI_InterMCU_Suspend();
   #endif
#endif
   // Enable watch dog
   wdtdrv_enable(WDTO_16MS);
   // Wait watch dog event
   while(1);
}


#if (USHELL_USB==ENABLE)
//! @brief In host mode, display basic lowlevel information about the device connected
//!
//! @note The device should be supported by the host (configured)
//!
void ushell_cmdusb_ls(void)
{
   U8 i,j,n,s;

   // Check USB host status
   if( (!Is_host_ready()) && (!Is_host_suspended()) )
   {
      print_msg((U8 code *)msg_no_device);
      return;
   }
   if( Is_host_suspended() )
   {
      print_msg((U8 code *)msg_usb_suspended);
   }
   
   s = selected_device;
   for( n=0; n<Get_nb_device(); n++ )
   {
      Host_select_device(n);
      printf("\n\rDevice %i @:0x%02X\n\r",n+1,usb_tree.device[selected_device].device_address);
      printf("VID:%04X, PID:%04X, ",Get_VID(),Get_PID());
      printf("MaxPower is %imA, ",2*Get_maxpower());
      if (Is_device_self_powered())
      {  print_msg((U8 code *)msg_device_self_powered); }
      else
      {  print_msg((U8 code *)msg_device_bus_powered); }
      printf("Control Endpoint is %i bytes, ",Get_ep0_size()); 
      if (Is_host_full_speed())
      {  print_msg((U8 code *)msg_device_full_speed); }
      else
      {  print_msg((U8 code *)msg_device_low_speed); }
      if (Is_device_supports_remote_wakeup())
      {  print_msg((U8 code *)msg_remote_wake_up_ok); }
      else
      {  print_msg((U8 code *)msg_remote_wake_up_ko); }
      printf("Supported interface(s):%02i\n\r",Get_nb_supported_interface());
      for(i=0;i<Get_nb_supported_interface();i++)
      {
         printf("Interface nb:%02i, AltS nb:%02i, Class:%02i, SubClass:%02i, Protocol:%02i\n\r",\
            Get_interface_number(i), Get_alts_s(i), Get_class(i), Get_subclass(i), Get_protocol(i));
         printf(" Endpoint(s) Addr:");
         if(Get_nb_ep(i))
         {
            for(j=0;j<Get_nb_ep(i);j++)
            {
               printf(" %02X", Get_ep_addr(i,j));
            }
         }
         else
         {
            printf("None");
         }
         ushell_putchar(ASCII_CR);ushell_putchar(ASCII_LF);
         printf(" Physical pipe(s):");
         if(Get_nb_ep(i))
         {
            for(j=0;j<Get_nb_ep(i);j++)
            {
               printf(" %02X", usb_tree.device[selected_device].interface[i].ep[j].pipe_number);
            }
         }
         else
         {
            printf("None");
         }
         ushell_putchar(ASCII_CR);ushell_putchar(ASCII_LF);         
      }
   }
   selected_device=s;
}

//! @brief In host mode, set host in suspend mode
//!
void ushell_cmdusb_suspend(void)
{
   if( !Is_host_ready() )
   {
      print_msg((U8 code *)msg_no_device);
   }
   Host_request_suspend();
}

//! @brief In host mode, resume host from suspend mode
//!
void ushell_cmdusb_resume(void)
{
   if( !Is_host_suspended() )
   {
      print_msg((U8 code *)msg_no_device);
   }
   Host_request_resume();
}


//! @brief In host mode, set host in suspend mode
//!
void ushell_cmdusb_force_enum(void)
{
   if( !Is_host_ready() )
   {
      print_msg((U8 code *)msg_no_device);
   }
   Host_force_enumeration();
}
#endif //USHELL_USB==ENABLE


#if (USHELL_DFU==ENABLE)
//! @brief This funtion erases the target connected at DFU
//!
void ushell_cmddfu_erase( void )
{
   if( !Is_dfu_connected() )
   {
      print_msg((U8 code *)msg_no_device);
      return;
   }
   Dfu_erase();
}

//! @brief This funtion loads the target connected at DFU
//!
void ushell_cmddfu_load( void )
{
   if( !Is_dfu_connected() )
   {
      print_msg((U8 code *)msg_no_device);
      return;
   }
   // Select source file
   if( !nav_setcwd( (FS_STRING)g_s_arg[0], TRUE, FALSE ) )
   {
      print_msg((U8 code *)msg_er_unknown_file);
      return;
   }
   Dfu_erase();
   if( !dfu_load_hex() )
   {
      print_msg((U8 code *)msg_ko);
      return;
   }
   print_msg((U8 code *)msg_ok);
}

//! @brief This funtion starts the target connected at DFU
//!
void ushell_cmddfu_start( void )
{
   if( !Is_dfu_connected() )
   {
      print_msg((U8 code *)msg_no_device);
      return;
   }
   Dfu_start_appli();
}
#endif   // (USHELL_DFU==ENABLE)


#if (USHELL_HID==ENABLE)
//! @brief This funtion enters in DFU mode the target connected at HID
//!
void ushell_cmdhid_enter_dfu(void)
{
   if( !Is_hid_connected() )
   {
      print_msg((U8 code *)msg_no_device);
   }
   Hid_send_enter_dfu();
}

//! @brief This funtion gets information about the target connected at HID
//!
void ushell_cmdhid_getinfo(void)
{
   if( !Is_hid_connected() )
   {
      print_msg((U8 code *)msg_no_device);
      return;
   }         
#if( HID_GENERIC_DEMO_FULL == ENABLE )         
   printf("Temperature %i C\n\r",Hid_get_temperature());
   printf("Potentiometer %i\n\r",Hid_get_potentiometer());
#else
   printf("No data available with this HID generic configuration.\n\r");
#endif
}
#endif // (USHELL_HID==ENABLE)


#ifdef   LOG_STR_CODE
//! @brief Display traces in ushell display
//!
//! @param str: pointer to on-chip code flash area
//!
extern void ushell_trace_msg(U8 code *str)
{
   ushell_putchar(CR);
   ushell_putchar(LF);
   print_msg((U8 code *)str);
   ushell_putchar(CR);
   ushell_putchar(LF);
   print_msg((U8 code *)msg_prompt);
}
#endif

