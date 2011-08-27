/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the high level management for nand-flash
//!
//!  memory devices which are rarely used. The code is put in a bank
//!  in order to save code space.
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

// TODO: remplacer les s_shift_xxx en (1<<s_shift_xxx): et ainsi faire des DIV/MUL plutot que des shift
// TODO: decomposer s_phys_page_addr en s_phys_block et s_phys_page (offset dans le block)

//_____  I N C L U D E S ___________________________________________________

#include "config.h"
#include "conf_nf.h"
#include "nf.h"              // NAND Flash informations (structure, parameters)
#include "nf_drv.h"
#include "nf_mngt.h"
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


//_____ P R I V A T E    D E C L A R A T I O N _____________________________

#if (NF_GENERIC_DRIVER==TRUE) || (defined NF_AUTO_DETECT_2KB) ||(defined NF_AUTO_DETECT_512B)
extern _MEM_TYPE_SLOW_       U8   g_n_zones               ; // number of zones (=1024 blocks) per device
extern _MEM_TYPE_SLOW_       U16  g_n_blocks              ; // number of blocks per device
#endif
extern _MEM_TYPE_SLOW_ U8   g_page_buffer[NF_FULL_PAGE_BUFFER_SIZE] ; // Used to bufferize a page
extern _MEM_TYPE_BIT_  bit  g_nf_init                               ; // Boolean set when driver is initialized
extern _MEM_TYPE_SLOW_ U16  g_last_sub_lut_log_sz                   ; // Size of the last sub-LUT. Unit in number of logical blocks
extern _MEM_TYPE_SLOW_ U16  g_sub_lut_log_sz                        ; // Size of the sub-LUT. Unit in number of logical blocks
                       Bool g_is_found_lut                          ;
                       Bool g_is_found_fbb                          ;
extern                 Bool g_fatal                                 ; // Used in LUT/FBB building and ECC management...
       _MEM_TYPE_SLOW_ U8   g_n_real_sub_lut;
       _MEM_TYPE_SLOW_ U16  g_curr_block_addr[    NF_N_DEVICES]; // holds the last block address for each device
       _MEM_TYPE_SLOW_ U8   g_byte[16]                              ; // Buffer which holds spare bytes
extern _MEM_TYPE_SLOW_ U8   g_n_sub_lut                             ; // Holds the number of sub-Lut
extern _MEM_TYPE_SLOW_ U16  g_lut_block_addr [ N_SUBLUT ]           ; // LUT block address
extern _MEM_TYPE_SLOW_ U8   g_lut_block_index[ N_SUBLUT ]           ; // LUT index, unit in (LUT size/page size)
static _MEM_TYPE_SLOW_ U8   s_nfd_rev                               ; // Nand Flash Driver revision;
extern _MEM_TYPE_SLOW_ U16  g_nf_first_block                        ; // Block addr of the beginning of dynamic area
static _MEM_TYPE_SLOW_ U8   s_n_quarantine_blocks[NF_N_DEVICES]     ; // count quarantine blocks (ECC error discovered in them, but block not yet bad) for each device
static _MEM_TYPE_SLOW_ U16  s_n_invalid_blocks[   NF_N_DEVICES]     ; // count invalid blocks (bad, management, LUT, ...) for each device
extern _MEM_TYPE_SLOW_ U16  g_n_export_blocks                       ; // Number of physical blocks exported for mass-storage use
extern _MEM_TYPE_SLOW_ U16  g_n_free_blocks                         ; // Number of free physical blocks
extern _MEM_TYPE_SLOW_ U16  g_fbb_block_addr                        ; // Free-Blocks block address
extern _MEM_TYPE_SLOW_ U8   g_fbb_block_index                       ; // Free-Blocks block index
extern _MEM_TYPE_SLOW_ U32  g_last_log_sector                       ; // Holds the last logical sector number
extern _MEM_TYPE_SLOW_ U32  g_copy_src                              ; // Used to copy NF pages (source page)
extern _MEM_TYPE_SLOW_ U16  g_block_to_kill[ NF_N_DEVICES]          ; // Holds the blocks number which will be erased
extern _MEM_TYPE_FAST_ U32  g_phys_page_addr[NF_N_DEVICES]          ; // Holds the current phys page number for each device
extern _MEM_TYPE_FAST_ U8   g_curr_dev_id                           ; // Holds the current device number that is used

extern _MEM_TYPE_MEDFAST_ U16 g_log_block_id                        ; // Logical Block address

extern _MEM_TYPE_SLOW_ Cache_lut g_cache_lut; // LUT cache
extern _MEM_TYPE_SLOW_ Cache_fbb g_cache_fbb; // Free-Blocks block cache


