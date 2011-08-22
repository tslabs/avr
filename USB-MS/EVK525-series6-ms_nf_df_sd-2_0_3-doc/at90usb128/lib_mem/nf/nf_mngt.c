/*This file has been prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the high level management for nand-flash
//!
//!  memory devices.
//!  It supports 1, 2 or 4 Nands of same type. Type can be 512B or 2kB Nand.
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

// TODO: remplacer les s_shift_xxx en (1<<s_shift_xxx): et ainsi faire des DIV/MUL plutot que des shift
// TODO: decomposer g_phys_page_addr en s_phys_block et s_phys_page (offset dans le block)

//_____  I N C L U D E S ___________________________________________________

#define _TRACE_        (DISABLE)
#define NF_ECC_MNGT    (DISABLE)

#include "config.h"
#include "conf_nf.h"
#include "nf.h"
#include "nf_drv.h"
#include "nf_mngt.h"
#include "lib_mcu/usb/usb_drv.h"            /* usb driver definition */
#include "lib_mcu/debug.h"


#ifndef __GNUC__
  extern __no_init volatile xdata Byte nf_send_cmd At(NF_CMD_LATCH_ENABLE_ADD);  // Command
  extern __no_init volatile xdata Byte nf_send_add At(NF_ADD_LATCH_ENABLE_ADD);  // Address
  extern __no_init volatile xdata Byte nf_data At(NF_ADDRESS_CMD_DATA);          // Data
#else
  extern volatile unsigned char nf_send_cmd __attribute__ ((section (".nf_cmd")));
  extern volatile unsigned char nf_send_add __attribute__ ((section (".nf_add")));
  extern volatile unsigned char nf_data     __attribute__ ((section (".nf_dat")));
#endif


//_____ D E F I N I T I O N ________________________________________________

//#error Attention au modulo, call uidiv, uldiv, ...
//Se servir plus souvent du Random Data Input pour le code 2K (création de LUT)

#if( NF_BAD_CONFIG==(FALSE) )

//_____ M A C R O S ________________________________________________________


//_____ P R I V A T E    D E C L A R A T I O N _____________________________

// Static definition, which can be optimized by the compiler
//
#if (NF_GENERIC_DRIVER==TRUE) || (defined NF_AUTO_DETECT_2KB) ||(defined NF_AUTO_DETECT_512B)
extern _MEM_TYPE_SLOW_        U8    g_n_zones               ; // number of zones (=1024 blocks) per device
extern _MEM_TYPE_SLOW_       U16  g_n_blocks              ; // number of blocks per device
extern _MEM_TYPE_FAST_       U8   g_n_row_cycles          ; // number of row cycles to access a page of the NF memory
extern _MEM_TYPE_SLOW_        U8    g_copy_back_cont        ; // 0 = copy back not supported, N = number of    CONTINUE subdivision contraint on copyback
extern _MEM_TYPE_SLOW_        U8    g_copy_back_discont     ; // 0 = copy back not supported, N = number of DISCONTINUE subdivision contraint on copyback
#endif

#if (NF_GENERIC_DRIVER==TRUE)
       _MEM_TYPE_FAST_       U8   g_shift_page_byte       ; // (1<<n) size of page,   unit in bytes
       _MEM_TYPE_FAST_       U8   g_shift_block_page      ; // (1<<n) size of physical block,  unit in pages
       _MEM_TYPE_SLOW_       U8   g_ofst_blk_status       ; // Offset of Block Status information in spare zone
static _MEM_TYPE_SLOW_       U8   s_shift_sector_byte     ; // (1<<n) size of sector, unit in bytes
static _MEM_TYPE_SLOW_       U8   s_shift_log_page_sector ; // (1<<n) size of logical   page,  unit in sectors
static _MEM_TYPE_SLOW_       U8   s_shift_log_block_sector; // (1<<n) size of logical  block,  unit in sectors
#endif


       Bool g_fatal       ; // Used in LUT/FBB building and ECC management...


static Bool s_mem         ;
#if (defined ATMEL_WARNING)
#  warning In waiting for a recoding of the control_access module.
#endif
static Bool s_start       ;

       _MEM_TYPE_SLOW_ U32  g_copy_src                         ; // Used to copy NF pages (source page)
       _MEM_TYPE_SLOW_ U16  g_nf_first_block=0                 ; // Block addr of the beginning of dynamic area

typedef enum
{
   STATE_READ_INIT=0       // The very first open_read must be done
,  STATE_READ_RESUME_PAGE  // A page has been read
,  STATE_WRITE_INIT        // The very first open_write must be done
,  STATE_WRITE_RESUME_PAGE // A page has been written
,  STATE_COMPLETE          // The read or write session is over.
} Nf_state;


       _MEM_TYPE_SLOW_ Cache_lut g_cache_lut; // LUT cache
       _MEM_TYPE_SLOW_ Cache_fbb g_cache_fbb; // Free-Blocks block cache


       _MEM_TYPE_SLOW_ U8   g_page_buffer[NF_FULL_PAGE_BUFFER_SIZE] ; // Used to bufferize a page

// Dynamic variables
//
static _MEM_TYPE_SLOW_    U32       s_save_log_addr              ; // Used for Stand-by / Restart operations
//static _MEM_TYPE_SLOW_    U16       s_save_n_sector              ; // Used for Stand-by / Restart operations
//static _MEM_TYPE_SLOW_    U32       s_save2_log_addr             ; // Used for Stand-by / Restart operations
//static _MEM_TYPE_SLOW_    U16       s_save2_n_sector             ; // Used for Stand-by / Restart operations
       _MEM_TYPE_BIT_     bit       g_nf_init                    ; // Boolean set when driver is initialized
       _MEM_TYPE_MEDFAST_ U16       g_log_block_id               ; // Logical Block address
       _MEM_TYPE_SLOW_    U16       g_n_export_blocks=0xFFFF     ; // Number of physical blocks exported for mass-storage use
       _MEM_TYPE_SLOW_    U16       g_n_free_blocks              ; // Number of free physical blocks
       _MEM_TYPE_SLOW_    U8        g_n_sub_lut                  ; // Holds the number of sub-Lut
       _MEM_TYPE_SLOW_    U16       g_sub_lut_log_sz             ; // Size of the sub-LUT. Unit in number of logical blocks
       _MEM_TYPE_SLOW_    U16       g_last_sub_lut_log_sz        ; // Size of the last sub-LUT. Unit in number of logical blocks
       _MEM_TYPE_SLOW_    U16       g_fbb_block_addr             ; // Free-Blocks block address
       _MEM_TYPE_SLOW_    U8        g_fbb_block_index            ; // Free-Blocks block index
       _MEM_TYPE_SLOW_    U16       g_lut_block_addr [ N_SUBLUT ]; // LUT block address
       _MEM_TYPE_SLOW_    U8        g_lut_block_index[ N_SUBLUT ]; // LUT index, unit in (LUT size/page size)

static _MEM_TYPE_FAST_    U16       s_n_sectors                  ; // Holds the number of sectors to read/write
static _MEM_TYPE_FAST_    U8        s_nb_sectors_step            ; // Holds the number of sectors read after each page
       _MEM_TYPE_FAST_    U8        g_curr_dev_id                ; // Holds the current device number that is used
static _MEM_TYPE_FAST_    U16       s_curr_n_byte                ; // Holds the position in the page
static _MEM_TYPE_FAST_    U32       s_curr_log_sector            ; // Holds the logical sector number
       _MEM_TYPE_SLOW_    U32       g_last_log_sector =0xFFFFFFFF; // Holds the last logical sector number on which a Write has been done
static _MEM_TYPE_FAST_    Nf_state  s_state                      ; // Holds the current state of the driver

       _MEM_TYPE_SLOW_    U16       g_block_to_kill[ NF_N_DEVICES]    ; // Holds the blocks number which will be erased
       _MEM_TYPE_FAST_    U32       g_phys_page_addr[NF_N_DEVICES]    ; // Holds the current phys page number for each device

       _MEM_TYPE_SLOW_    U32       g_save_phys_page_addr             ; // Holds the previous phys page number
       _MEM_TYPE_SLOW_    U8        g_save_curr_dev_id                ; // Holds the previous device number that is used

       _MEM_TYPE_FAST_    U32       g_next_phys_page_addr             ; // Holds the previous phys page number

typedef enum
{
   NF_TRANS_NORMAL  // make simple translation.
,  NF_TRANS_FLUSH   // make simple translation. Force flush of LUT and FBB caches.
,  NF_TRANS_SWAP    // Swap blocks LUT <-> FBB
} Nf_translate_mode;


//_____ P R I V A T E    F U N C T I O N S _________________________________
//
static void        nf_translate( Nf_translate_mode mode );
static Status_bool nf_open_read(    bit check_pending_write );
static Status_bool nf_open_write(   bit check_pending_write );
static void        nf_cache_lut_refill( U16 log_block_id  );
static void        nf_cache_lut_flush( void  );
static void        nf_erase_old_blocks( void );

U8                 nf_xfer_update_vars(void);
void               nf_write_sector_from_usb(U8);
void               nf_read_sector_to_usb(U8);
void               nf_update_spare_zone(U8, U8);

//_____ F U N C T I O N S __________________________________________________
//

//! Ensure that the memory is in a good state before starting to use it
//!
//! @param none
//!
//! @return a status:
//!           PASS if the command has been succesfully executed;
//!           FAIL else
//!
Status_bool nf_verify( void )
{
   if ( g_nf_init ) return PASS;

   return nf_verify_resume();
}



//! Initializes the NF driver on the first USB Test Unit Ready.
//!
//! @param none
//!
//! @return CTRL_GOOD if ok,
//!         CTRL_NO_PRESENT in case of problems.
//!
Ctrl_status nf_test_unit_ready ( void )
{
  Status_bool tmp_bool;

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   tmp_bool = nf_verify();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   return ( tmp_bool==PASS ) ? CTRL_GOOD : CTRL_FAIL;
}


//! Returns the address of the last valid logical sector.
//!
//! @param none
//!
//! @return CTRL_GOOD if ok,
//!         CTRL_NO_PRESENT in case of problems.
//!
Ctrl_status nf_read_capacity (U32  *u32_nb_sector )
{
  Status_bool status_bool;

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   status_bool = nf_verify();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   *u32_nb_sector = nf_get_sectors_number()-1;
   return ( status_bool==PASS ) ? CTRL_GOOD : CTRL_FAIL;
}


