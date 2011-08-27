/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the low level functions for the Nand-Flash Controller.
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
#define  _nfc_drv_c_

#include  "config.h"          // general project configuration file
#include  "conf_nf.h"         // global NAND Flash configuration file
#include  "nf.h"              // NAND Flash informations (structure, parameters)
#include  "nf_drv.h"          // declarations of constants, low levels routines, public prototypes

//_____ D E C L A R A T I O N ______________________________________________
//

#ifndef __GNUC__
  __no_init volatile xdata Byte nf_send_cmd  At(NF_CMD_LATCH_ENABLE_ADD);  // Command
  __no_init volatile xdata Byte nf_send_add  At(NF_ADD_LATCH_ENABLE_ADD);  // Address
  __no_init volatile xdata Byte nf_data      At(NF_ADDRESS_CMD_DATA);      // Data
#else
  volatile unsigned char nf_send_cmd  __attribute__ ((section (".nf_cmd")));
  volatile unsigned char nf_send_add  __attribute__ ((section (".nf_add")));
  volatile unsigned char nf_data      __attribute__ ((section (".nf_dat")));
#endif

#ifndef NF_XMCR_MODULE_SHARED
   #warning NF_XMCR_MODULE_SHARED must be defined to ENABLE if XMCR module is shared with other hardware peripherals
   #define NF_XMCR_MODULE_SHARED       DISABLED
#endif

#define     BAD_BLOCK_OFFSET     0
#ifndef __GNUC__
  extern   U16 bad_block_table[100];
#else
  U16 bad_block_table[100];
#endif

#if( NF_BAD_CONFIG==(FALSE) )


//_____ D E C L A R A T I O N ______________________________________________

void nfc_select_dev( U8 dev )
{
   if(0==dev)
   {
      Nandflash1_unselect();
      Nandflash0_select();
   }else{
      Nandflash0_unselect();
      Nandflash1_select();
   }
}


//! Tests the Nand Flash configuration
//!
//! The function verifies that the NF connected to device are
//! properly declared in conf_nf.h.
//!
//! @param none
//!
//! @return The number of device connected and corresponding
//! to NF identifiers.
//!
#if (NF_AUTO_DETECT_2KB==FALSE) && (NF_AUTO_DETECT_512B==FALSE)
U8 nfc_check_type( U8 nb_dev )
{
   U8 i_dev;
   if( 2 < nb_dev )
      nb_dev = 2; // Only 1 or 2 for this driver

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif
   nfc_init(        nb_dev, 0 );
   nfc_reset_nands( nb_dev ); // Reset all the NF devices

   // Test NF configuration
   //
   for( i_dev=0 ; i_dev<nb_dev ; i_dev++ )
   {
      Nfc_action( NFC_ACT_DEV_SELECT, i_dev);
      nfc_wait_busy();
      Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);
      Nfc_set_cmd(NF_READ_ID_CMD);
      Nfc_set_adc( 0 );

      //trace("Device ID: ");
	  //trace_hex( Nfc_rd_data_fetch_next() ); trace(" ");
      //trace_hex( Nfc_rd_data_fetch_next() ); trace(" ");
      //trace_hex( Nfc_rd_data_fetch_next() ); trace(" ");
      //trace_hex( Nfc_rd_data_fetch_next() ); trace("\n\r");

      if(( Nfc_rd_data_fetch_next()!=G_DEV_MAKER  )
      || ( Nfc_rd_data_fetch_next()!=G_DEV_ID     ))
      {
         return i_dev;
      }
      if( G_CE_TOGGLE )
      {
         // disable CE Low
         Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_NOP);
      }
   }

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif
   return nb_dev;
}
#endif


//! Reset all the NF devices.
//!
//! @param none
//!
void nfc_reset_nands( U8 nb_dev )
{
   U8 i_dev;
   Mcu_set_sfr_page_nfc();
   for( i_dev=0 ; i_dev<nb_dev ; i_dev++ )
   {
       trace("-Nfc_action\n\r");
	   Nfc_action(NFC_ACT_DEV_SELECT, i_dev);
      // The wait is mandatory here since the function is used to wait any
      // pending internal programmation (Cache Program cmd).
	  nfc_wait_busy();
      Nfc_set_cmd(NF_RESET_CMD);
      nfc_wait_busy();
   }
}


//! Enable the XMCR (Extending Memory Module) of the AVR to drive the NAND Flash
//!