#if (ERASING_ALL==ENABLE)
static void        ut_nfc_erase_all( void );
#endif


static void        nf_init_buffer(      void );
static Status_bool nf_scan(             void );
static Status_bool nf_rebuild(          void );
static Bool        is_nf_invalid(       void );

static U16         nf_fetch_free_block( U8 i_dev );

static U8          nf_refine_index( U16 block_addr, U8 inc, U8 pattern) ;


//! Clears the internal buffer.
//!
//! @return nothing
//!
static void nf_init_buffer( void )
{
   U16 u16_tmp;
   for ( u16_tmp=NF_FULL_PAGE_BUFFER_SIZE ; u16_tmp!=0 ; u16_tmp-=2 )
   {
      g_page_buffer[u16_tmp-1]=0xFF;
      g_page_buffer[u16_tmp-2]=0xFF;
   }
}



//! Initializes the nand flash memory driver.
//!
//! The device identification is performed to initialize the
//! driver accordingly
//!
//! @param none
//!
//! @return none
//!
void nf_init ( void )
{
   trace("NF init\n\r");
   g_nf_init=FALSE;
//   s_pending_write=FALSE;

#if (NF_GENERIC_DRIVER==TRUE)
#error Check this init...
   g_n_zones             = NF_N_ZONES;
   g_n_blocks            = NF_N_BLOCKS;
   g_shift_block_page    = NF_SHIFT_BLOCK_PAGE;
   g_shift_page_byte     = NF_SHIFT_PAGE_BYTE;
   s_shift_sector_byte   = NF_SHIFT_SECTOR_BYTE;
   g_n_row_cycles        = NF_N_ROW_CYCLES;

   if ( Is_nf_2k() ) // 2KB pages
   {
      g_ofst_blk_status     = 0;
   }
   if ( Is_nf_512() ) // 512B pages
   {
      g_ofst_blk_status     = 5;
   }

   s_shift_log_page_sector  = G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE + NF_SHIFT_N_DEVICES;
   s_shift_log_block_sector = s_shift_log_page_sector + G_SHIFT_BLOCK_PAGE;
#endif

   g_cache_lut.ctrl.valid = FALSE; g_cache_lut.ctrl.dirty = FALSE;
   g_cache_fbb.ctrl.valid = FALSE; g_cache_fbb.ctrl.dirty = FALSE;
   g_last_log_sector= 0xFFFFFFFF;
}


//! Ensure that the memory is in a good state before starting to use it
//!
//! The function will scan the memory, test if the memory is valid, clean it
//! if it has to and rebuild all the management blocks. This function shall be
//! called prior to any use of the memory after a power-up.
//!
//! @param none
//!
//! @return a status:
//!           PASS if the command has been succesfully executed;
//!           FAIL else
//!
Status_bool nf_verify_resume( void )
{
   U8 u8_nb_loop;
   Bool status_bool;

   trace("nf_verify_resume");    trace_nl();

#if (ERASING_ALL==ENABLE)
   ut_nfc_erase_all();
#endif
   
   status_bool = nf_scan();

   if(( PASS!=status_bool )
   || ( is_nf_invalid()   )  // The NF is not cleanly built
   ) {
      // The NF seems not cleanly built, or not built at all.
      //
      u8_nb_loop = 0;
      while( 1 )
      {
         u8_nb_loop++;
         if( u8_nb_loop > 2 )
         {
            status_bool=FAIL;
            break;  // Error NF access or control
         }
         nf_cleanup_memory();
         if( PASS != nf_scan() )
            continue;
         if( PASS != nf_rebuild() )
            continue;
         status_bool = PASS;
         break;
      }
   }
   if (status_bool==PASS)
   {
      g_nf_init = TRUE;
      Nf_check_lut();
      Nf_check_fbb( FALSE );
   }

   return status_bool;
}



//! Cleanup the memory by erasing all the management blocks.
//!
//! The sub-LUT blocks, the recovery block and the free-blocks block
//! will be erased on any devices.
//!
//! @param none
//!
void nf_cleanup_memory(void)
{
   U8   i_dev  =0;
   U16  i_block=0;
   U8   block_valid;
   U8   block_id;

   // Scan all the devices and looks for:
   // - the sub-LUT
   // - the recovery block
   // - the free-blocks block
   //
   for( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      // Select the devices
      //
      Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

      for( i_block=g_nf_first_block ; i_block<G_N_BLOCKS ; i_block++ )
      {

         nfc_open_page_read( nf_block_2_page(i_block), NF_SPARE_POS);
         block_valid = Nfc_rd_data_fetch_next();
         block_id    = Nfc_rd_data()           ;

         if ( block_valid!=0xFF )
         {
            continue; // The block is bad
         }

         if(( NFC_BLK_ID_SUBLUT==block_id )
         || ( NFC_BLK_ID_FBB   ==block_id ))
         {
            nfc_erase_block( nf_block_2_page(i_block), TRUE ) ;
            if ( FAIL==nfc_check_status() )
            {
               nfc_mark_bad_block( nf_block_2_page(i_block) );
            }
         }
      } // for( i_block...
   } // for( i_dev...
} // nf_cleanup_memory