Bool  nf_wr_protect ( void )
{
    return FALSE;
}

Bool  nf_removal ( void )
{
    return TRUE;
}


//! Returns a pointer on the internal buffer address.
//!
//! This function is used for test only.
//!
//! @param none
//!
//! @return pointer on an internal buffer of 2112 bytes.
//!
#if 0
U8*  nf_get_buffer_addr         ( void ) { return g_page_buffer; }
#endif


//! Returns the total number of sectors that can be used on
//! the memory.
//!
//! This number is computed during the power on, after a scan
//! of the memory.
//!
//! @param none
//!
//! @return Number of sectors.
//!
U32  nf_get_sectors_number      ( void )
{
   return
      (U32)g_n_export_blocks
   << (G_SHIFT_BLOCK_PAGE +G_SHIFT_PAGE_BYTE -S_SHIFT_SECTOR_BYTE)
   ;
}



U32 nf_block_2_page(U16 block_addr)
{
   return (U32)block_addr<<G_SHIFT_BLOCK_PAGE;
}



//! This function initializes the Nand Flash for a read operation.
//!
//! @param log_sector   Logical sector address to start read
//! @param nb_sector    Number of sectors to transfer
//!
//! @return CTRL_GOOD if ok, CTRL_FAIL if read outside memory
//!
Ctrl_status nf_read_10( U32 log_sector , U16 n_sectors)
{
  U8  status;
  
   if ( !g_nf_init )
      while(1);   // You shall call once mem_test_unit_ready() before.

   // Test that the logical sector address is valid
   //
   if ( 0==n_sectors )                                   { return CTRL_GOOD; }
   if ( (log_sector+n_sectors)>nf_get_sectors_number() ) { return CTRL_FAIL; }

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   s_n_sectors       = n_sectors;
   s_curr_log_sector = log_sector;
   trace("rd;"); trace_hex32(s_curr_log_sector); trace(";"); trace_hex16(s_n_sectors); trace_nl();
   s_save_log_addr   = log_sector + n_sectors;
   g_fatal           = FALSE;
   s_mem             = TRUE;
   s_start           = TRUE;

   // First read operation
   Nf_access_signal_on();
   nf_open_read(TRUE);
   Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);
   nfc_open_page_read( g_phys_page_addr[g_curr_dev_id], s_curr_n_byte );
   nf_read_sector_to_usb(s_nb_sectors_step);
   status = nf_xfer_update_vars();

   // Next read operations
   while (status == FALSE)    // exit when last page read
   {
      if (!(LSB0(g_next_phys_page_addr) & (SIZE_BLOCK_PAGE-1))    // new block
      && (g_curr_dev_id==0                                  ))    // on device 0. Should this case be implicit ? If yes, we can remove it.
      {
         nf_open_read(FALSE);
      }
      Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);
      Nfc_open_page_read( g_next_phys_page_addr, s_curr_n_byte ); // Use macro for fast execution
      nf_read_sector_to_usb(s_nb_sectors_step);                   // read the concerned sectors of the selected page
      status = nf_xfer_update_vars();                             // check if last page or not
   }

   Nf_access_signal_off();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   return CTRL_GOOD;
}



//! This function initializes the Nand Flash for a write operation.
//!
//! @param log_sector   Logical sector address to start read
//! @param nb_sector    Number of sectors to transfer
//!
//! @return CTRL_GOOD if ok, CTRL_FAIL if read outside memory
//!
Ctrl_status nf_write_10( U32 log_sector , U16 n_sectors)
{
  U8 status;
  Ctrl_status tmp_bool;

   // Test that the logical sector address is valid
   if ( 0==n_sectors )                                   { return CTRL_GOOD; }
   if ( (log_sector+n_sectors)>nf_get_sectors_number() ) { return CTRL_FAIL; }

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   s_n_sectors       = n_sectors;
   s_curr_log_sector = log_sector;
   s_save_log_addr   = log_sector + n_sectors;
   g_fatal           = FALSE;
   s_mem             = TRUE;
   s_start           = TRUE;

   trace("wr;"); trace_hex32(s_curr_log_sector); trace(";"); trace_hex16(s_n_sectors); trace_nl();
   
   // First write operation
   Nf_access_signal_on();
   if(( s_curr_log_sector==g_last_log_sector )                                           // New write is just after to the last write
   && (!(  ( 0==((U16)g_last_log_sector & ( ((U16)1<<(S_SHIFT_LOG_BLOCK_SECTOR)) -1)))   // Not on a logical block boundary
      && ( g_curr_dev_id==0                                                        ))))
   {
      trace("continue");trace_nl();
      nf_translate( NF_TRANS_NORMAL );
      Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);  // open the current device
      nfc_open_page_write( g_next_phys_page_addr, s_curr_n_byte );
   }
   else
   {      
      nf_open_write( TRUE );
   }
   nf_write_sector_from_usb(s_nb_sectors_step);    // s_nb_sectors_step has been calculated in nf_translate()
   if (Is_nf_2k())
   {
      nf_update_spare_zone((U8)(1<<(G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE))-s_nb_sectors_step, s_nb_sectors_step);    // update the spare zone once the page has been filled in
   }
   else
   {
     nf_update_spare_zone(0, 1);    // update the spare zone once the page has been filled in
   }
   g_last_log_sector  = s_curr_log_sector + s_nb_sectors_step;    // Memorize next logical sector to be managed
   status = nf_xfer_update_vars();

   // Next write operations
   while (status == FALSE)    // exit when operation finished
   {
      if(!(LSB0(g_next_phys_page_addr) & (SIZE_BLOCK_PAGE-1))  // new block
      && (g_curr_dev_id==0                                  )) // on device 0.
      {
         Nfc_set_cmd(NF_PAGE_PROGRAM_CMD); // Program the page
         nf_erase_old_blocks();
         nf_open_write( FALSE );
      }
      else
      {
         if( G_CACHE_PROG )
         {
            Nfc_set_cmd(NF_CACHE_PROGRAM_CMD);
         }else{
            Nfc_set_cmd(NF_PAGE_PROGRAM_CMD);
         }
         Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);
         Nfc_open_page_write( g_next_phys_page_addr, s_curr_n_byte ); // Use macro for fast execution
      }
      nf_write_sector_from_usb(s_nb_sectors_step);
      if (Is_nf_2k())
      {
         nf_update_spare_zone(0, s_nb_sectors_step);
      }
      else
      {
         nf_update_spare_zone(0, 1);    // update the spare zone once the page has been filled in
      }
      g_last_log_sector  = s_curr_log_sector + s_nb_sectors_step;    // Memorize next logical sector to be managed
      status = nf_xfer_update_vars();  // check if last block or not
   }

   tmp_bool = nf_dfc_write_stop(0);   // ends write operations with "nf_dfc_write_stop(0)" that save the current environnement
   Nf_access_signal_off();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   return tmp_bool;
}




//_____ P R I V A T E    F U N C T I O N S _________________________________
//

//! This function update transfer variables, check if operation (read/write) is finished
//! This function may be used either with READ and WRITE operations
//!
//! @param none
//!
//! @return TRUE if operation complete, FALSE if read/write to be continued
//!
U8 nf_xfer_update_vars(void)
{
   if ( // Are we processing the last page ?
      (  (s_curr_log_sector & (SIZE_PAGE_SECTOR-1))
      +  s_n_sectors
      )
   <  SIZE_PAGE_SECTOR
   ) {
      s_state = STATE_COMPLETE;
      return TRUE;
   }

   // Update position variables
   s_n_sectors       -= s_nb_sectors_step;
   s_curr_log_sector += s_nb_sectors_step;
   s_curr_n_byte = 0;
   if (s_n_sectors < SIZE_PAGE_SECTOR)
     s_nb_sectors_step = (U8) (s_n_sectors);   // next page must be read as partial (not all sectors remaining)
   else
     s_nb_sectors_step = SIZE_PAGE_SECTOR;     // next page to be read considered as entire (all sectors requested)

   // Save current parameters
   g_save_curr_dev_id    = g_curr_dev_id;
   g_save_phys_page_addr = g_phys_page_addr[g_curr_dev_id];

   // Fetch the next device id
   //
   g_phys_page_addr[g_curr_dev_id]+=1;
   g_curr_dev_id ++;
   if( g_curr_dev_id==NF_N_DEVICES ) { g_curr_dev_id=0; }

   g_next_phys_page_addr = g_phys_page_addr[g_curr_dev_id];

   if( s_n_sectors==0 )    // Operation complete !
   {
     s_state = STATE_COMPLETE;
     return TRUE;
   }

   return FALSE;
}


//! This function transfers a page content (NF) to the USB macro
//! The number of sectors to be read can be 1 up to 4 (more than 1 is available only for memories with 2kb pages)
//!
//! @param nb_sectors   number of sectors to be read from NF (1 sector = 512 bytes)
//!
//! @return none
//!
void nf_read_sector_to_usb(U8 nb_sectors)
{
 U8 j;

   for (j = 8*nb_sectors; j != 0; j--)                      // 8 * 64 bytes = 512 bytes
   {
      Disable_interrupt();

      Usb_write_byte(Nf_rd_byte());                         // read 64 bytes from card
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Usb_write_byte(Nf_rd_byte());
      Enable_interrupt();

      Usb_send_in();                            // validate transfer
      while(Is_usb_write_enabled()==FALSE)
      {
         if(!Is_usb_endpoint_enabled())
            return; // USB Reset
      }
   }
}



//! This function transfers USB data to the NF page
//! The number of sectors to be read can be 1 up to 4 (more than 1 is available only for memories with 2kb pages)
//!
//! @param nb_sectors   number of sectors to be read from USB (1 sector = 512 bytes)
//!
//! @return none
//!
void nf_write_sector_from_usb(U8 nb_sectors)
{
   U8 j;
   for (j = 8*nb_sectors ; j != 0 ; j--)        // 8 * 64 bytes = 512 bytes
   {
      while(!Is_usb_read_enabled())
      {
         if(!Is_usb_endpoint_enabled())
           return; // USB Reset
      }
      Disable_interrupt();                      // Global disable.

      Nf_wr_byte(Usb_read_byte());              // write 64 bytes to the card
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());
      Nf_wr_byte(Usb_read_byte());

      Usb_ack_receive_out();           // USB EPOUT read acknowledgement.
      Enable_interrupt();              // Global re-enable.
   }
}