void nf_XMCR_enable( void )
{
#if (NF_CLE_ALE_MANUAL == ENABLED)
  XMCRB |= ((1<<XMM2) | (1<<XMM1) | (1<<XMM0));   // limit XRAM interface to A7 (release PC0..7)
#else
  XMCRB |= ((1<<XMM2) | (1<<XMM1));                // limit XRAM interface to A9 (release PC2..7)
#endif  
  XMCRA |= (1<<SRE);                  // enable the external memory
}


//! Disable the XMCR module of the AVR, to allow access to others peripherals that may be connected on this same bus
//!
void nf_XMCR_disable( void )
{
  Nandflash0_unselect();
  Nandflash1_unselect();
  XMCRA &= ~(1<<SRE);  // disable the external memory
}






//! Check the status of the selected device.
//!
//! @return a status:
//!           PASS if the status is PASS;
//!           FAIL if the status is FAIL
//!
Status_bool nfc_check_status( void )
{
   Mcu_set_sfr_page_nfc();
   nfc_wait_busy(); // Send a status command and wait the completion of the last command
   if ( (Nfc_rd_data()&NF_MASK_STATUS_FAIL)==0 ) { return PASS; } // I/O 0   Pass:0  Fail:1
   else                                          { return FAIL; }
}

//! Opens a page for read. The function will adapt the commands according to the
//! type of flash memory. The busy is polled at the end of the function.
//!
//! @param page_addr          absolute page address of the block
//! @param byte_addr          relative byte address inside the page.
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The NF device should have been selected before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
// TODO: Improve the argument list: 6 bytes are needed
void nfc_open_page_read( U32 page_addr, U16 byte_addr)
{
   Mcu_set_sfr_page_nfc();
   nfc_wait_busy();
   Nfc_open_page_read( page_addr, byte_addr);
}



//! Opens a page for write. The function will adapt the commands according to the
//! type of flash memory.
//!
//! @param page_addr          absolute page address of the block
//! @param byte_addr          relative byte address inside the page.
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The NF device should have been selected before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
void nfc_open_page_write( U32 page_addr, U16 byte_addr)
{
   Mcu_set_sfr_page_nfc();
   Nfc_open_page_write( page_addr, byte_addr);
}



//! Mark a block as 'invalid' by clearing it entirely.
//!
//! @param page_addr          absolute page address of the block
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The device which holds this bad block should have been selected
//! before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
void nfc_mark_bad_block(U32 page_addr)
{
   U8  n_bytes;
   U8  i_byte;
   U8  i_page;

   Mcu_set_sfr_page_nfc();

   n_bytes= ( Is_nf_512() )
   ?  16  // 512B page access
   :  64  // 2KB  page access
   ;

   // Erasing the block is mandatory to prevent partial programming
   // (some 512B NF does support partial prog, but not after a copy back command).
   nfc_erase_block( page_addr, TRUE );
   for ( i_page=(U8)1<<G_SHIFT_BLOCK_PAGE ; i_page!=0 ; i_page--, page_addr++ )
   {
      nfc_open_page_write( page_addr, NF_SPARE_POS-8 );
      Nfc_wr_data('A'); Nfc_wr_data('t');
      Nfc_wr_data('m'); Nfc_wr_data('e');
      Nfc_wr_data('l'); Nfc_wr_data(' ');
      Nfc_wr_data(' '); Nfc_wr_data(' ');
      for ( i_byte=n_bytes ; i_byte!=0 ; i_byte-=4 )
      {
         Nfc_wr_data(0);
         Nfc_wr_data(0);
         Nfc_wr_data(0);
         Nfc_wr_data(0);
      }
      Nfc_set_cmd(NF_PAGE_PROGRAM_CMD); // Confirm programmation
   }
}



//! Erases a block.
//!
//! The erase will be done only if the block is not bad
//!
//! @param page_addr          absolute page address of the block
//! @param force_erase        TRUE forces erasing, FALSE erases the block (if not bad)
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The device which holds the block to delete should have been selected
//! before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
void nfc_erase_block( U32 page_addr, U8 force_erase )
{
   Mcu_set_sfr_page_nfc();
   if (FALSE == force_erase)
   {
      nfc_open_page_read( page_addr, NF_SPARE_POS + G_OFST_BLK_STATUS );
      if( (Nfc_rd_data() != 0xFF) ) return;    // The block is bad. We can not erase it
   }
   nfc_wait_busy();
   Nfc_unprotect_all_flash();                    // WP may be actif due to block protection
   Nfc_set_cmd (NF_BLOCK_ERASE_CMD);             // Auto Block Erase Setup
   Nfc_set_adr( LSB0(page_addr) );
   Nfc_set_adr( LSB1(page_addr) );
   if (3 == G_N_ROW_CYCLES)
   {
      Nfc_set_adr( MSB1(page_addr) );
   }
   Nfc_set_cmd(NF_BLOCK_ERASE_CONFIRM_CMD);      // Erase command
}



