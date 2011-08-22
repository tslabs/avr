/*This file has been prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the low level macros and definition for the
//!
//!  Nand-Flash Controller.
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


#ifndef _NFC_DRV_H_
#define _NFC_DRV_H_

#undef _GLOBEXT_
#if (defined _nfc_drv_c_)
#  define _GLOBEXT_
#else
#  define _GLOBEXT_ extern
#endif

//====FOR COMPATIBILITY WITH AVR (no DFC)==
#define  CLK_DFC_NFC_40MHz             0
#define  CLK_DFC_NFC_30MHz             0
#define  CLK_DFC_NFC_24MHz             0
//=========================================

//! @defgroup nfc_actions NFC Actions
//! List of the actions that can be performed by the NFC.
//! @{
#define NFC_ACT_NOP        0 //!< No Operation.
#define NFC_ACT_DEV_SELECT 1 //!< Device selection. The number is given by the extension.
#define NFC_ACT_READ       2 //!< Read flow through DFC.
#define NFC_ACT_WRITE      3 //!< Write flow through DFC.
#define NFC_ACT_ASSERT_CE  4 //!< Assert the CE pad of the last selected device.
#   define NFC_EXT_CELOW     1
#   define NFC_EXT_NOP       0
#define NFC_ACT_STOP       5 //!< Stop the NFC, or the DFC flow.
#define NFC_ACT_ADC_EXT    6 //!< Column address extension. The extension gives A9:8.
//! @}

//************************** XMCR Addresses *********************
//************ A13  A12  A11  A10  A9   A8   
//************ PC5  PC4  PC3  PC2  PC1  PC0  
//************ CS3  CS2  CS1  CS0  ALE  CLE  
//--------------------------------------------------------------------
//************  1    1    x    x    1    0   | ADD_LATCH
//************  1    1    x    x    0    1   | CMD_LATCH
//************  1    1    x    x    0    0   | CMD_DATA
// We need to set bits A13:12 to get an address > 0x2100 (=external memory)
// (these lines are not physically addressed no NF3 and NF4
// The CS0 and CS1 are drived by manual pin IO control
// The ALE and CLE may be drived by manual pin IO control (see define NF_CLE_ALE_MANUAL in conf_nf.h)
#define  NF_ADD_LATCH_ENABLE_ADD       0x3A00    // Address Latch Enable Address
#define  NF_CMD_LATCH_ENABLE_ADD       0x3900    // Command Latch Enable Address
#define  NF_ADDRESS_CMD_DATA           0x3800    // Command / Data Address register


/************************** Low Level routines *****************************/
#  define  Nf_rd_byte()                  (nf_data)
#  define  Nf_wr_byte(b)                 (nf_data = b)
#if (NF_CLE_ALE_MANUAL == ENABLED)
#  define  Nf_send_command(command)      {Nandflash_CLE_select(); nf_send_cmd = command; Nandflash_CLE_unselect(); }
#  define  Nf_send_address(address)      {Nandflash_ALE_select(); nf_send_add = address; Nandflash_ALE_unselect(); }
#else
#  define  Nf_send_command(command)      (nf_send_cmd = command)
#  define  Nf_send_address(address)      (nf_send_add = address)
#endif

// Compatibility with other firmware prototypes
#define  Mcu_set_sfr_page_nfc()
#define  Nfc_set_cmd(command)          Nf_send_command(command)
#define  Nfc_rd_status()               Nf_rd_byte()      // once the STATUS cmd has been sent, all read commands will read the status
#define  Nfc_rd_data()                 Nf_rd_byte()
#define  Nfc_rd_data_fetch_next()      Nf_rd_byte()
#define  Nfc_wr_data(dat)              Nf_wr_byte(dat)
#define  Nfc_set_adc(adr)              Nf_send_address(adr)
#define  Nfc_set_adr(adr)              Nf_send_address(adr)
#define  Nfc_unprotect_all_flash()          // assumes that memories are not write protected
//#define  Nfc_wait_cache_busy()         nfc_wait_busy()    // declared below
#define  Nfc_action(a,b)               { if( a == NFC_ACT_DEV_SELECT ) nfc_select_dev(b); }
#define  nfc_init(a,b)                 nf_XMCR_enable()
#define  Nfc_get_ecc()                 (0xFF)
#define  Nfc_init_ecc()