//! This function updates the spare zone of each page that has been finished to be written
//!
//! @param  U8 sect_start     (0..3) indicates which sector has started to be written
//!         U8 nb_sect        (1..4) indicates the number of sectors concerned
//!
//!        <global parameters>
//!
//! @return none
//!
void nf_update_spare_zone(U8 sect_start, U8 nb_sect)
{
   // Calculate first spare zone address
   U8 i;
   _MEM_TYPE_SLOW_ U16 byte_addr = NF_SPARE_POS + ((U16)sect_start)*16;

  // Send command + address if 2kb' page model
   if (Is_nf_2k())
   {
      Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
      Nfc_set_adc( (byte_addr)%256 );
      Nfc_set_adc( (byte_addr)/256 );
   }

   // Send data (spare zone x sectors written)
   for( i=nb_sect ; i!=0 ; i-- )
   {
      Nfc_wr_data( 0xFF                 ); // 0- [FW] block is valid (for 2048 compatibility)
      Nfc_wr_data( NFC_BLK_ID_DATA      ); // 1- [FW] Free-blocks block
      Nfc_wr_data( 0                    ); // 2- [HW] ECC is valid
      Nfc_wr_data( NFC_OFST_3_DATA_DST  ); // 3- [SW] Source block (recovery) (HW capability not used)
      Nfc_wr_data( NFC_SPARE_DATA_VALID ); // 4- [FW] Data is valid
      Nfc_wr_data( 0xFF                 ); // 5- [FW] block is valid (for 512 compatibility)
      Nfc_wr_data( MSB(g_log_block_id)  ); // 6- [HW] LBA
      Nfc_wr_data( LSB(g_log_block_id)  ); // 7- [HW] LBA
      Nfc_wr_data( 0xFF                 ); // 8-9-10- [HW] ECC2
      Nfc_wr_data( 0xFF                 );
      Nfc_wr_data( 0xFF                 );
      Nfc_wr_data( 0xFF                 ); // 11-12-    [HW] LBA
      Nfc_wr_data( 0xFF                 );
      Nfc_wr_data( 0xFF                 );
      Nfc_wr_data( 0xFF                 );
      Nfc_wr_data( 0xFF                 ); // 13-14-15- [HW] ECC1
   }
}



//! This function must be called when a write10 operation (from USB) is finished
//! Last page written is programmed and environment is saved
//!
//! @param log_sector   Logical sector address to start read
//! @param nb_sector    Number of sectors to transfer
//!
//! @return CTRL_GOOD if ok, CTRL_FAIL if read outside memory
//!
Ctrl_status nf_dfc_write_stop( U16 u16_nb_sector_remaining ) // function that stops the dfc interface of the memory
{
   Nfc_set_cmd(NF_PAGE_PROGRAM_CMD); // Program the page
   // Note that if the page is not completely filled in yet but still programmed, the next copy tail will allow partial page program

   // retreive exact logical/physical position (in case that transfer
   // did not perform the exact number of sectors)
   s_curr_log_sector = s_save_log_addr - u16_nb_sector_remaining;
   if( 0!=u16_nb_sector_remaining )
   {
      nf_translate( NF_TRANS_NORMAL );
   }
   g_last_log_sector  = s_curr_log_sector; // Memorize next logical sector to be managed

   // Test if transfer stop at end of logical blocks.
   if( s_n_sectors==0 )
   {
      if(!(LSB0(g_next_phys_page_addr) & (SIZE_BLOCK_PAGE-1))  // new block
      && (g_curr_dev_id==0                                  )) // on device 0.
     {
         // Delete old blocks
         //
         nf_erase_old_blocks();
      }
   }
   trace("nf_dfc_write_stop;"); trace_hex32(g_last_log_sector); trace_nl();

   // Ensure that all internal programming are complete, since we are
   // using cache programming (WP may be asserted for icons or fonts
   // loading). Moreover, on some NFs (ST, Samsung)
   // it is necessary to reset the devices after copy-back commands
   // If not, strange behaviour may happen: no busy when opening
   // a page for reading...
   nfc_reset_nands( NF_N_DEVICES ); // Reset all the NF devices

   Nf_check_fbb( FALSE );
   Nf_check_lut();

   return CTRL_GOOD;
} // end of nf_dfc_write_stop



//! Erase the source blocks
//!
//! @param none
//!
//! @return none
//!
static void nf_erase_old_blocks( void )
{
   U8  i;

   // Delete old blocks
   //
   for ( i=0 ; i<NF_N_DEVICES ; i++ )
   {
      Nfc_action(NFC_ACT_DEV_SELECT, i);
      trace("nf_erase_old_blocks;"); trace_hex16(g_block_to_kill[i]);trace_nl();
      nfc_erase_block( nf_block_2_page(g_block_to_kill[i]), TRUE );
   }
}



//! Prepare a read session on the flash memory.
//!
//! This function translate the logical sector address
//! to a physical page number. The LUT cache is used.
//!
//! @param s_curr_log_sector static that should be initialized before
//!
//! @return a status:
//!           PASS if the function has been succesfully executed;
//!
//#error finaliser l'activation du CE[dev]
//#error attention a l'horloge CLK read et CLK write
static Status_bool nf_open_read( bit check_pending_write )
{
   if(( check_pending_write   )
   && ( 0xFFFFFFFF!=g_last_log_sector ))
   {
      nf_copy_tail();
   }

   // Both LUT and FBB caches are flushed. Why?
   // - Avoid LUT/FBB caches flush and refill during audio playback
   // - Avoid recovery on power-on
   nf_translate( NF_TRANS_FLUSH );

   return PASS;
}


//! Prepare a write session on the flash memory.
//!
//! This function translates the logical sector address
//! to a physical page number. Then, the block used are
//! swapped with free blocks. The LUT and FBB caches are
//! used for that purpose. The head of the used block(s)
//! are copied into the free block(s).
//! Assumption is made that there is at least 2 free entries
//! in the FBB cache: one to swap the LUT blocks, and another
//! one to recycle the FBB block itself (if needed).
//!
//! @param static that should be initialized before
//!
//! @return a status:
//!           PASS if the function has been succesfully executed;
//!
static Status_bool nf_open_write( bit check_pending_write )
{
   U8  u8_tmp;
   _MEM_TYPE_SLOW_ U8  u8_curr_page;
   _MEM_TYPE_SLOW_ U8  u8_last_page;
   U16 u16_tmp;

   if(( check_pending_write   )
   && ( 0xFFFFFFFF!=g_last_log_sector ))
   {
      nf_copy_tail();
   }

   nf_translate( NF_TRANS_SWAP );

   // both FBB and LUT blocks are invalid
   // Mark FBB only. Warning: Partial Prog
   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   nfc_open_page_write(
      nf_block_2_page( g_fbb_block_addr )  // base address
   +  (U32)g_fbb_block_index               // offset to Free-blocks block entry
   ,  NF_SPARE_POS+NFC_SPARE_OFST_6_LBA );
   Nfc_wr_data( NFC_OFST_6_FBB_INVALID );                         // 6- FBB-LUTs are invalid
   Nfc_set_cmd( NF_PAGE_PROGRAM_CMD );                            // Warning: Partial Programming

   // Mark sources blocks (recovery). Warning: Partial Prog
   for( u8_tmp=0 ; u8_tmp<NF_N_DEVICES ; u8_tmp++ )
   {
      Nfc_action(NFC_ACT_DEV_SELECT, u8_tmp);
      nfc_open_page_write(
         nf_block_2_page( g_block_to_kill[u8_tmp] )
      ,  NF_SPARE_POS+NFC_SPARE_OFST_3_BYTE_3 );
      Nfc_wr_data( NFC_OFST_3_DATA_SRC );                         // 3- [SW] Mark as source block (recovery) (HW capability not used)
      Nfc_set_cmd( NF_PAGE_PROGRAM_CMD );                         // Warning: Partial Programming
   }

   // Copy the head of the buffer
   //
   // for each logical page (current included)
   u8_last_page= (s_curr_log_sector >>S_SHIFT_LOG_PAGE_SECTOR) & (SIZE_BLOCK_PAGE -1);
   for(  u8_curr_page=0
   ;     u8_curr_page <= u8_last_page
   ;     u8_curr_page++ )
   {
      // If the last page (current)
      if ( u8_last_page==u8_curr_page ) { u8_tmp = g_curr_dev_id; } // then copy this physical page only for the device before the current device
      else                              { u8_tmp = NF_N_DEVICES;  } // else copy this physical page for all device

      while( u8_tmp!=0 ) // for each device
      {
         u8_tmp--;
         Nfc_action(NFC_ACT_DEV_SELECT, u8_tmp);

         g_copy_src= nf_block_2_page( g_block_to_kill[u8_tmp] ) + u8_curr_page;
         nf_copy(g_phys_page_addr[u8_tmp]);
         g_phys_page_addr[u8_tmp]++;
      }
      // end of loop, for each device
   }
   // end of loop, for each logical page

   u16_tmp=
      (SIZE_SECTOR_BYTE)                          // size (unit byte) of the sector (spare zone excluded)
   *  (  s_curr_log_sector                        // current sector in physical page
      &  (SIZE_PAGE_SECTOR -1)
      );

   Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);
   if( u16_tmp )
   {
      //** copy the head of the current physical page
      nfc_open_page_read( nf_block_2_page( g_block_to_kill[g_curr_dev_id] ) + u8_last_page, 0 );
      // copy the head page of the old block in buffer
      nf_upload(g_page_buffer, (U8)(u16_tmp/16));
   }

   // copy the buffer in the head page of the new block
   nfc_open_page_write( g_phys_page_addr[g_curr_dev_id], 0 );

   // write the first byte to the current byte position in the physical page
   nf_download(g_page_buffer, (U8)(u16_tmp/16));
   // END of the copy head

   return PASS;
} // nf_open_write