//! Reads the number spare bytes specified and stores them in a array.
//!
//! @param p_byte             pointer on the array in which are stored the spare bytes.
//! @param n_byte             number of spare bytes to read.
//! @param page_addr          absolute page address of the block.
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The NF device should have been selected before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
void nfc_read_spare_byte(
   U8 _MEM_TYPE_SLOW_ * p_byte
,  U8  n_byte
,  U32 page_addr)
{
   U8  i;

   trace("Page: "); trace_hex32(page_addr); trace_nl();
   trace("Bytes: "); trace_hex(n_byte); trace_nl();

   Mcu_set_sfr_page_nfc();
   nfc_open_page_read( page_addr, NF_SPARE_POS);

   for ( i=0 ; i!=n_byte ; i++ )
   {
      p_byte[i] = Nfc_rd_data_fetch_next();
	  trace_hex(p_byte[i]);
   }
   
   trace_nl();
}

//! Tests the true busy. Note that we test twice the ready, since there is
//! an hardware minimum requirement between the end of the busy and the first
//! read cycle. Since the busy is not wired, the ready is tested twice.
//!
void nfc_wait_busy( void )
{
   register int Reg;
   Nfc_set_cmd(NF_READ_STATUS_CMD);
   // trace("--Nfc_rd_status ");    trace_hex(Nfc_rd_status());    trace("\n\r");
   Reg = Nfc_rd_status();
   if( Is_nf_2k() )
   {
      if( G_CACHE_PROG )
      {
         while( (Nfc_rd_status() & NF_MASK_STATUS_T_RDY_2KB )==0 );
         while( (Nfc_rd_status() & NF_MASK_STATUS_T_RDY_2KB )==0 );
      }
      else
      {
         while( (Nfc_rd_status() & NF_MASK_STATUS_READY     )==0 );
         while( (Nfc_rd_status() & NF_MASK_STATUS_READY     )==0 );
      }
   }
   if( Is_nf_512() )
   {
      while( (Nfc_rd_status() & NF_MASK_STATUS_T_RDY_512B )==0 );
      while( (Nfc_rd_status() & NF_MASK_STATUS_T_RDY_512B )==0 );
   }
}





#if (NF_DETECTION_ID==ENABLE) || (NF_AUTO_DETECT_2KB==TRUE) || (NF_AUTO_DETECT_512B==TRUE)

//! Read the ID of the Nand-Flash
//!
//! @param read_id_cmd  Read_id command (NF_READ_ID_CMD, NF_READ_ID2_CMD)
//! @param nf_num       Nand Flash number
//!
//! @return :
//!   MSB0(ret) (MSB) is the Maker Code,
//!   MSB1(ret) is the Device Id,
//!   MSB2(ret) is 3rd byte returned,
//!   MSB3(ret) (LSB) is 4th byte returned.
//!
//! @pre <code>nf_init()</code> should have been called before.
//!
U32 nfc_read_id( U8 read_id_cmd, U8 nf_num )
{
   U32 ret;

   Mcu_set_sfr_page_nfc();
   Nfc_action(NFC_ACT_DEV_SELECT, nf_num);
   nfc_wait_busy();
   Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);
   Nfc_set_cmd (read_id_cmd);
   Nfc_set_adc( 0 );

   MSB0(ret)= Nfc_rd_data_fetch_next(); // Maker Code
   MSB1(ret)= Nfc_rd_data_fetch_next(); // Device Id
   MSB2(ret)= Nfc_rd_data_fetch_next(); // extra
   MSB3(ret)= Nfc_rd_data_fetch_next(); // extra (Multi Plane Support)

   Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_NOP);
   return ret;
}


//! Check the status Ready/Busy of the Nand Flash
//! @return Bool
//!    TRUE   -> The Nand Flash is ready and connected
//!    FALSE  -> The Nand Flash must be no connected (timeout)
//!
static Bool nfc_nf_is_ready( void )
{
   register U8 Reg;
   U8 u8_timeout;

   Nfc_set_cmd( NF_READ_STATUS_CMD );  // send status for each read, because the NF must be in reset sequence
   Reg = Nfc_rd_status();            // active first read

   for (u8_timeout=NF_MAX_RB_TIMEOUT ; u8_timeout!=0 ; u8_timeout--)
   {
      if(( (Nfc_rd_status() & NF_MASK_STATUS_READY) !=0 )    // the busy pin is not tested, and the bit ready may be wrong penddind the rise of busy pin
      && ( (Nfc_rd_status() & NF_MASK_STATUS_READY) !=0 ) )  // To not read a wrong status, we check the status after 6 cycles (300ns)
      {
            return TRUE;  // NF READY
      }
   }
   return FALSE;          // TIMEOUT
}