//! Opens a page for read. The macro will adapt the commands according to the
//! type of flash memory. The busy is polled at the end of the function.
//!
//! @param page_addr          absolute page address of the block
//! @param byte_addr          relative byte address inside the page.
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The NF device should have been selected before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
#define  Nfc_open_page_read( page_addr, byte_addr)               \
   if ( Is_nf_512() ) /* 512B pages */                     \
   {                                                       \
      if ( (byte_addr)<256 )                               \
      {                                                    \
         Nfc_set_cmd(NF_READ_A_AREA_CMD);                  \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);    \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
         nfc_wait_busy();                                  \
         if(G_CE_TOGGLE)                                   \
         {  Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_NOP);}  \
      }                                                    \
      else if ( (byte_addr)<512 )                          \
      {                                                    \
         Nfc_set_cmd(NF_READ_B_AREA_CMD);                  \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_action(NFC_ACT_ADC_EXT, 1);                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);    \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
         nfc_wait_busy();                                  \
         if(G_CE_TOGGLE)                                   \
         {  Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_NOP);}  \
      }                                                    \
      else                                                 \
      {                                                    \
         Nfc_set_cmd(NF_READ_C_AREA_CMD);                  \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_action(NFC_ACT_ADC_EXT, 2);                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_CELOW);    \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
         nfc_wait_busy();                                  \
         if(G_CE_TOGGLE)                                   \
         {  Nfc_action( NFC_ACT_ASSERT_CE, NFC_EXT_NOP);}  \
      }                                                    \
   }                                                       \
   if ( Is_nf_2k() ) /* 2KB pages */                       \
   {                                                       \
      Nfc_set_cmd(NF_READ_CMD);                            \
      Nfc_set_adc( LSB( byte_addr) );                      \
      Nfc_set_adc( MSB( byte_addr) );                      \
      Nfc_set_adr( LSB0(page_addr) );                      \
      Nfc_set_adr( LSB1(page_addr) );                      \
      if ( 3==G_N_ROW_CYCLES )                             \
      {                                                    \
         Nfc_set_adr( MSB1(page_addr) );                   \
      }                                                    \
      Nfc_set_cmd(NF_READ_CMD2);                           \
      nfc_wait_busy();                                     \
   }                                                       \
   Nfc_set_cmd(NF_READ_CMD); /* Back to the read mode */




//! Opens a page for write. The macro will adapt the commands according to the
//! type of flash memory.
//!
//! @param page_addr          absolute page address of the block
//! @param byte_addr          relative byte address inside the page.
//!
//! @pre <code>nf_init()</code> should have been called before.
//! The NF device should have been selected before with <code>Nfc_action(NFC_ACT_DEV_SELECT, id)</code>.
//!
#define Nfc_open_page_write( page_addr, byte_addr)         \
   Nfc_wait_cache_busy();                                  \
   Nfc_unprotect_all_flash(); /* WP may be actif due to block protection */ \
                                                           \
   if ( Is_nf_512() ) /* 512B pages */                     \
   {                                                       \
      if ( (byte_addr)<256 )                               \
      {                                                    \
         Nfc_set_cmd(NF_READ_A_AREA_CMD);                  \
         Nfc_set_cmd(NF_SEQUENTIAL_DATA_INPUT_CMD);        \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
      }                                                    \
      else if ( (byte_addr)<512 )                          \
      {                                                    \
         Nfc_set_cmd(NF_READ_B_AREA_CMD);                  \
         Nfc_set_cmd(NF_SEQUENTIAL_DATA_INPUT_CMD);        \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_action(NFC_ACT_ADC_EXT, 1);                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
      }                                                    \
      else                                                 \
      {                                                    \
         Nfc_set_cmd(NF_READ_C_AREA_CMD);                  \
         Nfc_set_cmd(NF_SEQUENTIAL_DATA_INPUT_CMD);        \
         Nfc_set_adc( LSB( byte_addr) );                   \
         Nfc_action(NFC_ACT_ADC_EXT, 2);                   \
         Nfc_set_adr( LSB0(page_addr) );                   \
         Nfc_set_adr( LSB1(page_addr) );                   \
         if ( 3==G_N_ROW_CYCLES )                          \
         {                                                 \
            Nfc_set_adr( MSB1(page_addr) );                \
         }                                                 \
      }                                                    \
   }                                                       \
   if ( Is_nf_2k() ) /* 2KB pages */                       \
   {                                                       \
      Nfc_set_cmd(NF_SEQUENTIAL_DATA_INPUT_CMD);           \
      Nfc_set_adc( LSB( byte_addr) );                      \
      Nfc_set_adc( MSB( byte_addr) );                      \
      Nfc_set_adr( LSB0(page_addr) );                      \
      Nfc_set_adr( LSB1(page_addr) );                      \
      if ( 3==G_N_ROW_CYCLES )                             \
      {                                                    \
         Nfc_set_adr( MSB1(page_addr) );                   \
      }                                                    \
   }