//! Reload the LUT cache memory, starting from the specified
//! logical block number given.
//!
//! The cache is filled with physical blocks. The cache does only work inside
//! a sub-LUT. If the logical number is 'near' to the end of the sub-LUT, the
//! cache starts before the logical number, in order to be filled untill the
//! end of the sub-LUT.
//!
//! @param log_block_id   logical block number.
//!
//! @return nothing
//!
static void nf_cache_lut_refill(U16 log_block_id)
{
   U8  u8_tmp;
   U8  sub_lut_id;

   U16 byte_addr;
   _MEM_TYPE_SLOW_ U32 page_addr;
   _MEM_TYPE_SLOW_ U16 sub_lut_log_sz;    // size of the sub-LUT. Unit in logical blocks.
   _MEM_TYPE_SLOW_ U16 sub_lut_log_first; // number of the first logical block of the sub-lut.

   Assert( g_cache_lut.ctrl.dirty==FALSE ); // No refill if the cache is dirty

   sub_lut_id        = log_block_id>>(    NF_SHIFT_SUBLUT_PHYS - NF_SHIFT_N_DEVICES);
   sub_lut_log_sz    = ( sub_lut_id==(g_n_sub_lut-1) ? g_last_sub_lut_log_sz : g_sub_lut_log_sz );
   sub_lut_log_first = (U16)sub_lut_id<<( NF_SHIFT_SUBLUT_PHYS - NF_SHIFT_N_DEVICES);

   trace("nf_cache_lut_refill;"); trace_hex16(g_lut_block_addr[sub_lut_id]);trace_nl();

   if (sub_lut_log_sz<NF_CACHE_LUT_LOG_SZ)
   {  // The cache is bigger than the sub LUT
      g_cache_lut.first = log_block_id;                               // First included
      g_cache_lut.last  = log_block_id + (sub_lut_log_sz-1);          // Last included
   }
   else if ( (log_block_id+NF_CACHE_LUT_LOG_SZ) <= (sub_lut_log_first+sub_lut_log_sz) )
   {  // The cache is done starting from the logical block number
      g_cache_lut.first = log_block_id;                               // First included
      g_cache_lut.last  = log_block_id + (NF_CACHE_LUT_LOG_SZ-1);     // Last included
   }
   else
   {  // The cache is done starting from the last logical block of the sub-LUT
      g_cache_lut.last  = sub_lut_log_first +sub_lut_log_sz -1;       // Last included
      g_cache_lut.first = g_cache_lut.last - (NF_CACHE_LUT_LOG_SZ-1); // First included
   }

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);

   page_addr =
      nf_block_2_page( g_lut_block_addr[sub_lut_id]) // base address
   +  (U32)(g_lut_block_index[sub_lut_id])           // offset to sub-LUT
   ;
   byte_addr = ( (g_cache_lut.first-sub_lut_log_first)*NF_N_DEVICES*2 ); // logical position of the block
   Assert( byte_addr<SIZE_PAGE_BYTE );

   nfc_open_page_read( page_addr, byte_addr);
   trace_hex32(page_addr); trace(";"); trace_hex16(byte_addr);trace_nl();
   for ( u8_tmp=0 ; u8_tmp<(g_cache_lut.last+1-g_cache_lut.first)*NF_N_DEVICES ; u8_tmp++ )
   { // fill the cache buffer
      Assert( u8_tmp<CACHE_LUT_SIZE);
      MSB(g_cache_lut.mem[u8_tmp]) = Nfc_rd_data_fetch_next();
      LSB(g_cache_lut.mem[u8_tmp]) = Nfc_rd_data_fetch_next();
      trace_hex16(g_cache_lut.mem[u8_tmp]);
      trace("-");
      Assert( g_cache_lut.mem[u8_tmp]>=g_nf_first_block );
      Assert( g_cache_lut.mem[u8_tmp]< G_N_BLOCKS       );
   }
   trace_nl();
   g_cache_lut.ctrl.valid = TRUE;
} // end of nf_cache_lut_refill



//! Reload the FBB cache memory, starting from 0.
//!
//! The cache is filled with physical blocks.
//!
//! @param none
//!
//! @return nothing
//!
void nf_cache_fbb_refill(void)
{
   U8  u8_tmp;
   U16 byte_addr;
   _MEM_TYPE_SLOW_ U32 page_addr;

   Assert(g_cache_fbb.ctrl.dirty==FALSE); // No refill if the cache is dirty
   g_cache_fbb.p   = 0;

   trace("nf_cache_fbb_refill;"); trace_hex16(g_fbb_block_addr);trace_nl();

   // Ensure that the cache is bigger than the real number of free blocks
   //
   g_cache_fbb.max = Min(NF_CACHE_FBB_LOG_SZ, (g_n_free_blocks>>NF_SHIFT_N_DEVICES) ); // Last included

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);

   page_addr =
      nf_block_2_page( g_fbb_block_addr  ) // base address
   +  (U32)g_fbb_block_index               // offset to Free-blocks block entry
   ;
   byte_addr=0;

   nfc_open_page_read(page_addr, 0);

   for ( u8_tmp=0 ; u8_tmp<g_cache_fbb.max*NF_N_DEVICES ; u8_tmp++ )
   { // fill the cache buffer
      Assert( u8_tmp<CACHE_FBB_SIZE);
      MSB(g_cache_fbb.mem[u8_tmp]) = Nfc_rd_data_fetch_next();
      LSB(g_cache_fbb.mem[u8_tmp]) = Nfc_rd_data_fetch_next();
      Assert( g_cache_fbb.mem[u8_tmp]>=g_nf_first_block );
      Assert( g_cache_fbb.mem[u8_tmp]< G_N_BLOCKS       );
      byte_addr+=2;
      Assert( byte_addr<SIZE_PAGE_BYTE); // Should stay in the page. Algo limitation.
   }
   g_cache_fbb.ctrl.valid = TRUE;
} // end of nf_cache_fbb_refill



//! Flushes the LUT cache into a new LUT entry.
//!
//! A copy of the current LUT is made in a new page, taking into account
//! the last LUT modification in its cache. If the block containing the LUT
//! is full, a new block is taken from the FBB.
//!
//! @param none
//!
//! @return nothing
//!
static void nf_cache_lut_flush( void  )
{
   _MEM_TYPE_SLOW_ U32  page_addr;
   U16  byte_addr;
   _MEM_TYPE_SLOW_ U16  sub_lut_log_sz;
   _MEM_TYPE_SLOW_ U8   sub_lut_id;
   U8   u8_tmp;

   Assert(TRUE==g_cache_lut.ctrl.valid);
   Assert(TRUE==g_cache_lut.ctrl.dirty);

   sub_lut_id     = g_cache_lut.first>>(NF_SHIFT_SUBLUT_PHYS-NF_SHIFT_N_DEVICES);
   sub_lut_log_sz = ( sub_lut_id==(g_n_sub_lut-1) ) ? g_last_sub_lut_log_sz : g_sub_lut_log_sz ;

   trace("nf_cache_lut_flush;"); trace_hex16(g_lut_block_addr[sub_lut_id]);trace_nl();

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);

   page_addr=
      nf_block_2_page( g_lut_block_addr[sub_lut_id] )  // base address
   +  (U32)(g_lut_block_index[sub_lut_id])             // offset to sub-LUT
   ;

   nfc_open_page_read( page_addr, 0);

   for ( byte_addr=0 /* used as block address! */ ; byte_addr<(sub_lut_log_sz*NF_N_DEVICES) ; )
   {  // Read the LUT stored in Nand
      g_page_buffer[2*byte_addr   ] = Nfc_rd_data_fetch_next();
      g_page_buffer[2*byte_addr +1] = Nfc_rd_data_fetch_next();
#if (_ASSERT_==ENABLE)
      MSB(_debug)= g_page_buffer[2*byte_addr   ];
      LSB(_debug)= g_page_buffer[2*byte_addr +1];
      Assert( _debug>=g_nf_first_block );
      Assert( _debug< G_N_BLOCKS       );
#endif
      byte_addr ++;
   }

   // Modify the page
   //
   byte_addr = // logical position of the block
      (  g_cache_lut.first                               // Absolute logical block number...
      &  ( (U16)NF_SUBLUT_SIZE/NF_N_DEVICES -1)          // ...Modulo the number of logical block in a LUT
      )
   *  NF_N_DEVICES*2;

   Assert( byte_addr<2048 );

   for ( u8_tmp=0 ; u8_tmp<(g_cache_lut.last+1-g_cache_lut.first)*NF_N_DEVICES ; u8_tmp++ )
   {  // flush the cache buffer in the buffer
      Assert( u8_tmp<CACHE_LUT_SIZE );
      Assert( byte_addr<(NF_PAGE_BUFFER_SIZE-1) );
      Assert( g_cache_lut.mem[u8_tmp]>=g_nf_first_block );
      Assert( g_cache_lut.mem[u8_tmp]< G_N_BLOCKS       );
      g_page_buffer[byte_addr++] = MSB(g_cache_lut.mem[u8_tmp]) ;
      g_page_buffer[byte_addr++] = LSB(g_cache_lut.mem[u8_tmp]) ;
   }

   // Program the page
   //
   g_lut_block_index[sub_lut_id]++;

   if ( g_lut_block_index[sub_lut_id] ==  (1<<G_SHIFT_BLOCK_PAGE) )
   { // Need a new block for the flush
      _MEM_TYPE_SLOW_ U16 u16_swap;

      if ( FALSE==g_cache_fbb.ctrl.valid ) { nf_cache_fbb_refill(); }

#define U16_FBB_OFST   ((U16)NF_N_DEVICES*g_cache_fbb.p + S_MNGT_DEV)
      Assert( g_cache_fbb.mem[U16_FBB_OFST]>=g_nf_first_block );
      Assert( g_cache_fbb.mem[U16_FBB_OFST]< G_N_BLOCKS       );
      u16_swap                      = g_lut_block_addr[ sub_lut_id] ;
      g_lut_block_addr[ sub_lut_id] = g_cache_fbb.mem[U16_FBB_OFST] ;
      g_cache_fbb.mem[U16_FBB_OFST] = u16_swap ;
      g_lut_block_index[sub_lut_id] = 0;
#undef U16_FBB_OFST

      trace("nf_cache_lut_flush;swap;"); trace_hex16(g_lut_block_addr[sub_lut_id]);trace_nl();
      g_cache_fbb.ctrl.dirty=TRUE;

      nf_write_lut(0, sub_lut_id, sub_lut_log_sz); // TODO: we should test the status returned

      nfc_erase_block( nf_block_2_page( u16_swap ), TRUE );

      g_cache_fbb.p++;
      if( g_cache_fbb.p==(g_cache_fbb.max-1) )
      { // Only 1 remaining entry in FBB cache
         nf_cache_fbb_flush( FALSE ); // No dirty test, since we know that the cache is dirty
         nf_cache_fbb_refill();
      }
   }
   else
   {
      nf_write_lut(0, sub_lut_id, sub_lut_log_sz); // TODO: we should test the status returned
   }

   g_cache_lut.ctrl.dirty=FALSE;

   Nf_check_fbb( FALSE );
   Nf_check_lut();
} // end of nf_cache_lut_flush