//! Read the ID of the Nand-Flash and update the global variable
//!
//! @return :
//!   nf index of listing "nf_listing"
//!   otherwise : NO_NF_CONNECTED or NF_UNKNOW
//!
U8  nfc_detect( void )
{
   U32   u32_nf_ids;
   U8    u8_i, u8_conf;

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_enable();
#endif

   // Init the Nand Flash Controller
   trace("nfc_init\n\r");
   nfc_init(        NF_MAX_DEVICES, 0 );
   trace("nfc_reset_nands\n\r");
   nfc_reset_nands( NF_MAX_DEVICES ); // Reset all the NF devices

   // Check the presence of device 0
   if ( FALSE == nfc_nf_is_ready() )
   {
      #if (NF_XMCR_MODULE_SHARED == ENABLED)
         nf_XMCR_disable();
      #endif
      return NO_NF_CONNECTED;
   }

   // Read the Nand Flash IDs of device 0
   u32_nf_ids = nfc_read_id( NF_READ_ID_CMD, 0 );

   // Identify the Nand Flash (device 0)
   for( u8_i=0 ; u8_i < (sizeof(nf_list_id)/sizeof(St_nf_id)) ; u8_i++)
   {
      if((nf_list_id[u8_i].manuf == MSB0(u32_nf_ids))
      && (nf_list_id[u8_i].dev   == MSB1(u32_nf_ids)))
         break; // here, ID is know
   }
   if( u8_i == (sizeof(nf_list_id)/sizeof(St_nf_id)) )
   {
      #if (NF_XMCR_MODULE_SHARED == ENABLED)
         nf_XMCR_disable();
      #endif
      return NF_UNKNOW;
   }

   // Set NF configuration parameters for initialisation and access
#if (NF_GENERIC_DRIVER==TRUE)
#  error Test me...
   g_shift_page_byte    =;
   g_shift_block_page   =;
#endif

#if (NF_GENERIC_DRIVER==TRUE) || (NF_AUTO_DETECT_2KB==TRUE) ||(NF_AUTO_DETECT_512B==TRUE)

   // Record info
   u8_conf     =  nf_list_id[u8_i].conf;
   g_dev_maker =  MSB0(u32_nf_ids); // Device maker
   g_dev_id    =  MSB1(u32_nf_ids); // Device ID

   // Search the number of block of device
   for( u8_i=0 ; u8_i < (sizeof(nf_list_link_id_block)/sizeof(St_nf_link_id_block)) ; u8_i++)
   {
      if( nf_list_link_id_block[u8_i].dev_id == g_dev_id )
         break; // ID found
   }
   if( u8_i == (sizeof(nf_list_link_id_block)/sizeof(St_nf_link_id_block)) )
      while(1);   // Error in NF definition

   g_n_zones            =  nf_list_link_id_block[u8_i].nb_zones;
#if (NF_AUTO_DETECT_2KB==TRUE)
   if( 1 == g_n_zones )
      g_n_row_cycles    =  2;
   else
      g_n_row_cycles    =  3;
#endif
#if (NF_AUTO_DETECT_512B==TRUE)
   if( 2 >= g_n_zones )
      g_n_row_cycles    =  2;
   else
      g_n_row_cycles    =  3;
#endif
   g_n_blocks           =  g_n_zones*1024L;

   g_copy_back_cont     = nf_list_conf[u8_conf].copy_back_cont   ;
   g_copy_back_discont  = nf_list_conf[u8_conf].copy_back_discont;
   g_cache_program      = nf_list_conf[u8_conf].cache_program    ;
   g_ce_toggle             = nf_list_conf[u8_conf].ce_toggle;
/*   
   g_clock_dfc_nfc      = (nf_list_conf[u8_conf].dfc_nfc_clock<<5) & MSK_DNFCKS;

   Mcu_set_sfr_page_nfc();
   Nfc_set_read_timing((U8)nf_list_conf[u8_conf].timing_read );
   if( !g_ce_toggle )
   {
      // Enable CE low
      Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);
   }
*/
#endif

#if (NF_XMCR_MODULE_SHARED == ENABLED)
   nf_XMCR_disable();
#endif
   return u8_i;
}

#endif