//_____ D E C L A R A T I O N ______________________________________________
//                                                      

#define  NF_MAX_DEVICES                2
//! Define here the number of NF connected to the microcontroller
//! For AVR8 the number must be 1

#define  NF_MAX_RB_TIMEOUT             0xFF


//! @defgroup nfc_flashes_commands Flash-memories commands
//! Set of commands of the FLASH memories (NF, SMC, ...)
//! @{
#define  NF_READ_A_AREA_CMD            0x00 //!< Read Command of zone A (512B)
#define  NF_READ_B_AREA_CMD            0x01 //!< Read Command of zone B (512B)
#define  NF_READ_C_AREA_CMD            0x50 //!< Read Command of zone C (spare) (512B)

#define  NF_READ_CMD                   0x00 //!< Read Command (2KB)
#define  NF_READ_CMD2                  0x30 //!< Confirm read Command (2KB)
#define  NF_RANDOM_READ_CMD_C1         0x05 //!< Random read Command (2KB)
#define  NF_RANDOM_READ_CMD_C2         0xE0 //!< Confirm random read Command (2KB)

#define  NF_READ_ID_CMD                0x90 //!< Read ID Command
#define  NF_READ_ID2_CMD               0x91 //!< Read ID 2 Command

#define  NF_RESET_CMD                  0xff //!< Reset command

#define  NF_SEQUENTIAL_DATA_INPUT_CMD  0x80 //!< Sequential data input command
#define  NF_PAGE_PROGRAM_CMD           0x10 //!< Page Program command

#define  NF_RANDOM_DATA_INPUT_CMD      0x85 //!< Random data input command (2KB)
#define  NF_COPY_BACK_CMD              0x35 //!< Copy-back command (2KB)
#define  NF_CACHE_PROGRAM_CMD          0x15 //!< Cache program (fast) command (2KB)

#define  NF_BLOCK_ERASE_CMD            0x60 //!< Erase command
#define  NF_BLOCK_ERASE_CONFIRM_CMD    0xD0 //!< Confirm erase command

#define  NF_READ_STATUS_CMD            0x70 //!< Read Status command
//! @}



//! @defgroup nfc_mask_status Flash-memories read-status bits
//! Masks of the status returned by the NF_READ_STATUS_CMD command.
//! @{
#define  NF_MASK_STATUS_FAIL               (1<<0) //!< Fail
#define  NF_MASK_STATUS_READY              (1<<6) //!< Ready
#define  NF_MASK_STATUS_T_RDY_2KB          (1<<5) //!< True Ready
#define  NF_MASK_STATUS_T_RDY_512B         (1<<6) //!< True Ready
//! @}



//! Macro that tests the cache busy.
//!
#define Nfc_wait_cache_busy()                                        \
   {                                                                 \
      register int Reg;                                              \
      Nfc_set_cmd(NF_READ_STATUS_CMD);                               \
      Reg = Nfc_rd_status();                                         \
      while( (Nfc_rd_status() & NF_MASK_STATUS_READY)==0 /*busy*/ ); \
      while( (Nfc_rd_status() & NF_MASK_STATUS_READY)==0 /*busy*/ ); \
   }

#define SIZE_PAGE_BYTE                  ((U16)1<<G_SHIFT_PAGE_BYTE)
#define SIZE_SECTOR_BYTE                ((U16)1<<S_SHIFT_SECTOR_BYTE)
#define SIZE_BLOCK_PAGE                 ((U8)1<<G_SHIFT_BLOCK_PAGE)
#define SIZE_PAGE_SECTOR                ((U8)1<<(G_SHIFT_PAGE_BYTE - S_SHIFT_SECTOR_BYTE))