//! Writes a LUT in memory from a buffer.
//!
//! @param pos             offset of the lub-lut in the buffer
//! @param i_sub_lut       id of the sub-LUT
//! @param sub_lut_log_sz  Size of the sub-LUT, in logical block unit
//!
//! @return nothing
//!
Status_bool nf_write_lut(
   U8  pos
,  U8  i_sub_lut
,  U16 sub_lut_log_sz)
{
   U16  block_addr; // Physical block number
   U8 _MEM_TYPE_SLOW_ * p_buf=g_page_buffer + (U16)pos*((U16)1<<(G_SHIFT_PAGE_BYTE));

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   // The buffer is built as:
   // |    512    |    512    |    512    |    512    |
   //
   // The page is built as:
   // |    512    |    512    |    512    |    512    |SZ|
   //
   // The LUT fits in a physical page.
   //
   nfc_open_page_write(
      nf_block_2_page( g_lut_block_addr[i_sub_lut] ) // base address
   +  (U32)(g_lut_block_index[i_sub_lut])            // offset to sub-LUT
   ,  0 );

   trace("nf_write_lut;");trace_hex16(g_lut_block_addr[i_sub_lut]); trace(";"); trace_hex16(g_lut_block_index[i_sub_lut]);trace_nl();
   for( block_addr=0 ; block_addr<((U16)1<<(G_SHIFT_PAGE_BYTE-1)) ; block_addr+=1 )
   {
      if ( block_addr<(sub_lut_log_sz*NF_N_DEVICES) )
      {
#if (_ASSERT_==ENABLE)
         Assert(           ( block_addr*2)<(NF_PAGE_BUFFER_SIZE-1) );
         MSB(_debug)= p_buf[ block_addr*2   ] ;
         LSB(_debug)= p_buf[ block_addr*2 +1] ;
         Assert( _debug>=g_nf_first_block );
         Assert( _debug< G_N_BLOCKS       );
#endif
         Nfc_wr_data( p_buf[ block_addr*2   ] );
         Nfc_wr_data( p_buf[ block_addr*2 +1] );
         trace_hex(p_buf[ block_addr*2   ]);
         trace_hex(p_buf[ block_addr*2 +1]);
         trace("-");
      }
      else
      {
         Nfc_wr_data( 0xFF );
         Nfc_wr_data( 0xFF );
         trace("FFFF-");
      }
   }
   trace_nl();

   // Write the spare information
   //
   Nfc_wr_data( 0xFF              );                              // 0- block is valid (for 2048 compatibility)
   Nfc_wr_data( NFC_BLK_ID_SUBLUT );                              // 1- sub-LUT
   Nfc_wr_data( i_sub_lut         );                              // 2- sub-LUT id
   Nfc_wr_data( 0xFF              );                              // 3- unused
   Nfc_wr_data( g_n_sub_lut       );                              // 4- number of sub-LUT
   Nfc_wr_data( 0xFF              );                              // 5- block is valid (for 512 compatibility)
   Nfc_wr_data( MSB(sub_lut_log_sz) );                            // 6-7 Number of log blocks in sub-LUT
   Nfc_wr_data( LSB(sub_lut_log_sz) );
   Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); // 8-9-10-   ECC2
   Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF );                      // 11-12-    LBA
   Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); // 13-14-15- ECC1

   Nfc_set_cmd( NF_PAGE_PROGRAM_CMD );
   if ( FAIL==nfc_check_status() ) { return FAIL; }

   return PASS;
} // end of nf_write_lut



//! Flushes the FBB cache into a new FBB entry.
//!
//! A copy of the current FBB is made in a new page, taking into account
//! the last FBB modification in its cache. If the block containing the FBB
//! is full, a new block is taken from the FBB itself.
//!
//! @param b_ecc_err FALSE: normal operation,
//!        b_ecc_err TRUE: remove a block from list (p)
//!
//! @return nothing
//!
void nf_cache_fbb_flush( Bool b_ecc_err )
{
   _MEM_TYPE_SLOW_ U32  page_addr;
   _MEM_TYPE_SLOW_ U16  byte_addr;
   _MEM_TYPE_SLOW_ U16 u16_delete=0;
   _MEM_TYPE_SLOW_ U16  u16_tmp;
   U8   u8_tmp;
   _MEM_TYPE_SLOW_ U8   u8_pos;
   Bool bool_delete=FALSE;

   Assert(TRUE==g_cache_fbb.ctrl.valid);
   Assert(TRUE==g_cache_fbb.ctrl.dirty);
   // Assert(g_cache_fbb.p==(g_cache_fbb.max-1)); This is no more the case for ECC error not correctable

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);

   trace("nf_cache_fbb_flush;"); trace_hex16(g_fbb_block_addr);trace_nl();

   page_addr=
      nf_block_2_page( g_fbb_block_addr  )  // base address
   +  (U32)g_fbb_block_index                // offset to page
   ;

   g_fbb_block_index++;

   if ( g_fbb_block_index ==  (1<<G_SHIFT_BLOCK_PAGE)/(1<<0) )
   {  // Need to recycle the FBB block itself, using a free-block ! This is always possible
      // since there is always a free block (.p=.max-1) specially for that purpose.
      byte_addr = ((U16)NF_N_DEVICES*g_cache_fbb.p + S_MNGT_DEV);

      Assert( g_cache_fbb.mem[byte_addr]>=g_nf_first_block );
      Assert( g_cache_fbb.mem[byte_addr]< G_N_BLOCKS       );
      u16_delete                 = g_fbb_block_addr           ;
      g_fbb_block_addr           = g_cache_fbb.mem[byte_addr] ;
      g_cache_fbb.mem[byte_addr] = u16_delete;
      g_fbb_block_index = 0;

      trace("nf_cache_fbb_flush;swap;"); trace_hex16(g_fbb_block_addr);trace_nl();
      
      // g_cache_fbb.ctrl.dirty=TRUE; This is already the case !

      g_cache_fbb.p++;
      bool_delete=TRUE;
   }

   if( !b_ecc_err )  { u8_pos = g_cache_fbb.p;   }
   else              { u8_pos = g_cache_fbb.max; }

   byte_addr = ((U16)NF_N_DEVICES*2*u8_pos);

   Assert( byte_addr<SIZE_PAGE_BYTE );

   nfc_open_page_read( page_addr, byte_addr);

   Assert( g_cache_fbb.p<=(g_n_free_blocks/NF_N_DEVICES) );
   for (
      u16_tmp=0
   ;  u16_tmp<((g_n_free_blocks/NF_N_DEVICES)-u8_pos)*NF_N_DEVICES*2
   ; )
   {  // Move the free-blocks after <pos> to the beginning of the buffer.
      Assert( u16_tmp<(NF_PAGE_BUFFER_SIZE-3) );
      g_page_buffer[u16_tmp++] = Nfc_rd_data_fetch_next();
      g_page_buffer[u16_tmp++] = Nfc_rd_data_fetch_next();
#if (_ASSERT_==ENABLE)
      MSB(_debug)= g_page_buffer[u16_tmp-2];
      LSB(_debug)= g_page_buffer[u16_tmp-1];
      Assert( _debug>=g_nf_first_block );
      Assert( _debug< G_N_BLOCKS       );
#endif
   }

   for ( u8_tmp=0 ; u8_tmp<(u8_pos*NF_N_DEVICES); u8_tmp++ )
   {  // Then add the cache content
      Assert( u8_tmp < CACHE_FBB_SIZE );
      Assert( u16_tmp< NF_FULL_PAGE_BUFFER_SIZE );
#if (_ASSERT_==ENABLE)
      // When coming from ECC 2-bits error, an entry has been removed
      // from the cache, so the last entry is 0xffff
      if( !b_ecc_err )
      {
         Assert( g_cache_fbb.mem[u8_tmp]>=g_nf_first_block );
         Assert( g_cache_fbb.mem[u8_tmp]< G_N_BLOCKS       );
      }
#endif
      g_page_buffer[u16_tmp++] = MSB(g_cache_fbb.mem[u8_tmp]);
      g_page_buffer[u16_tmp++] = LSB(g_cache_fbb.mem[u8_tmp]);
   }
#if (_ASSERT_==ENABLE)
   // Ensure that, when there is no fbb recycle, the blocks in the cache at
   // position p are identical to the blocks in the beginning of the new line.
   if( (FALSE==bool_delete) && ( !b_ecc_err ))
   {
      for ( u8_tmp=0 ; u8_tmp<NF_N_DEVICES ; u8_tmp++ )
      {
         MSB(_debug)= g_page_buffer[u8_tmp*2   ];
         LSB(_debug)= g_page_buffer[u8_tmp*2 +1];
         Assert( _debug==g_cache_fbb.mem[(g_cache_fbb.p)*NF_N_DEVICES + u8_tmp] );
      }
   }
#endif

   nf_write_fbb(); // Should test the result
   Nf_check_fbb( b_ecc_err );
   Nf_check_lut();

   if( TRUE==bool_delete )
   {
      nfc_erase_block( nf_block_2_page( u16_delete ), TRUE );
   }

   Assert( u16_tmp==(g_n_free_blocks*2) ); // All the list should have been processed
   g_cache_fbb.ctrl.dirty=FALSE;
} // end of nf_cache_fbb_flush