//! Scan the memory and looks for sub-LUT, free-blocks block and recovery blocks
//!
//! @param none
//!
//! @return a status:
//!           PASS if the scan has been succesfully executed;
//!           FAIL if the scan encountered any problem
//!
static Status_bool nf_scan( void )
{
   U8          i_dev  =0;
   U16         i_block=0;
   U8          n_quarantine_blocks=0;
   U16         n_invalid_blocks=0;

   g_last_sub_lut_log_sz =(U16)-1;
   g_sub_lut_log_sz      =(U16)NF_SUBLUT_SIZE/NF_N_DEVICES;


   // Initialize the recovery structure. This should be done by the startup !
   //
   g_is_found_lut  =FALSE;
   g_is_found_fbb  =FALSE;
   g_fatal         =FALSE;
   g_n_real_sub_lut=0;

   trace("nf_scan");    trace_nl();
   
   // Scan all the devices and looks for:
   // - the sub-LUT blocks
   // - the recovery block
   // - the free-blocks block
   //
   for( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      n_invalid_blocks   = 0;
      n_quarantine_blocks= 0;
      g_curr_block_addr[i_dev]= G_N_BLOCKS -1; // points on the last block

      trace("Device "); trace_hex(i_dev); trace("\n\r");
      Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

      for( i_block=g_nf_first_block ; i_block<G_N_BLOCKS ; i_block++ )
      {
         nfc_read_spare_byte( g_byte, 16, nf_block_2_page(i_block) );
         if ( g_byte[G_OFST_BLK_STATUS]!=0xFF )
         {
            n_invalid_blocks +=1 ;
            trace_hex16(i_block); trace(" ("); trace_u32(i_block); trace("): bad Block\n\r");
            continue; // The block is bad
         }

         if(( g_byte[NFC_SPARE_OFST_1_BLK_ID]!=NFC_BLK_ID_SUBLUT     )
         && ( g_byte[NFC_SPARE_OFST_1_BLK_ID]!=NFC_BLK_ID_FBB        )
         && ( g_byte[NFC_SPARE_OFST_1_BLK_ID]!=NFC_BLK_ID_QUARANTINE )
         && ( g_byte[NFC_SPARE_OFST_1_BLK_ID]!=NFC_BLK_ID_DATA       ))
         {
            n_invalid_blocks +=1;
            trace_hex16(i_block); trace(" ("); trace_u32(i_block); trace("): Unknown\n\r");
            continue;
         }
         else if ( g_byte[NFC_SPARE_OFST_1_BLK_ID]==NFC_BLK_ID_QUARANTINE )
         {
            n_quarantine_blocks +=1;
            trace_hex16(i_block); trace(" ("); trace_u32(i_block); trace("): Quarantine\n\r");
            continue;
         }
         else if ( g_byte[NFC_SPARE_OFST_1_BLK_ID]==NFC_BLK_ID_SUBLUT )
         {
            n_invalid_blocks +=1;
            if ( i_dev==S_MNGT_DEV )
            {
               U8 sub_lut_id                   = g_byte[NFC_SPARE_OFST_2_BYTE_2];
               g_n_sub_lut                     = g_byte[NFC_SPARE_OFST_4_BYTE_4];
               g_is_found_lut = TRUE;
               g_n_real_sub_lut++;
               g_lut_block_addr[sub_lut_id] = i_block;
               if ( sub_lut_id==(g_n_sub_lut-1) )
               {
                  MSB(g_last_sub_lut_log_sz)= g_byte[NFC_SPARE_OFST_6_LBA  ];
                  LSB(g_last_sub_lut_log_sz)= g_byte[NFC_SPARE_OFST_6_LBA+1];
               }
               g_lut_block_index[sub_lut_id]= nf_refine_index(i_block, 1, NFC_BLK_ID_SUBLUT);
               trace_hex16(i_block); trace(" ("); trace_u32(i_block); trace("): SUB-LUT (id ");trace_hex(sub_lut_id);trace(" ofst "); trace_hex(g_lut_block_index[sub_lut_id]); trace(")\n\r");
               continue ;
            }
            else
            {  // LUT found on bad NF
               g_fatal=TRUE;
               break;
            }
         }

         else if ( g_byte[NFC_SPARE_OFST_1_BLK_ID]==NFC_BLK_ID_FBB )
         {
            n_invalid_blocks +=1;
            if ( i_dev==S_MNGT_DEV )
            {
               if ( TRUE==g_is_found_fbb )
               {
                  g_fatal=TRUE; // already found
                  break;
               }
               g_fbb_block_addr  = i_block;
               g_fbb_block_index = nf_refine_index(i_block, 1, NFC_BLK_ID_FBB);
               nfc_read_spare_byte( g_byte, 16, nf_block_2_page(i_block) + (U32)g_fbb_block_index );     // Reload
               if( NFC_OFST_6_FBB_VALID!=g_byte[NFC_SPARE_OFST_6_LBA] )
               {
                  g_fatal=TRUE; // FBB not valid. Force rebuild
                  break;
               }

               MSB(g_n_free_blocks)   = g_byte[NFC_SPARE_OFST_2_BYTE_2];
               LSB(g_n_free_blocks)   = g_byte[NFC_SPARE_OFST_3_BYTE_3];
               s_nfd_rev              = g_byte[NFC_SPARE_OFST_4_BYTE_4];
               MSB(g_n_export_blocks) = g_byte[NFC_SPARE_OFST_EXPORT];
               LSB(g_n_export_blocks) = g_byte[NFC_SPARE_OFST_EXPORT+1];
               trace("      g_n_free_blocks="); trace_hex16(g_n_free_blocks); trace_nl();
               trace("      g_n_export_blocks="); trace_hex16(g_n_export_blocks); trace_nl();
               g_is_found_fbb=TRUE;
               trace_hex16(i_block); trace(" ("); trace_u32(i_block); trace("): FBB (ofst "); trace_hex( g_fbb_block_index ); trace(")\n\r");
               continue ;
            }
            else
            {
               g_fatal=TRUE;
               break;
            }
         }
      } // for ( i_block

      // A fatal error on one device is enough to cleanup all the devices !
      //
      s_n_invalid_blocks[   i_dev]= n_invalid_blocks;
      s_n_quarantine_blocks[i_dev]= n_quarantine_blocks;

      if ( TRUE==g_fatal ) { break; }
   } // for ( i_dev

   return (g_fatal==TRUE) ? FAIL: PASS;
} // nf_scan