// Spare zone offsets definition
//
//#define NF_SPARE_SIZE                16  // in bytes

#define NFC_SPARE_OFST_1_BLK_ID         1
#define NFC_BLK_ID_SYSTEM       0x39    // System block (Font, screen, firmware, ...)
#define NFC_BLK_ID_SUBLUT       0xE8    // sub-LUT block
#define NFC_BLK_ID_RCV          0x72    // Recovery block
#define NFC_BLK_ID_FBB          0xB4    // Free-blocks block
#define NFC_BLK_ID_DATA         0xFF    // Data block (mass storage). Also the default value after an erase
#define NFC_BLK_ID_QUARANTINE   0x8D    // Block which encountered an ECC error not recoverable. Not yet bad...

#define NFC_SPARE_OFST_2_BYTE_2         2
#define NFC_SPARE_OFST_3_BYTE_3         3 // NFC_BLK_ID_DATA: used as 'data valid'
#define NFC_OFST_3_DATA_SRC                0x00    // Identify a source block (recovery)
#define NFC_OFST_3_DATA_DST                0xFF    // Identify a recipient block (recovery)

#define NFC_SPARE_OFST_4_BYTE_4         4
#define NFC_OFST_4_FBB_DRIVER_RELEASE      0x01    // Current NF release. When a new firmware implies that LUT and FBB must be rebuild, just increment this number.
#define NFC_SPARE_DATA_VALID    0xFF
#define NFC_SPARE_DATA_INVALID     0
#define NFC_SPARE_OFST_EXPORT          11

#define NFC_SPARE_OFST_6_LBA            6 // and 7
#define NFC_OFST_6_FBB_VALID    0xFF
#define NFC_OFST_6_FBB_INVALID     0

#define NFC_SPARE_OFST_ECC2             8 // and 9, 10
#define NFC_SPARE_OFST_ECC1            13 // and 14, 15

#if( NF_BAD_CONFIG==(FALSE) )
#  if (NF_GENERIC_DRIVER==TRUE) || (NF_AUTO_DETECT_2KB==TRUE) || (NF_AUTO_DETECT_512B==TRUE)
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_n_zones          ; // number of zones (=1024 blocks) per device
      _GLOBEXT_ _MEM_TYPE_SLOW_ U16  g_n_blocks         ; // number of blocks per device
      _GLOBEXT_ _MEM_TYPE_FAST_ U8   g_n_row_cycles     ; // number of row cycles to access a page of the NF memory
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_copy_back_cont   ; // 0 = copy back not supported, N = number of    CONTINUE subdivision contraint on copyback
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_copy_back_discont; // 0 = copy back not supported, N = number of DISCONTINUE subdivision contraint on copyback
      _GLOBEXT_                 Bool g_cache_program    ; // 1 = cache program supported, 0 = not supported
      _GLOBEXT_                 Bool g_ce_toggle           ; // tell if CE must be low during read cycle or not                                                            
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_clock_dfc_nfc    ; // Clock of dfc and nfc
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_dev_maker        ; // Device maker
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_dev_id           ; // Device ID
#     define G_N_ZONES                 g_n_zones
#     define G_N_BLOCKS                g_n_blocks
#     define G_N_ROW_CYCLES            g_n_row_cycles
#     define G_COPY_BACK_CONT          g_copy_back_cont
#     define G_COPY_BACK_DISCONT       g_copy_back_discont
#     define G_CACHE_PROG              g_cache_program
#     define G_CE_TOGGLE               g_ce_toggle
#     define G_CLK_DFC_NFC             g_clock_dfc_nfc
#     define G_DEV_MAKER               g_dev_maker
#     define G_DEV_ID                  g_dev_id
#  else
#     define G_N_ZONES                 (NF_N_ZONES)
#     define G_N_BLOCKS                (NF_N_BLOCKS)
#     define G_N_ROW_CYCLES            (NF_N_ROW_CYCLES)
#     define G_COPY_BACK_CONT          (NF_COPYBACK_CONT)
#     define G_COPY_BACK_DISCONT       (NF_COPYBACK_DISCONT)
#     define G_CACHE_PROG              (NF_CACHE_PROGRAM)
#     define G_CE_TOGGLE               (NF_CE_TOGGLE)
#     define G_CLK_DFC_NFC             (CLK_DFC_NFC)
#     define G_DEV_MAKER               (NF_DEV_MAKER)
#     define G_DEV_ID                  (NF_DEV_ID)
#  endif