//! Writes the Free-blocks block into the Nand Flash
//!
//! @param none
//!
//! @return nothing
//!
Status_bool nf_write_fbb( void )
{
   U16 u16_tmp;

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   nfc_open_page_write(
      nf_block_2_page( g_fbb_block_addr )  // base address
   +  (U32)g_fbb_block_index               // offset to Free-blocks block entry
   , 0 );
   for ( u16_tmp=0 ; u16_tmp<((U16)1<<(G_SHIFT_PAGE_BYTE-1)) ; )
   {
      if ( u16_tmp<g_n_free_blocks )
      {
         Nfc_wr_data( g_page_buffer[2*u16_tmp   ] );
         Nfc_wr_data( g_page_buffer[2*u16_tmp +1] );
      }
      else
      {
         Nfc_wr_data( 0xFF );
         Nfc_wr_data( 0xFF );
      }
      u16_tmp++;
   }
   // Write the spare information
   //
   Nfc_wr_data( 0xFF );                                           // 0- block is valid (for 2048 compatibility)
   Nfc_wr_data( NFC_BLK_ID_FBB );                                 // 1- Free-blocks block
   Nfc_wr_data( MSB(g_n_free_blocks));                            // 2-3 Number of free blocks
   Nfc_wr_data( LSB(g_n_free_blocks));
   Nfc_wr_data( NFC_OFST_4_FBB_DRIVER_RELEASE );                  // 4- Nand-Flash driver ID
   Nfc_wr_data( 0xFF );                                           // 5- block is valid (for 512 compatibility)
   Nfc_wr_data( NFC_OFST_6_FBB_VALID );                           // 6- FBB-LUTs valid
   Nfc_wr_data( 0xFF );                                           // 7- Unused
   Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); // 8-9-10-   Unused
   Nfc_wr_data( MSB(g_n_export_blocks));                          // 11-12- Number of exported blocks
   Nfc_wr_data( LSB(g_n_export_blocks));
   Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); Nfc_wr_data( 0xFF ); // 13-14-15- Unused

   Nfc_set_cmd( NF_PAGE_PROGRAM_CMD );
   if ( FAIL==nfc_check_status() ) { return FAIL; }
   return PASS;
} // end of nf_write_fbb



#if (_ASSERT_==ENABLE)
static void nf_check_fbb( Bool b_ecc_err )
{
   U16 u16_tmp;

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   trace("nf_check_fbb\n\r");
   trace("g_fbb_block_addr "); trace_hex16(g_fbb_block_addr);
   trace("\n\rs_fbb_block_index= "); trace_hex(g_fbb_block_index); trace("\n\r");
   nfc_open_page_read(
      nf_block_2_page( g_fbb_block_addr )  // base address
   +  (U32)g_fbb_block_index               // offset to Free-blocks block entry
   , 0 );
   trace("n free blocks: "); trace_u16(g_n_free_blocks); trace_nl();
   for ( u16_tmp=0 ; u16_tmp<g_n_free_blocks ; )
   {
#if (_TRACE_==ENABLE)
      if( u16_tmp>=2048 ) break;
      if( !(u16_tmp% 8) ) {
         trace("\n\r");
         trace_u16(u16_tmp);
      }
#endif
      MSB(_debug)= Nfc_rd_data_fetch_next();
      LSB(_debug)= Nfc_rd_data_fetch_next();
      trace(" 0x"); trace_hex( MSB(_debug) ); trace_hex( LSB(_debug) );
      // When coming from ECC 2-bits error, an entry has been removed
      // from the cache, so the last entry is 0xffff
      if( !b_ecc_err )
      {
         Assert( _debug>=g_nf_first_block );
         Assert( _debug< G_N_BLOCKS       );
      }
#if 0
      This tests have been commented out: when the cache are dirty, we can not just check the memory
      with possible protected block address (fbb, lut).
      Ex: FBB is dirty, a recycle have been done between on of the free block and g_lut_block_addr[x].
      In the FBB memory, the (new) lut address is still present, even if it is not the case in the cache.
      if ( (u16_tmp%NF_N_DEVICES)==S_MNGT_DEV )
      { // Check dangerous address collision for NF_MNGT only
         Assert( _debug!=g_fbb_block_addr );
         for ( u8_tmp=0 ; u8_tmp<g_n_sub_lut ; u8_tmp++ ) {
            Assert( _debug!=g_lut_block_addr[u8_tmp] );
         }
      }
#endif
      u16_tmp++;
   }
   trace_nl();
} // nf_check_fbb


static void nf_check_lut( void )
{
   U16 u16_tmp;
   U8  sub_lut_id;
   U16 sub_lut_log_sz;

   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   for ( sub_lut_id=0 ; sub_lut_id<g_n_sub_lut ; sub_lut_id++ )
   {
      nfc_open_page_read(
         nf_block_2_page( g_lut_block_addr[sub_lut_id] ) // base address
      +  (U32)g_lut_block_index[sub_lut_id]              // offset to LUT block entry
      , 0 );

      sub_lut_log_sz = ( sub_lut_id==(g_n_sub_lut-1) ) ? g_last_sub_lut_log_sz : g_sub_lut_log_sz ;
      for ( u16_tmp=0 ; u16_tmp<(sub_lut_log_sz*NF_N_DEVICES) ; )
      {
         MSB(_debug)= Nfc_rd_data_fetch_next();
         LSB(_debug)= Nfc_rd_data_fetch_next();
         Assert( _debug>=g_nf_first_block );
         Assert( _debug< G_N_BLOCKS       );
#if 0
         if ( (u16_tmp%NF_N_DEVICES)==S_MNGT_DEV )
         { // Check dangerous address collision for NF_MNGT only
            Assert( _debug!=g_fbb_block_addr );
            for ( u8_tmp=0 ; u8_tmp<g_n_sub_lut ; u8_tmp++ ) {
               Assert( _debug!=g_lut_block_addr[u8_tmp] );
            }
         }
#endif
         u16_tmp++;
      }
   }
}
#endif



// Here are the variable supposed to be initialized:
//    g_curr_dev_id
//    s_curr_log_sector
//    g_block_to_kill[] block addr
//    g_phys_page_addr[] page addr
//
void nf_copy_tail(void)
{
   _MEM_TYPE_SLOW_ U8  u8_tmp;
                   U8  u8_tmp2;
   _MEM_TYPE_SLOW_ U16 u16_tmp;
   _MEM_TYPE_SLOW_ U16 byte_addr;

   // Test if we do not reach the end of the logical block
   //
   if( 0!=((U16)g_last_log_sector & ( ((U16)1<<(S_SHIFT_LOG_BLOCK_SECTOR)) -1)) )
   {
      trace("nf_copy_tail;"); trace_hex32(g_last_log_sector); trace_nl();
      if( Is_not_nf_512() )
      {  // Following is not possible on 512B Nand

         u8_tmp = // current offset sector in the current page
            LSB0(g_last_log_sector)
         &  ( SIZE_PAGE_SECTOR-1 )
         ;

         u8_tmp2 = SIZE_PAGE_SECTOR -u8_tmp;
         if( 0!=u8_tmp )
         {  // Copy the rest of the current line

            byte_addr=((U16)u8_tmp) * (SIZE_SECTOR_BYTE);
            Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);  // open the current device
            nfc_open_page_read(                                                       // Open the old block at :
                  nf_block_2_page( g_block_to_kill[g_curr_dev_id] )                   // adresse of the beginning of the old block
               +  (LSB0(g_phys_page_addr[g_curr_dev_id])&(SIZE_BLOCK_PAGE -1))        // current offset page in the old and new block
            ,  byte_addr );                                                           // current offset sector in the current page
            // for each sector in the physical page
            u16_tmp = u8_tmp2 * SIZE_SECTOR_BYTE;
            g_last_log_sector += u8_tmp2;                                             // update the current logical sector

            // read the sector of the old page
            nf_upload(g_page_buffer+byte_addr, u16_tmp/16 );

            // Read the associated spare zone
            byte_addr= NF_SPARE_POS + (((U16)u8_tmp)*16);
            nfc_open_page_read(                                                       // Open the old block at :
                  nf_block_2_page( g_block_to_kill[g_curr_dev_id] )                   // adresse of the beginning of the old block
               +  (LSB0(g_phys_page_addr[g_curr_dev_id])&(SIZE_BLOCK_PAGE -1))        // current offset page in the old and new block
            ,  byte_addr );                                                           // current offset sector in the current page
            nf_upload(g_page_buffer+byte_addr, u8_tmp2 );

            byte_addr=((U16)u8_tmp) * (SIZE_SECTOR_BYTE);
            nfc_open_page_write( // Open the new block at the current position
               g_phys_page_addr[g_curr_dev_id]
            ,  byte_addr );

            // write the sector in the new page
            nf_download(g_page_buffer+byte_addr, (U8)(u16_tmp/16) );

            // write the associated spare zone
            byte_addr=NF_SPARE_POS + (((U16)u8_tmp)*16);
            Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
            Nfc_set_adc( (byte_addr)%256 );
            Nfc_set_adc( (byte_addr)/256 );
            nf_download(g_page_buffer+byte_addr, u8_tmp2 );
            Nfc_set_cmd(NF_PAGE_PROGRAM_CMD);

            g_phys_page_addr[g_curr_dev_id]++;                                                   // update the current physical page of the current device
            g_curr_dev_id++;                                                                     // update the current device
            if( g_curr_dev_id==NF_N_DEVICES ) { g_curr_dev_id=0; }
         }
      }

      // then copy the rest of the logical block
      //
      while( 0!=((U16)g_last_log_sector & (((U16)1<<(S_SHIFT_LOG_BLOCK_SECTOR))-1)) )
      {
         Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);  // open the current device

         g_copy_src=
            nf_block_2_page(g_block_to_kill[g_curr_dev_id])                     // adresse of the beginning of the old block
         +  (LSB0(g_phys_page_addr[g_curr_dev_id])&(SIZE_BLOCK_PAGE -1))        // current offset page in the old and new block
         ;
         nf_copy(g_phys_page_addr[g_curr_dev_id]);
         g_phys_page_addr[g_curr_dev_id]++;

         g_last_log_sector+=SIZE_PAGE_SECTOR;                            // update the current logical sector
         g_curr_dev_id++;                                                          // update the current device
         if( g_curr_dev_id==NF_N_DEVICES ) { g_curr_dev_id=0; }
      }

      // Delete old blocks
      //
      nf_erase_old_blocks();
   }else{
      trace("nf_copy_tail empty??;"); trace_hex32(g_last_log_sector); trace_nl();
   }
   g_last_log_sector= 0xFFFFFFFF;
}



//! Download packets of 16 bytes from RAM to the NAND Flash
//!
//! @param  U8 * datbuf       pointer to RAM
//!         U8 loop           number of 16 bytes packets
//!
//!        <global parameters>
//!
//! @return none
//!
void nf_download(U8 _MEM_TYPE_SLOW_* datbuf, U8 loop)
{
  U8 _MEM_TYPE_SLOW_ * tempbuf = datbuf;
  U8 i = loop;

  while (i != 0)
  {
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     Nf_wr_byte(*(tempbuf)); tempbuf++;
     i--;
  }
}