//! @brief Test if the memory needs to be rebuilt.
//!
//! @param recovery_params  internal structure used to hold the last pointer position
//!
//! @return a status:
//!           TRUE if the memory has to be rebuilt;
//!           FALSE if the memory holds all the management blocks
//!
static Bool is_nf_invalid ( void )
{
   if( // If we do not find everything
      ( FALSE==g_is_found_lut )
   || ( FALSE==g_is_found_fbb )
   ) {
      g_fatal=TRUE;
   }

   // Test LUT coherency
   //
   if(( TRUE       ==g_is_found_lut   )
   && ( g_n_sub_lut!=g_n_real_sub_lut ))
   {
      g_fatal=TRUE;
   }

//#error se proteger si le nombre de devices changent alors que lut, recovery et free blocs sont déjà créés sur le bloc MNGT.
   if ( (U16)-1==g_n_export_blocks     ) { g_fatal=TRUE; }
   if (       0==g_n_export_blocks     ) { g_fatal=TRUE; }
   if ( (U16)-1==g_last_sub_lut_log_sz ) { g_fatal=TRUE; }

   // Test Nand Flash driver release.
   //
   if ( s_nfd_rev!=NFC_OFST_4_FBB_DRIVER_RELEASE )
   {
      g_fatal=TRUE;
   }

   return g_fatal;
}