#  if (NF_GENERIC_DRIVER==TRUE)
      _GLOBEXT_ _MEM_TYPE_FAST_ U8   g_shift_page_byte  ; // (1<<n) size of page,   unit in bytes
      _GLOBEXT_ _MEM_TYPE_FAST_ U8   g_shift_block_page ; // (1<<n) size of physical block,  unit in pages
      _GLOBEXT_ _MEM_TYPE_SLOW_ U8   g_ofst_blk_status  ; // Offset of Block Status information in spare zone
#     define Is_nf_2k()              (11==g_shift_page_byte )
#     define Is_not_nf_2k()          (11!=g_shift_page_byte )
#     define Is_nf_512()             ( 9==g_shift_page_byte )
#     define Is_not_nf_512()         ( 9!=g_shift_page_byte )
#     define G_SHIFT_BLOCK_PAGE        g_shift_block_page
#     define G_SHIFT_PAGE_BYTE         g_shift_page_byte
#     define G_OFST_BLK_STATUS         g_ofst_blk_status
#     define NF_SPARE_POS              ( Is_nf_2k() ? 2048 : 512 )
#     define NF_N_GOOD_STATIC_BLOCK    ( Is_nf_2k() ?   32 : 256 )
#else
#     if (NF_SHIFT_PAGE_BYTE==11) // 2KB pages
#        define Is_nf_2k()      ( TRUE  )
#        define Is_not_nf_2k()  ( FALSE )
#        define Is_nf_512()     ( FALSE )
#        define Is_not_nf_512() ( TRUE  )
#        define NF_SPARE_POS         2048  // Spare zone starts at this byte offset in the page
#        define NF_N_GOOD_STATIC_BLOCK 32  // Number of the *valid* blocks in the static area
#        define G_OFST_BLK_STATUS       0  // Offset of the Bad Block information in spare zone
#     endif
#     if (NF_SHIFT_PAGE_BYTE==9) // 512B pages
#        define Is_nf_2k()        ( FALSE )
#        define Is_not_nf_2k()    ( TRUE  )
#        define Is_nf_512()       ( TRUE  )
#        define Is_not_nf_512()   ( FALSE )
#        define NF_SPARE_POS           512  // Spare zone starts at this byte offset in the page
#        define NF_N_GOOD_STATIC_BLOCK 256  // Number of the *valid* blocks in the static area
#        define G_OFST_BLK_STATUS        5  // Offset of the Bad Block information in spare zone
#     endif
#     define G_SHIFT_BLOCK_PAGE        (NF_SHIFT_BLOCK_PAGE)
#     define G_SHIFT_PAGE_BYTE         (NF_SHIFT_PAGE_BYTE)
#  endif

#endif // NF_BAD_CONFIG

//_____ F U N C T I O N S __________________________________________________

// Values used by "nfc_detect" fonction
#define     NO_NF_CONNECTED   0xFF
#define     NF_UNKNOW         0xFE

// Functions prototypes

void        nf_XMCR_enable(       void );
void        nf_XMCR_disable(      void );
void        nfc_select_dev( U8 dev );
void        nfc_init(             U8 nb_dev, U16 last_protected_block );
Status_bool nfc_check_status(     void );
U8          nfc_check_type(       U8 nb_dev );
void        nfc_reset_nands(      U8 nb_dev );
void        nfc_open_page_read(   U32 page_addr, U16 byte_addr);
void        nfc_open_page_write(  U32 page_addr, U16 byte_addr);
void        nfc_mark_bad_block(   U32 page_addr );
void        nfc_erase_block(      U32 page_addr, U8 force_erase );
void        nfc_copy_back_init(   U32 page_addr );
void        nfc_copy_back_conf(   U32 page_addr );
U32         nfc_read_id(          U8 read_id_cmd, U8 nf_num );
U8          nfc_detect(           void );
void        nfc_wait_busy(        void );

void        nfc_read_spare_byte(
   U8 _MEM_TYPE_SLOW_ * p_byte
,  U8  n_byte
,  U32 page_addr
);

void        nfc_print_block(      U16 block_addr, U8 dev_id);

#endif  // _NFC_DRV_H_