//! Upload packets of 16 bytes from the NAND Flash to RAM
//!
//! @param  U8 * datbuf       pointer to RAM
//!         U8 loop           number of 16 bytes packets
//!
//! @return none
//!
void nf_upload(U8 _MEM_TYPE_SLOW_* datbuf, U8 loop)
{
  U8 _MEM_TYPE_SLOW_ * tempbuf = datbuf;
  U8 i = loop;

  while (i != 0)
  {
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     *(tempbuf) = Nf_rd_byte(); tempbuf++;
     i--;
  }
}


//! Translate a logical sector to physical parameters
//!
//! This function translates the logical sector address
//! to a physical page number. Then, the block used are
//! swapped with free blocks. The LUT and FBB caches are
//! used for that purpose.
//! Assumption is made that there is at least 2 free entries
//! in the FBB cache: one to swap the LUT blocks, and another
//! one to recycle the FBB block itself (if needed).
//!
//! @param modify_lut    FALSE for simple translation,
//!                      TRUE if LUT shall be modified (write session)
//!        <global parameters>
//!
//! @return none
//!
static void nf_translate( Nf_translate_mode mode )
{
   _MEM_TYPE_MEDFAST_ U8  u8_tmp   = (s_curr_log_sector & (SIZE_PAGE_SECTOR -1) );
   _MEM_TYPE_MEDFAST_ U8  u8_shift;
   _MEM_TYPE_MEDFAST_ U8  u8_curr_page;
   _MEM_TYPE_MEDFAST_ U8  u8_last_page;

   // Here begins the translation:
   // logical sector number (s_curr_log_sector):
   //
   // -> logical block number       (g_log_block_id)
   // -> device number              (g_curr_dev_id)
   // -> position in page           (s_curr_n_byte)
   // -> nb of sectors in first page(s_nb_sectors_step)
   //
   g_log_block_id    =  s_curr_log_sector >> S_SHIFT_LOG_BLOCK_SECTOR;
   g_curr_dev_id     = (s_curr_log_sector >> (G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE)) % NF_N_DEVICES;
   s_curr_n_byte     = ((U16)u8_tmp) << S_SHIFT_SECTOR_BYTE;
   s_nb_sectors_step = SIZE_PAGE_SECTOR - u8_tmp;
   s_nb_sectors_step = Min(s_n_sectors, s_nb_sectors_step); // Adapt if nb sector to read is lower than a page

//   Nfc_put_lba(g_log_block_id);

   Nf_check_fbb( FALSE );
   Nf_check_lut();

   if ( TRUE==g_cache_lut.ctrl.valid )
   {
      if( NF_TRANS_FLUSH==mode )
      {
         if ( TRUE==g_cache_lut.ctrl.dirty )
         {
            nf_cache_lut_flush();
            nf_cache_lut_refill(g_log_block_id);
         }
      }

      if(( g_log_block_id<g_cache_lut.first )
      || ( g_log_block_id>g_cache_lut.last  ))
      { // MISS: need to refill the cache

         if ( TRUE==g_cache_lut.ctrl.dirty ) { nf_cache_lut_flush(); }
         nf_cache_lut_refill(g_log_block_id);
      }
   }
   else { nf_cache_lut_refill(g_log_block_id); } // The cache is not valid. Just re-fill it.



   if ( TRUE==g_cache_fbb.ctrl.valid )
   {
      if( NF_TRANS_FLUSH==mode )
      {
         if ( TRUE==g_cache_fbb.ctrl.dirty )
         {
            nf_cache_fbb_flush( FALSE );
            nf_cache_fbb_refill();
         }
      }
   }
   else { nf_cache_fbb_refill(); }



   u8_curr_page = (g_log_block_id-g_cache_lut.first)*NF_N_DEVICES;
   u8_last_page = (U16)g_cache_fbb.p*NF_N_DEVICES;

   for( u8_tmp=0 ; u8_tmp<NF_N_DEVICES ; u8_tmp++, u8_curr_page++, u8_last_page++)
   {
      if( NF_TRANS_SWAP==mode )
      {
         nf_swap(u8_tmp, u8_curr_page, u8_last_page);
      }

      g_phys_page_addr[u8_tmp] = nf_block_2_page( g_cache_lut.mem[u8_curr_page] ); // ... Then adapt it to page number
   }

   if( NF_TRANS_SWAP==mode )
   {
      g_cache_fbb.p++;
      if( g_cache_fbb.p==(g_cache_fbb.max-1) )
      { // Only 1 remaining entry in FBB cache
         nf_cache_fbb_flush( FALSE ); // No dirty test, since we know that the cache is dirty
         nf_cache_fbb_refill();
      }
   }
   else
   {
      // Build the physical memory address
      //
      u8_shift =                                          // page offset is
         ( s_curr_log_sector >> S_SHIFT_LOG_PAGE_SECTOR ) // convert sector id to page id
      &  ( SIZE_BLOCK_PAGE  -1                          ) // modulo number of phys pages in a phys block
      ;

      for ( u8_tmp=0 ; u8_tmp<NF_N_DEVICES ; u8_tmp++ )
      {
         if ( u8_tmp<g_curr_dev_id ) { g_phys_page_addr[u8_tmp] +=  u8_shift + 1 ; } // already read
         else                        { g_phys_page_addr[u8_tmp] +=  u8_shift     ; } // to be read
      }
   }

   g_next_phys_page_addr=g_phys_page_addr[g_curr_dev_id];
   trace("nf_translate;g_phys_page_addr["); trace_u8(g_curr_dev_id); trace("] = "); trace_hex32(g_phys_page_addr[g_curr_dev_id]); trace_nl();
}



//! Copy a NF page to a new one.
//!
//! This function copies a NF page into a new page.
//! It uses the copy-back command if it is possible.
//!
//! @param g_copy_src          (global) Source page address
//! @param copy_dst            Recipient page address
//!
void nf_copy( U32 copy_dst )
{
   Bool b_copy_fast;
   U8   zone_A, zone_B;

   // Compute the possibility of fast copy
   if( 0 == G_COPY_BACK_CONT )
   {
      b_copy_fast = 0;     // never possible
   }
   else
   {
      if( (1 == G_COPY_BACK_CONT) && (1==G_COPY_BACK_DISCONT) )
      {
         b_copy_fast = 1;  // always possible
      }
      else
      {
         // Check block address
         b_copy_fast = 1;  // by default possible
         if( 1 != G_COPY_BACK_CONT )
         {
/*
            zone_A = (g_copy_src>>G_SHIFT_BLOCK_PAGE) / ((U16)G_N_BLOCKS/G_COPY_BACK_CONT);
            zone_B = (copy_dst  >>G_SHIFT_BLOCK_PAGE) / ((U16)G_N_BLOCKS/G_COPY_BACK_CONT);
*/
            // block_zone = page add / number of page / 1024
            if( Is_nf_2k() )
            {
               // block_zone = (page add >> (G_SHIFT_BLOCK_PAGE + 10)) / (G_N_ZONES/G_COPY_BACK_CONT);
               // block_zone = ((page add >> 16) * G_COPY_BACK_CONT) / G_N_ZONES;
               zone_A = (MSB1(g_copy_src)*G_COPY_BACK_CONT)/G_N_ZONES;
               zone_B = (MSB1(copy_dst  )*G_COPY_BACK_CONT)/G_N_ZONES;
            }else{
               // block_zone = page add >> (G_SHIFT_BLOCK_PAGE + 10) / (G_N_ZONES/G_COPY_BACK_CONT);
               zone_A = ((U8)(g_copy_src>>(G_SHIFT_BLOCK_PAGE + 10)) *G_COPY_BACK_CONT) /G_N_ZONES;
               zone_B = ((U8)(copy_dst  >>(G_SHIFT_BLOCK_PAGE + 10)) *G_COPY_BACK_CONT) /G_N_ZONES;
            }
            if( zone_A != zone_B )
               b_copy_fast = 0;     // no possible
         }
         if( 1 != G_COPY_BACK_DISCONT )
         {
// define mandatory to delete compile error on MODULO 0
#if (NF_GENERIC_DRIVER==TRUE) || (NF_AUTO_DETECT_2KB==TRUE) || (NF_AUTO_DETECT_512B==TRUE)
            zone_A = ((U16)g_copy_src>>G_SHIFT_BLOCK_PAGE) % G_COPY_BACK_DISCONT;
            zone_B = ((U16)copy_dst  >>G_SHIFT_BLOCK_PAGE) % G_COPY_BACK_DISCONT;
#elif ( G_COPY_BACK_DISCONT != 0 )
            zone_A = ((U16)g_copy_src>>G_SHIFT_BLOCK_PAGE) % G_COPY_BACK_DISCONT;
            zone_B = ((U16)copy_dst  >>G_SHIFT_BLOCK_PAGE) % G_COPY_BACK_DISCONT;
#endif
            if( zone_A != zone_B )
               b_copy_fast = 0;     // no possible
         }
      }
   }
      
   // Start copy
   if( !b_copy_fast )
   {
      // copy the page of the old block in buffer
      nfc_open_page_read( g_copy_src, 0 );
      nf_upload(                                // Works by packet of 16 bytes
         g_page_buffer
      ,     ((U16)1<<(G_SHIFT_PAGE_BYTE-4))     // Data zone (Page size / 16)
         +  ((U16)1<<(G_SHIFT_PAGE_BYTE-5-4))); // Spare zone (Page size / 32 / 16)

      // Add LBA markers to help recovery function
      // Need to explain a bit why LBA are written: 
      // nf_copy is called from copy_head and copy_tail.
      // - copy_head: need to write all the LBA of the pages to help recovery finding
      //   where the last sector is written.
      //   Moreover, in case that nf_copy is called from copy_head and source block at page 0
      //   does not contain LBA.
      // - copy_tail: no need to mark the last LBA of the last page to identify the source
      //   block since we use another method
      g_page_buffer[NF_SPARE_POS+NFC_SPARE_OFST_3_BYTE_3] = NFC_OFST_3_DATA_DST;  // 3- [SW] Source block (recovery) (HW capability not used)
      g_page_buffer[NF_SPARE_POS+NFC_SPARE_OFST_6_LBA   ] = MSB(g_log_block_id);  // 6- LBA
      g_page_buffer[NF_SPARE_POS+NFC_SPARE_OFST_6_LBA+1 ] = LSB(g_log_block_id);  // 7- LBA
      if( Is_nf_2k() )
      {
         g_page_buffer[NF_SPARE_POS +16*1 +NFC_SPARE_OFST_6_LBA  ] =
         g_page_buffer[NF_SPARE_POS +16*2 +NFC_SPARE_OFST_6_LBA  ] =
         g_page_buffer[NF_SPARE_POS +16*3 +NFC_SPARE_OFST_6_LBA  ] = MSB(g_log_block_id);  // 6- LBA
         g_page_buffer[NF_SPARE_POS +16*1 +NFC_SPARE_OFST_6_LBA+1] =
         g_page_buffer[NF_SPARE_POS +16*2 +NFC_SPARE_OFST_6_LBA+1] =
         g_page_buffer[NF_SPARE_POS +16*3 +NFC_SPARE_OFST_6_LBA+1] = LSB(g_log_block_id);  // 7- LBA
      }

      // copy the buffer in the page of the new block
      nfc_open_page_write( copy_dst, 0 );
      nf_download(                                // Works by packet of 16 bytes
         g_page_buffer
      ,     ((U16)1<<(G_SHIFT_PAGE_BYTE-4))       // Data zone (Page size / 16)
         +  ((U16)1<<(G_SHIFT_PAGE_BYTE-5-4)));  // Spare zone (Page size / 32 / 16)
      Nfc_set_cmd(NF_PAGE_PROGRAM_CMD);
   }
   else
   {
      nfc_copy_back_init( g_copy_src );

      Nfc_unprotect_all_flash();                              // WP may be actif due to block protection
      Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
      Nfc_set_adc( (NF_SPARE_POS+NFC_SPARE_OFST_3_BYTE_3)%256 );
      Nfc_set_adc( (NF_SPARE_POS+NFC_SPARE_OFST_3_BYTE_3)/256 );
      Nfc_set_adr( LSB0(copy_dst) );
      Nfc_set_adr( LSB1(copy_dst) );
      if ( 3==G_N_ROW_CYCLES )
      {
         Nfc_set_adr( MSB1(copy_dst) );
      }

      // Remove Source block mask
      Nfc_wr_data( NFC_OFST_3_DATA_DST );

      // Add LBA markers to help recovery function
      Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
      Nfc_set_adc( (NF_SPARE_POS+NFC_SPARE_OFST_6_LBA)%256 );
      Nfc_set_adc( (NF_SPARE_POS+NFC_SPARE_OFST_6_LBA)/256 );
      Nfc_wr_data( MSB(g_log_block_id) );
      Nfc_wr_data( LSB(g_log_block_id) );

      if( Is_nf_2k() )
      {
         Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
         Nfc_set_adc( (NF_SPARE_POS + 16*1 +NFC_SPARE_OFST_6_LBA)%256 );
         Nfc_set_adc( (NF_SPARE_POS + 16*1 +NFC_SPARE_OFST_6_LBA)/256 );
         Nfc_wr_data( MSB(g_log_block_id) );
         Nfc_wr_data( LSB(g_log_block_id) );

         Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
         Nfc_set_adc( (NF_SPARE_POS + 16*2 +NFC_SPARE_OFST_6_LBA)%256 );
         Nfc_set_adc( (NF_SPARE_POS + 16*2 +NFC_SPARE_OFST_6_LBA)/256 );
         Nfc_wr_data( MSB(g_log_block_id) );
         Nfc_wr_data( LSB(g_log_block_id) );

         Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
         Nfc_set_adc( (NF_SPARE_POS + 16*3 +NFC_SPARE_OFST_6_LBA)%256 );
         Nfc_set_adc( (NF_SPARE_POS + 16*3 +NFC_SPARE_OFST_6_LBA)/256 );
         Nfc_wr_data( MSB(g_log_block_id) );
         Nfc_wr_data( LSB(g_log_block_id) );
      }
      Nfc_set_cmd(NF_PAGE_PROGRAM_CMD);
   }
}