//! Prepare a copy-back session
//!
//! @param page_addr          absolute source page address of the block
//!
//! @pre <code>nf_init()</code> should have been called before.
//!
void nfc_copy_back_init( U32 page_addr )
{
   Mcu_set_sfr_page_nfc();
   nfc_wait_busy();
   Nfc_unprotect_all_flash(); // WP may be actif due to block protection
   Nfc_set_cmd(NF_READ_CMD);
   Nfc_set_adc( 0 );
   Nfc_set_adc( 0 );
   Nfc_set_adr( LSB0(page_addr) );
   Nfc_set_adr( LSB1(page_addr) );
   if ( 3==G_N_ROW_CYCLES )
   {
      Nfc_set_adr( MSB1(page_addr) );
   }
   Nfc_set_cmd(NF_COPY_BACK_CMD);
   nfc_wait_busy();
}


//! Launch a copy-back session
//!
//! @param page_addr          absolute recipient page address of the block
//!
//! @pre <code>nf_init()</code> should have been called before.
//!
#if (0)
void nfc_copy_back_conf( U32 page_addr )
{
   Mcu_set_sfr_page_nfc();
   nfc_wait_busy();
   Nfc_unprotect_all_flash();                              // WP may be actif due to block protection
   Nfc_set_cmd(NF_RANDOM_DATA_INPUT_CMD);
   Nfc_set_adc( 0 );
   Nfc_set_adc( 0 );
   Nfc_set_adr( LSB0(page_addr) );
   Nfc_set_adr( LSB1(page_addr) );
   if ( 3==G_N_ROW_CYCLES )
   {
      Nfc_set_adr( MSB1(page_addr) );
   }
   Nfc_set_cmd(NF_PAGE_PROGRAM_CMD);
}
#endif

#endif // NF_BAD_CONFIG

//! Display the 2 first pages of a block.
//!
//! @param block_addr          Physical block address
//! @param dev_id              Device number
//!
//! @pre <code>nf_init()</code> should have been called before.
//!
#if 0
void nfc_print_block(U16 block_addr, U8 dev_id)
{
   _MEM_TYPE_SLOW_ U32 page_addr=(U32)block_addr*((U8)1<<G_SHIFT_BLOCK_PAGE);
   _MEM_TYPE_SLOW_ U16 n_bytes;
   _MEM_TYPE_SLOW_ U16 i_byte;
   _MEM_TYPE_SLOW_ U8  i_page;

   Mcu_set_sfr_page_nfc();
   trace("\n\rDisplay block 0x");
   trace_hex( MSB(block_addr) );
   trace_hex( LSB(block_addr) );

   n_bytes= ( Is_nf_512() )
   ?  512   // 512B page access
   :  2048  // 2KB  page access
   ;
   Nfc_action(NFC_ACT_DEV_SELECT, dev_id);
   //for ( i_page=(U8)1<<G_SHIFT_BLOCK_PAGE ; i_page!=0 ; i_page--, page_addr++ )
   for ( i_page=0 ; i_page<64 ; i_page++, page_addr++ )
   {
      trace("\n\rOpening page 0x");
      trace_hex( MSB0(page_addr) );
      trace_hex( MSB1(page_addr) );
      trace_hex( MSB2(page_addr) );
      trace_hex( MSB3(page_addr) );
      #if 0
      nfc_open_page_read( page_addr, 0 );
      i_byte=Nfc_rd_data_fetch_next();
      for ( i_byte=0 ; i_byte<n_bytes ; )
      {
         if      ( !(i_byte%32) )
         {
            trace("\n\r0x");
            trace_hex( MSB(i_byte) );
            trace_hex( LSB(i_byte) );
            trace(" 0x");
         }
         else if ( !(i_byte%16) ) trace("   ");
         else if ( !(i_byte% 8) ) trace(" ");
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         i_byte+=4;
      }
      #else
      nfc_open_page_read( page_addr, n_bytes );
      i_byte=Nfc_rd_data_fetch_next();
      #endif
      trace("\n\rSpare zone: 0x");
      for ( i_byte=4*4 ; i_byte!=0 ; i_byte-- )
      { // discard spare zone
         if( i_byte%4==0 ) trace_nl();
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
      }
      trace("\n\r");
   }
   trace("\n\rOther way to access spare zone: 0x");
   page_addr=(U32)block_addr*((U8)1<<G_SHIFT_BLOCK_PAGE);
   nfc_open_page_read( page_addr, NF_SPARE_POS );
   i_byte=Nfc_rd_data_fetch_next();
   {
      for ( i_byte=4*4 ; i_byte!=0 ; i_byte-- )
      { // discard spare zone
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
         trace_hex( Nfc_rd_data_fetch_next() );
      }
      trace("\n\r");
   }
}
#endif