//! @brief Rebuild the memory and create LUT, Recovery and Free-blocks blocks.
//!
//! TBD
//!
//! It uses s_n_invalid_blocks (number of invalid blocks) and
//! g_curr_block_addr (current block address of each device).
//!
//! @return a status:
//!           PASS if there are no error;
//!           FAIL a programmation error occured: the block is marked as bad. The
//!           function must be recall.
//!
static Status_bool nf_rebuild ( void )
{
   Status_bool status_bool=PASS;
   Bool        b_duplicate;
   U8   i_sub_lut;
   U8   i_dev  =0;
   _MEM_TYPE_SLOW_ U16  i_block=0;
   _MEM_TYPE_SLOW_ U16  u16_tmp;
   _MEM_TYPE_SLOW_ U16  sub_lut_log_sz;
   _MEM_TYPE_SLOW_ U16  log_block_addr;
   _MEM_TYPE_SLOW_ U16  log_block_addr_min;
   _MEM_TYPE_SLOW_ U16  log_block_addr_max;

   // Refine the computation
   //
   s_n_invalid_blocks[S_MNGT_DEV] +=
      1                                        // Need a block for the Free-blocks block
   +  (G_N_BLOCKS*NF_N_DEVICES)/NF_SUBLUT_SIZE // and one for each sub-LUT
   ;

   // Take the max number of invalid blocks of each devices
   //
   u16_tmp=s_n_invalid_blocks[0] ;
   for ( i_dev=1 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      u16_tmp=Max( u16_tmp, s_n_invalid_blocks[i_dev] );
   }

   // Take the max number of quarantine blocks of each devices
   //
   i_sub_lut=s_n_quarantine_blocks[0] ;
   for ( i_dev=1 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      i_sub_lut=Max( i_sub_lut, s_n_quarantine_blocks[i_dev] );
   }

   sub_lut_log_sz = (U16)NF_N_DEVICES*(G_N_BLOCKS -g_nf_first_block -u16_tmp);

   // Finally compute the number of exportable physical blocks and free blocks
   //
   Assert( u16_tmp<(G_N_BLOCKS -g_nf_first_block) );
   g_n_export_blocks= (U16)( ((U32)( (U32)sub_lut_log_sz ) * 1000) / 1024);
   g_n_export_blocks= Align_down( g_n_export_blocks, NF_N_DEVICES);

   g_n_free_blocks  = (U16)sub_lut_log_sz - g_n_export_blocks;
   g_n_free_blocks -= (U16)NF_N_DEVICES*i_sub_lut;

   if( g_n_free_blocks<=NF_LOW_N_FREE_THRESHOLD )
   {
      while(1); // TBD
   }

   Assert( g_n_free_blocks>0 );
   Assert( g_n_free_blocks<(1L<<NF_SHIFT_PAGE_BYTE) ); // limit the free blocks in order to fit in 1 page

   // Compute the number of needed sub-LUT
   // Affect to each management block a free block address
   //
   Nfc_action(NFC_ACT_DEV_SELECT, S_MNGT_DEV);
   g_fbb_block_addr = nf_fetch_free_block(S_MNGT_DEV);
   nfc_erase_block( nf_block_2_page( g_fbb_block_addr ), TRUE );
   g_n_sub_lut= 0;
   u16_tmp    = g_n_export_blocks;
//#error il faut positionner les index(LUT, FBB, RCV...)
   while(1)
   {
      Assert( g_n_sub_lut<N_SUBLUT );
      g_lut_block_addr [g_n_sub_lut]=nf_fetch_free_block(S_MNGT_DEV);
      g_lut_block_index[g_n_sub_lut]=0;
      nfc_erase_block( nf_block_2_page( g_lut_block_addr [g_n_sub_lut] ), TRUE );
      g_n_sub_lut++;
      if( u16_tmp>NF_SUBLUT_SIZE )  u16_tmp-=NF_SUBLUT_SIZE;
      else                          break;
   }
   g_last_sub_lut_log_sz=u16_tmp/NF_N_DEVICES;

   // Build the sub-LUTs
   //
   for ( i_sub_lut=0 ; i_sub_lut<g_n_sub_lut ;  )
   {
      U8  n_sublut_in_buf = g_n_sub_lut - i_sub_lut; // Count remaining sublut to build

      log_block_addr_max =
      log_block_addr_min = (U16)i_sub_lut<<(NF_SHIFT_SUBLUT_PHYS-NF_SHIFT_N_DEVICES); // first included

      if( n_sublut_in_buf>(NF_PAGE_BUFFER_SIZE/(2*NF_SUBLUT_SIZE)) )
      {
         n_sublut_in_buf = NF_PAGE_BUFFER_SIZE/(2*NF_SUBLUT_SIZE);
         log_block_addr_max += ((U16)n_sublut_in_buf)*g_sub_lut_log_sz; // last not included
      }
      else
      {
         log_block_addr_max += ((U16)n_sublut_in_buf-1)*g_sub_lut_log_sz +g_last_sub_lut_log_sz; // last not included
      }

      nf_init_buffer();

      // Report affected logical blocks
      //
      u16_tmp=g_n_export_blocks/NF_N_DEVICES; // Number of logical blocks used for the mass storage

      b_duplicate=FALSE;

      for ( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
      {
         Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

         g_block_to_kill[i_dev]=0xFFFF;

         for ( i_block=g_nf_first_block ; i_block<G_N_BLOCKS ; i_block++ )
         {
            nfc_read_spare_byte( g_byte, 8, nf_block_2_page(i_block) );
            if(( 0xFF           !=g_byte[G_OFST_BLK_STATUS          ] ) // The block is bad
            || ( NFC_BLK_ID_DATA!=g_byte[NFC_SPARE_OFST_1_BLK_ID    ] ) // or is not a data block
            || (  ( 0xFF        ==g_byte[NFC_SPARE_OFST_6_LBA       ] ) // or is not affected
               && ( 0xFF        ==g_byte[NFC_SPARE_OFST_6_LBA+1     ] )
               )
            ) {
               continue;
            }

            MSB(log_block_addr) = g_byte[NFC_SPARE_OFST_6_LBA  ];
            LSB(log_block_addr) = g_byte[NFC_SPARE_OFST_6_LBA+1];

            if( log_block_addr>=u16_tmp )
            {  // The LBA seems bad: it does not fit in any LUT. This happens when unplugging the player.
               // Block is erased.
               // Anyway, stay in the loop to track similar problems.
               nfc_erase_block( nf_block_2_page(i_block), TRUE );
               status_bool=FAIL;
            }

            if(( log_block_addr>=log_block_addr_min )
            && ( log_block_addr< log_block_addr_max ))
            {
               U16 ofst=2*((U16)i_dev + (log_block_addr%((U16)NF_PAGE_BUFFER_SIZE/2/NF_N_DEVICES))*NF_N_DEVICES) ;
               if(
                  ( 0xFF==g_page_buffer[ ofst    ] )
               && ( 0xFF==g_page_buffer[ ofst +1 ] )
               )
               {  // no redundant phys blocks
                  Assert(      ( ofst +1 ) < NF_PAGE_BUFFER_SIZE );
                  g_page_buffer[ ofst    ] = MSB(i_block);
                  g_page_buffer[ ofst +1 ] = LSB(i_block);
               }
               else
               {  // A duplicated logical block is detected. This happens when unplugging the player.
                  // Anyway, stay in the loop to track any other redundant blocks, for that sub-LUT.
                  _MEM_TYPE_SLOW_ U16 tmp_addr;
                  MSB(tmp_addr)=g_page_buffer[ ofst    ];
                  LSB(tmp_addr)=g_page_buffer[ ofst +1 ];
                  //trace("Dupl "); trace_hex32(tmp_addr); trace("-"); trace_hex32(i_block);; trace("\n\r");

                  if(0xFFFF!=g_block_to_kill[i_dev])
                  {  // !!! There are more than 1 duplicated block on the device. This should never happen...
                     nfc_erase_block( nf_block_2_page(g_block_to_kill[i_dev]), TRUE );
                     return FAIL;
                  }

                  b_duplicate=TRUE;
                  g_log_block_id=log_block_addr;

                  nfc_open_page_read(
                     nf_block_2_page(i_block)
                  ,  NF_SPARE_POS+NFC_SPARE_OFST_3_BYTE_3
                  );
                  if( NFC_OFST_3_DATA_DST!=Nfc_rd_data_fetch_next() )
                  {
                     trace("1. Src block="); trace_hex16(i_block); trace_nl();
                     trace("1. Dst block="); trace_hex16(tmp_addr); trace_nl();
                     //nfc_print_block(i_block, 0);
                     //nfc_print_block(tmp_addr, 0);
                     //while(1);
                     g_block_to_kill[i_dev]=i_block;                        // source block
                     g_phys_page_addr[i_dev] = nf_block_2_page( tmp_addr ); // recipient block
                  }
                  else
                  {
                     trace("2. Src block="); trace_hex16(tmp_addr); trace_nl();
                     trace("2. Dst block="); trace_hex16(i_block); trace_nl();
                     //nfc_print_block(tmp_addr, 0);
                     //nfc_print_block(i_block, 0);
                     //while(1);
                     g_block_to_kill[i_dev]= tmp_addr ;                     // source block
                     g_page_buffer[ ofst    ]=MSB(i_block);
                     g_page_buffer[ ofst +1 ]=LSB(i_block);
                     g_phys_page_addr[i_dev] = nf_block_2_page( i_block );  // recipient block
                  }
               }
            }
         } // for ( i_block ../..
      } // for ( i_dev ../..

      if( b_duplicate )
      {
         U8 i_page;
         U8 i_sect;

         trace("recovery\n\r");
         // Test that recovery can be done
         for ( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
         {
            if( 0xFFFF==g_block_to_kill[i_dev] )
            {  // !Ooops... we can not recover from that case since there are duplication
               // only on on device
               for ( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
               {
                  if( 0xFFFF!=g_block_to_kill[i_dev] )
                  {
                     nfc_erase_block( nf_block_2_page(g_block_to_kill[i_dev]), TRUE );
                  }
               }
               return FAIL;
            }
         }

         // Initialize variable for nf_copy_tail
         g_curr_dev_id=0;
         g_last_log_sector= ((U32)g_log_block_id) << S_SHIFT_LOG_BLOCK_SECTOR;

         // Look for last written sector
         for( i_page=0 ; i_page<SIZE_BLOCK_PAGE ; i_page++ )
         {
            Nfc_action(NFC_ACT_DEV_SELECT, g_curr_dev_id);  // open the current device
            for( i_sect=0 ; i_sect<SIZE_PAGE_SECTOR ; i_sect++ )
            {
               nfc_open_page_read(
                  g_phys_page_addr[g_curr_dev_id]
               ,  NF_SPARE_POS + (((U16)i_sect)*16) + NFC_SPARE_OFST_6_LBA
               );
               if(( 0xFF==Nfc_rd_data_fetch_next() )
               && ( 0xFF==Nfc_rd_data_fetch_next() ))
                  goto recovery_exit;
               else
               {
                  g_last_log_sector++;
                  trace("g_last_log_sector="); trace_hex32(g_last_log_sector); trace_nl();
               }
            }
            g_phys_page_addr[g_curr_dev_id]++;                                                   // update the current physical page of the current device
            g_curr_dev_id++;                                                                     // update the current device
            if( g_curr_dev_id==NF_N_DEVICES ) { g_curr_dev_id=0; }
            trace("g_curr_dev_id="); trace_hex(g_curr_dev_id); trace_nl();
            trace("g_phys_page_addr="); trace_hex32(g_phys_page_addr[g_curr_dev_id]); trace_nl();
         }
recovery_exit:
         trace("recovery stop on g_last_log_sector="); trace_hex32(g_last_log_sector); trace_nl();
         trace("g_curr_dev_id="); trace_hex(g_curr_dev_id); trace_nl();
         trace("g_phys_page_addr="); trace_hex32(g_phys_page_addr[g_curr_dev_id]); trace_nl();
         //while(1);
         nf_copy_tail();
         return FAIL;
      }

      // At least one redundant have been found: the LUT must be rebuilt since the fetch of free block
      // may not have seen that affected block (redundant) are in fact free.
      if( PASS!=status_bool ) { return FAIL; }

      // Affect a free physical block to the logical block
      //
      for( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
      {
         Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

         for(u16_tmp=0
         ;   u16_tmp<(log_block_addr_max-log_block_addr_min)
         ;   u16_tmp++ )
         {
            U16 ofst=2*((U16)i_dev + u16_tmp*NF_N_DEVICES);
            if(( 0xFF==g_page_buffer[ofst  ] )
            && ( 0xFF==g_page_buffer[ofst+1] ))
            {
               i_block=nf_fetch_free_block(i_dev);
               Assert(       ofst+1<NF_PAGE_BUFFER_SIZE);
               g_page_buffer[ofst  ] = MSB(i_block);
               g_page_buffer[ofst+1] = LSB(i_block);
            }
         }
      } // for ( i_dev ../..

      // Each sub-LUT will fit in a physical page and will be of the same size
      // except the last one which contains less
      //
      for( ; n_sublut_in_buf!=0 ; n_sublut_in_buf--, i_sub_lut++ )
      {
         sub_lut_log_sz= ( i_sub_lut==(g_n_sub_lut-1) ) ? g_last_sub_lut_log_sz : g_sub_lut_log_sz ;

         // Write the sub-LUT in the page
         //
         status_bool = nf_write_lut(i_sub_lut%(NF_PAGE_BUFFER_SIZE/(2*NF_SUBLUT_SIZE)), i_sub_lut, sub_lut_log_sz);
         if ( PASS!=status_bool )
         {
            nfc_mark_bad_block( nf_block_2_page( g_lut_block_addr[i_sub_lut] ) );
            return FAIL;
         }
      }
   }

//#error: si recovery, il faut effacer la lut en question. Il faut donc la reconstruire.
//        Pour cela, il faut trouver des blocs libres.
// 1ere methode: effacer aussi le free-blocks block et le reconstruire, ainsi que la sub-LUT
// 2eme methode: marquer les free block pour les reconnaitre et reconstruire la sub lut

   // Build the free-blocks block
   // First, fill the internal buffer with the free blocks
   //
   for ( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

      for ( u16_tmp=0 ; u16_tmp<(g_n_free_blocks/NF_N_DEVICES) ; u16_tmp++ )
      {
         // This define is better than using a variable that holds the expression...
         #define OFST   (2*(i_dev + u16_tmp*NF_N_DEVICES))
         i_block=nf_fetch_free_block(i_dev);
         nfc_erase_block( nf_block_2_page(i_block), TRUE );
         Assert(       OFST   <NF_PAGE_BUFFER_SIZE);
         Assert(       OFST +1<NF_PAGE_BUFFER_SIZE);
         Assert( i_block>=g_nf_first_block );
         Assert( i_block< G_N_BLOCKS       );
         g_page_buffer[OFST   ] = MSB(i_block);
         g_page_buffer[OFST +1] = LSB(i_block);
         #undef OFST
      }
   }

   // Then write the buffer in the free-blocks block
   // Note that the list of free-blocks holds on one page only; the
   // algo is thus made for both 512B and 2kB pages.
   //
   g_fbb_block_index=0;
   status_bool = nf_write_fbb();
   if ( PASS!=status_bool )
   {
      nfc_mark_bad_block( nf_block_2_page( g_fbb_block_addr ) );
      return FAIL;
   }

//#error Effacer les free blocks !!!!!!
//#error il faut determiner s_lut_index[all] pour les sub-lut existantes
//#error si il existe un bloc de recovery, alors la lut associée n'est plus valide
//#error rendre parametrable la taille du buffer (actuellement 2k). Si <512 et no partial prog: fatal error

   //nf_init_buffer(); // Cleanup the buffer
   return PASS;
}



//! Returns the first free block seen, scanning downstream.
//!
//! It uses g_curr_block_addr (current block address of each device).
//!
//! @param i_dev              device number on which we look for a free block
//!
//! @return the physical block number
//!
static U16 nf_fetch_free_block(U8 i_dev)
{

   U16 block_addr= g_curr_block_addr[i_dev];

   while ( block_addr>=g_nf_first_block )
   {
      nfc_read_spare_byte( g_byte, 8, nf_block_2_page( block_addr ) );
      if(( 0xFF           ==g_byte[G_OFST_BLK_STATUS          ] ) // the block is valid
      && ( NFC_BLK_ID_DATA==g_byte[NFC_SPARE_OFST_1_BLK_ID    ] ) // the block is a data block
      && ( 0xFF           ==g_byte[NFC_SPARE_OFST_6_LBA       ] ) // and is not affected
      && ( 0xFF           ==g_byte[NFC_SPARE_OFST_6_LBA+1     ] ))
      {
         // Since we rebuild the flash, we should not see any of these blocks
         //
         Assert( NFC_BLK_ID_SUBLUT!=g_byte[NFC_SPARE_OFST_1_BLK_ID] );
         Assert( NFC_BLK_ID_FBB   !=g_byte[NFC_SPARE_OFST_1_BLK_ID] );

         // Find a free and valid block addr. Store the current position
         //
         g_curr_block_addr[i_dev] = block_addr-1;
         return block_addr;
      }
      block_addr-=1 ;
   }
   // This situation is dramatic: it should never happen !
   // Force Rebuild on next startup
   nfc_erase_block( nf_block_2_page( g_fbb_block_addr ), TRUE );
   while(1);
   Assert( FALSE ) ; // Not enough free blocks: fatal error!
}



//! Refines the position of the 'block' index according to a particular
//! pattern. This allow to find exactely where are stored the last
//! sub-LUT, Free_blocks or Recovery entry.
//! The index is roughly initialized at the beginning of the block that holds
//! the 'pattern' at the location 1 of the spare zone. The function parses the
//! block until the pattern is no more found.
//!
//! @param block_addr  physical block address
//! @param inc         increment
//! @param pattern     pattern which is scanned
//!
//! @return the offset (in page) of the last valid entry in the block
//!
static U8 nf_refine_index(
   U16 block_addr
,  U8  inc
,  U8  pattern)
{
   _MEM_TYPE_SLOW_ U8 u8_tmp;
   _MEM_TYPE_SLOW_ U8 val=0;
   do
   {
      val+= inc; // Assume that the pattern has already be seen previously
      if( val>=SIZE_BLOCK_PAGE )
         { break; }
      nfc_open_page_read(
         nf_block_2_page(block_addr) + val
      ,  NF_SPARE_POS+NFC_SPARE_OFST_1_BLK_ID
      );
      u8_tmp = Nfc_rd_data();
   } while( pattern==u8_tmp );
   val-= inc; // come back to last valid entry
   Assert( val<(1<<G_SHIFT_BLOCK_PAGE) ); // The offset shall not be outside the block
   return val;
}



#if (ERASING_ALL==ENABLE)
static void ut_nfc_erase_all( void )
{
   _MEM_TYPE_SLOW_ U8   i_dev  =0;
   _MEM_TYPE_SLOW_ U16  i_block=0;
   _MEM_TYPE_SLOW_ U32  page_addr;

   for( i_dev=0 ; i_dev<NF_N_DEVICES ; i_dev++ )
   {
      // Select the devices
      //
      Nfc_action(NFC_ACT_DEV_SELECT, i_dev);

      for( i_block=g_nf_first_block ; i_block<G_N_BLOCKS ; i_block++ )
      {
         page_addr= (U32)i_block<<NF_SHIFT_BLOCK_PAGE;
         nfc_erase_block( page_addr, FALSE ) ;
      }
   }
}
#endif