//! Swap 2 blocks from the LUT and the FBB.
//!
//! This function swaps 2 blocks: one taken into the LUT
//! according to the LBA, and one from the FBB. This is used
//! when modifying a block.
//! The g_block_to_kill[] holds the block address source
//! (LUT) that will be erased after processing.
//!
//! @param dev_id              Device Id of recipient page
//! @param u8_ofst_lut         block offset in LUT
//! @param u8_ofst_fbb         block offset in FBB
//!
//! @return none
//!
void nf_swap(U8 dev_id, U8 u8_ofst_lut, U8 u8_ofst_fbb)
{
   Assert( dev_id      < NF_N_DEVICES  );
   Assert( u8_ofst_lut < CACHE_LUT_SIZE);
   Assert( u8_ofst_fbb < CACHE_FBB_SIZE);
   Assert( g_cache_lut.mem[u8_ofst_lut]  >=g_nf_first_block );
   Assert( g_cache_lut.mem[u8_ofst_lut]  < G_N_BLOCKS       );
   Assert( g_cache_fbb.mem[u8_ofst_fbb]  >=g_nf_first_block );
   Assert( g_cache_fbb.mem[u8_ofst_fbb]  < G_N_BLOCKS       );
   g_block_to_kill[dev_id     ] = g_cache_lut.mem[u8_ofst_lut] ;
   g_cache_lut.mem[u8_ofst_lut] = g_cache_fbb.mem[u8_ofst_fbb] ;
   g_cache_fbb.mem[u8_ofst_fbb] = g_block_to_kill[dev_id]      ;
   trace("g_cache_lut.mem["); trace_u8(u8_ofst_lut); trace("] = "); trace_hex32(g_cache_lut.mem[u8_ofst_lut]); trace_nl();
   trace("g_cache_fbb.mem["); trace_u8(u8_ofst_fbb); trace("] = "); trace_hex32(g_cache_fbb.mem[u8_ofst_fbb]); trace_nl();

   // both FBB and LUT caches becomes invalid
   g_cache_lut.ctrl.dirty=TRUE;
   g_cache_fbb.ctrl.dirty=TRUE;
}



//! This function perform a last copy tail if required, when USB enters suspend or is disconnected
//! This function may be declared in "conf_usb.h" for "Usb_suspend_action()" and "Usb_vbus_off_action()"
//! /!\  "g_last_log_sector" must be initialized to "0xFFFFFFFF" at startup to avoid spurious writes on during USB plug-in
//!
//! => Don't forget to eject (windows) the peripheral from the PC before unpluggin it to avoid uncomplete writing (FAT not actualized)
//!
//! @param  none
//!
//! @return none
//!
void nf_usb_stop(void)
{
#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   if ( 0xFFFFFFFF!=g_last_log_sector )
   {
      Nf_access_signal_on();
      nf_copy_tail();

      if ( TRUE==g_cache_lut.ctrl.dirty )
      { 
         nf_cache_lut_flush();
         nf_cache_lut_refill(0);
      }
      if ( TRUE==g_cache_fbb.ctrl.dirty )
      {
         nf_cache_fbb_flush( FALSE );
         nf_cache_fbb_refill();
      }
      Nf_access_signal_off();
   }

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif
}
#endif // NF_BAD_CONFIG





//_________________D E B U G    F U N C T I O N S __________________________________
// Only required for debug purpose

// Erase all the blocks of the memory
void nf_erase_all_blocks(void)
{
  U16 i_block;

  for (i_block = G_N_BLOCKS ; i_block != 0 ; i_block--)
  {
    nfc_erase_block( nf_block_2_page(i_block), TRUE );
  }
}

//_________________R A M   A C C E S S   F U N C T I O N S _____________________


//! This fonction initialise the memory for a write operation
//! from ram buffer
//!
//!         DATA FLOW is: RAM => NF
//!
//! (sector = 512B)
//! @param addr         Sector address to write
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    nf_ram_2_nf(U32 addr, U8 *ram)
{
  Ctrl_status tmp_bool;
  U16 i;

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   s_n_sectors       = 1;
   s_curr_log_sector = addr;
   s_save_log_addr   = addr + 1;
   g_fatal           = FALSE;
   s_mem             = TRUE;
   s_start           = TRUE;

   // First write operation
   Nf_access_signal_on();
   if(( s_curr_log_sector==g_last_log_sector )                                           // New write is just after to the last write
   && (!(  ( 0==((U16)g_last_log_sector & ( ((U16)1<<(S_SHIFT_LOG_BLOCK_SECTOR)) -1)))   // Not on a logical block boundary
      && ( g_curr_dev_id==0                                                        ))))
   {
      nf_translate( NF_TRANS_NORMAL );
      Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);  // open the current device
      nfc_open_page_write( g_next_phys_page_addr, s_curr_n_byte );
   }
   else
   {      
      nf_open_write( TRUE );
   }
   
   for(i=0;i<512;i++)
   {
      Nf_wr_byte(*ram);
      ram++;
   }

   if (Is_nf_2k())
   {
      nf_update_spare_zone((U8)(1<<(G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE))-s_nb_sectors_step, s_nb_sectors_step);    // update the spare zone once the page has been filled in
   }
   else
   {
     nf_update_spare_zone(0, 1);    // update the spare zone once the page has been filled in
   }
   nf_xfer_update_vars();

   tmp_bool = nf_dfc_write_stop(0);   // ends write operations with "nf_dfc_write_stop(0)" that save the current environnement
   Nf_access_signal_off();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   return tmp_bool;
}

//! This fonction read 1 sector from NF to ram buffer
//!
//!         DATA FLOW is: NF => RAM
//!
//! (sector = 512B)
//! @param addr         Sector address to read
//! @param ram          Ram buffer pointer
//!
//! @return                Ctrl_status
//!   It is ready    ->    CTRL_GOOD
//!   A error occur  ->    CTRL_FAIL
//!
Ctrl_status    nf_nf_2_ram(U32 addr, U8 *ram)
{
  U16 i;

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   s_n_sectors       = 1;
   s_curr_log_sector = addr;
   s_save_log_addr   = addr + 1;
   g_fatal           = FALSE;
   s_mem             = TRUE;
   s_start           = TRUE;

   // First read operation
   Nf_access_signal_on();
   nf_open_read(TRUE);
   Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);
   nfc_open_page_read( g_phys_page_addr[g_curr_dev_id], s_curr_n_byte );
   s_nb_sectors_step = (U8)                              // determine number of sectors to be read
                       (Min(                             // on this first page
                             1,
                             ((1<<(G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE))-(s_curr_n_byte >> 9))
                           )
                       );
   
   Disable_interrupt();  
   for(i=0;i<512;i++)
   {
      *ram=Nf_rd_byte();
      ram++;
   }
   Enable_interrupt();  
   nf_xfer_update_vars();
   Nf_access_signal_off();

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif

   return CTRL_GOOD;   
}

