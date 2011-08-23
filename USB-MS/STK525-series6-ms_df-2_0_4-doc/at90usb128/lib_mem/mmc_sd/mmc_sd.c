/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the low-level SD/MMC routines
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


/*_____ I N C L U D E S ____________________________________________________*/
#include "config.h"                         /* system configuration */
#include "conf_sdmmc.h"
#include "lib_mcu/usb/usb_drv.h"            /* usb driver definition */
#include "lib_mcu/spi/spi_drv.h"            /* spi driver definition */
#include "mmc_sd.h"                         /* MMC SD definition */


/*_____ D E F I N I T I O N ________________________________________________*/

static   U32   gl_ptr_mem;                   // memory data pointer

bit  mmc_sd_init_done = FALSE;
U8   r1;
U16  r2;

          U8   csd[16];                     // stores the Card Specific Data
volatile  U32  capacity;                    // stores the capacity in bytes
volatile  U32  mmc_sd_last_block_address;   // stores the address of the last block (sector)
          U16  erase_group_size;            // stores the number of blocks concerned by an erase command
          U8   card_type;                   // stores SD_CARD or MMC_CARD type card


#if       (MMC_SD_RAM == ENABLED)
          U8   data_mem[513]; // data buffer
#endif
#if      (MMC_SD_READ_CID == ENABLED)
          U8   cid[16];
#endif


/*_____ D E C L A R A T I O N ______________________________________________*/

//!
//! @brief This function initializes the SPI communication link between the MMC_SD and the MMC_SD driver.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return none
//!
//!/
void mmc_sd_spi_init(void)
{
   Spi_select_master();
   Spi_set_mode(SPI_MODE_0);
   Spi_init_bus();
   Spi_set_rate(SPI_RATE_0);  // SCK freq == fosc/2.
   Spi_disable_ss();
   Spi_enable();
}


//!
//! @brief This function initializes the Dataflash controller & the SPI bus(over which the MMC_SD is controlled).
//!
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return bit
//!   The memory is ready     -> OK (always)
//!/
bit mmc_sd_init (void)
{
  U16 retry;

  // INIT HARDWARE
  mmc_sd_spi_init();

  // RESET THE MEMORY CARD
  mmc_sd_init_done = FALSE;
  card_type = MMC_CARD;
  retry = 0;
  do
  {
    // reset card and go to SPI mode
    r1 = mmc_sd_send_command(MMC_GO_IDLE_STATE, 0);
    Spi_write_data(0xFF);            // write dummy byte
    // do retry counter
    retry++;
    if(retry > 100)
      return KO;
  }
  while(r1 != 0x01);   // check memory enters idle_state

  // IDENTIFICATION OF THE CARD TYPE (SD or MMC)
  // Both cards will accept CMD55 command but only the SD card will respond to ACMD41
  r1 = mmc_sd_send_command(SD_APP_CMD55,0);
  Spi_write_data(0xFF);  // write dummy byte

  r1 = mmc_sd_send_command(SD_SEND_OP_COND_ACMD, 0);
  Spi_write_data(0xFF);  // write dummy byte

  if ((r1&0xFE) == 0)   // ignore "in_idle_state" flag bit
  {
    card_type = SD_CARD;    // card has accepted the command, this is a SD card
  }
  else
  {
    card_type = MMC_CARD;   // card has not responded, this is a MMC card
    // reset card again
    retry = 0;
    do
    {
      // reset card again
      r1 = mmc_sd_send_command(MMC_GO_IDLE_STATE, 0);
      Spi_write_data(0xFF);            // write dummy byte
      // do retry counter
      retry++;
      if(retry > 100)
        return KO;
    }
    while(r1 != 0x01);   // check memory enters idle_state
  }

  // CONTINUE INTERNAL INITIALIZATION OF THE CARD
  // Continue sending CMD1 while memory card is in idle state
  retry = 0;
  do
  {
     // initializing card for operation
     r1 = mmc_sd_send_command(MMC_SEND_OP_COND, 0);
     Spi_write_data(0xFF);            // write dummy byte
     // do retry counter
     retry++;
     if(retry == 50000)    // measured approx. 500 on several cards
        return KO;
  }
  while (r1);

  // DISABLE CRC TO SIMPLIFY AND SPEED UP COMMUNICATIONS
  r1 = mmc_sd_send_command(MMC_CRC_ON_OFF, 0);  // disable CRC (should be already initialized on SPI init)
  Spi_write_data(0xFF);            // write dummy byte

  // SET BLOCK LENGTH TO 512 BYTES
  r1 = mmc_sd_send_command(MMC_SET_BLOCKLEN, 512);
  Spi_write_data(0xFF);            // write dummy byte
  if (r1 != 0x00)
    return KO;    // card unsupported if block length of 512b is not accepted

  // GET CARD SPECIFIC DATA
  if (KO ==  mmc_sd_get_csd(csd))
    return KO;

  // GET CARD CAPACITY and NUMBER OF SECTORS
  mmc_sd_get_capacity();

  // GET CARD IDENTIFICATION DATA IF REQUIRED
#if (MMC_SD_READ_CID == ENABLED)
  if (KO ==  mmc_sd_get_cid(cid))
    return KO;
#endif

  mmc_sd_init_done = TRUE;

  return(OK);
}


//!
//! @brief This function sends a command WITH NO DATA STATE to the SD/MMC and waits for R1 response
//!        This function also selects and unselects the memory => should be used only for single command transmission
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  command   command to send (see mmc_sd.h for command list)
//!         arg       argument of the command
//!
//! @return U8
//!         R1 response (R1 == 0xFF if time out error)
//!/

U8 mmc_sd_send_command(U8 command, U32 arg)
{
  Mmc_sd_select();                    // select MMC_SD
  r1 = mmc_sd_command(command, arg);
  Mmc_sd_unselect();                  // unselect MMC_SD
  return r1;
}

//!
//! @brief This function sends a command WITH DATA STATE to the SD/MMC and waits for R1 response
//!        The memory /CS signal is not affected so this function can be used to send a commande during a large transmission
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  command   command to send (see mmc_sd.h for command list)
//!         arg       argument of the command
//!
//! @return U8
//!         R1 response (R1 == 0xFF time out error)
//!/
U8 mmc_sd_command(U8 command, U32 arg)
{
U8 retry;

  Spi_write_data(0xFF);            // write dummy byte
  Spi_write_data(command | 0x40);  // send command
  Spi_write_data(arg>>24);         // send parameter
  Spi_write_data(arg>>16);
  Spi_write_data(arg>>8 );
  Spi_write_data(arg    );
  Spi_write_data(0x95);            // correct CRC for first command in SPI (CMD0)
                                  // after, the CRC is ignored
  // end command
  // wait for response
  // if more than 8 retries, card has timed-out and return the received 0xFF
  retry = 0;
  r1    = 0xFF;
  while((r1 = mmc_sd_send_and_read(0xFF)) == 0xFF)
  {
    retry++;
    if(retry > 10) break;
  }
  return r1;
}



//!
//! @brief This function sends a byte over SPI and returns the byte read from the slave.
//!
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  data_to_send   byte to send over SPI
//!
//! @return U8
//!   Byte read from the slave
//!/
U8 mmc_sd_send_and_read(U8 data_to_send)
{
   Spi_write_data(data_to_send);
   return (Spi_read_data());
}



//!
//! @brief This function reads the CSD (Card Specific Data) of the memory card
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  buffer to fill
//!
//! @return bit
//!         OK / KO
//!/
bit mmc_sd_get_csd(U8 *buffer)
{
U8 retry;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();                  // select MMC_SD
  // issue command
  r1 = mmc_sd_command(MMC_SEND_CSD, 0);
  // check for valid response
  if(r1 != 0x00)
  {
    Mmc_sd_unselect();     // unselect MMC_SD
    mmc_sd_init_done = FALSE;
    return KO;
  }
  // wait for block start
  retry = 0;
  while((r1 = mmc_sd_send_and_read(0xFF)) != MMC_STARTBLOCK_READ)
  {
    if (retry > 8)
    {
      Mmc_sd_unselect();     // unselect MMC_SD
      return KO;
    }
    retry++;
  }
  for (retry = 0; retry <16; retry++)
  {
    Spi_write_data(0xFF);
    buffer[retry] = Spi_read_data();
  }
  Spi_write_data(0xFF);   // load CRC (not used)
  Spi_write_data(0xFF);
  Spi_write_data(0xFF);   // give clock again to end transaction
  Mmc_sd_unselect();     // unselect MMC_SD
  return OK;
}



//!
//! @brief This function reads the CID (Card Identification Data) of the memory card
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  buffer to fill
//!
//! @return bit
//!         OK / KO
//!/
bit mmc_sd_get_cid(U8 *buffer)
{
U8 retry;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();                  // select MMC_SD
  // issue command
  r1 = mmc_sd_command(MMC_SEND_CID, 0);
  // check for valid response
  if(r1 != 0x00)
  {
    Mmc_sd_unselect();     // unselect MMC_SD
    mmc_sd_init_done = FALSE;
    return KO;
  }
  // wait for data block start
  retry = 0;
  while((r2 = mmc_sd_send_and_read(0xFF)) != MMC_STARTBLOCK_READ)
  {
    if (retry > 8)
    {
      Mmc_sd_unselect();     // unselect MMC_SD
      return KO;
    }
    retry++;
  }
  // store valid data block
  for (retry = 0; retry <16; retry++)
  {
    Spi_write_data(0xFF);
    buffer[retry] = Spi_read_data();
  }
  Spi_write_data(0xFF);   // load CRC (not used)
  Spi_write_data(0xFF);
  Spi_write_data(0xFF);   // give clock again to end transaction
  Mmc_sd_unselect();     // unselect MMC_SD
  return OK;
}



//!
//! @brief This function extracts structure information from CSD array
//! and compute the number of blocks of the memory card (stored in global U32 mmc_sd_last_block_address),
//! its capacity in bytes (stored in global U32 capacity)
//! and the block group size for an erase operation
//! Here is defined the position of required fields in CSD array :
//! READ_BL_LEN :
//!         [83:80] == data[5] && 0x0f
//! C_SIZE :
//!         [73:72] == data[6] && 0x03
//!         [71:64] == data[7]
//!         [63:62] == data[8] && 0xc0
//! C_SIZE_MULT :
//!         [49:48] == data[9] && 0x03
//!         [47]    == data[10] && 0x80
//! ERASE_GRP_SIZE (MMC card only) :
//!         [46:42] == data[10] && 0x7c
//! ERASE_GRP_MULT (MMC card only) :
//!         [41:40] == data[10] && 0x03
//!         [39:37] == data[11] && 0xe0
//! SECTOR_SIZE (SD card only) :
//!         [45:40] == data[10] && 0x3F
//!         [39]    == data[11] && 0x80
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  None
//!
//! @return bit
//!         OK
//!/
void mmc_sd_get_capacity(void)
{
  U16 c_size;
  U8  c_size_mult;
  U8  read_bl_len;
  U8  erase_grp_size;
  U8  erase_grp_mult;

  // extract variables from CSD array
  c_size      = ((csd[6] & 0x03) << 10) + (csd[7] << 2) + ((csd[8] & 0xC0) >> 6);
  c_size_mult = ((csd[9] & 0x03) << 1) + ((csd[10] & 0x80) >> 7);
  read_bl_len = csd[5] & 0x0F;
  if (card_type == MMC_CARD)
  {
    erase_grp_size = ((csd[10] & 0x7C) >> 2);
    erase_grp_mult = ((csd[10] & 0x03) << 3) | ((csd[11] & 0xE0) >> 5);
  }
  else
  {
    erase_grp_size = ((csd[10] & 0x3F) << 1) + ((csd[11] & 0x80) >> 7);
    erase_grp_mult = 0;
  }

  // compute last block addr
  mmc_sd_last_block_address = ((U32)(c_size + 1) * (U32)((1 << (c_size_mult + 2)))) - 1;
  if (read_bl_len > 9)  // 9 means 2^9 = 512b
    mmc_sd_last_block_address <<= (read_bl_len - 9);

  // compute card capacity in bytes
  capacity = (1 << read_bl_len) * (mmc_sd_last_block_address + 1);

  // compute block group size for erase operation
  erase_group_size = (erase_grp_size + 1) * (erase_grp_mult + 1);
}



//!
//! @brief    This function reads the STATUS regsiter of the memory card
//!           After a read the error flags are automatically cleared
//!
//! @warning  Code:?? bytes (function code length)
//!
//! @param    None
//!
//! @return bit
//!           The open succeeded      -> OK
//!/
bit     mmc_sd_get_status(void)
{
  U8 retry, spireg;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();       // select MMC_SD

  // send command
  Spi_write_data(MMC_SEND_STATUS | 0x40);  // send command
  Spi_write_data(0);                       // send parameter
  Spi_write_data(0);
  Spi_write_data(0);
  Spi_write_data(0);
  Spi_write_data(0x95);            // correct CRC for first command in SPI (CMD0)
                                  // after, the CRC is ignored
  // end command
  // wait for response
  // if more than 8 retries, card has timed-out and return the received 0xFF
  retry = 0;
  r2 = 0xFFFF;
  spireg = 0xFF;
  while((spireg = mmc_sd_send_and_read(0xFF)) == 0xFF)
  {
    retry++;
    if(retry > 10)
    {
      Mmc_sd_unselect();
      return KO;
    }
  }
  r2 = ((U16)(spireg) << 8) + mmc_sd_send_and_read(0xFF);    // first byte is MSb

  Spi_write_data(0xFF);   // give clock again to end transaction
  Mmc_sd_unselect();     // unselect MMC_SD

  return OK;
}


//!
//! @brief This function waits until the MMC/SD is not busy.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  None
//!
//! @return bit
//!          OK when card is not busy
//!/
bit mmc_sd_wait_not_busy(void)
{
  U16 retry;

  // Select the MMC_SD memory gl_ptr_mem points to
  Mmc_sd_select();
  retry = 0;
  while((r1 = mmc_sd_send_and_read(0xFF)) != 0xFF)
  {
    retry++;
    if (retry == 50000)
    {
      Mmc_sd_unselect();
      return KO;
    }
  }
  Mmc_sd_unselect();
  return OK;
}



//!
//! @brief This function check the presence of a memory card
//!     - if the card was already initialized (removal test), the host send a CRC_OFF command (CMD59) and check the answer
//!     - if the card was not already initialized (insertion test), the host send a CMD0 reset command and check the answer
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return bit
//!   The memory is present (OK)
//!   The memory does not respond (disconnected) (KO)
//!/
bit mmc_sd_check_presence(void)
{
  U16 retry;

  retry = 0;
  if (mmc_sd_init_done == FALSE)
  {
    // If memory is not initialized, try to initialize it (CMD0)
    // If no valid response, there is no card
    while ((r1 = mmc_sd_send_command(MMC_GO_IDLE_STATE, 0)) != 0x01)
    {
      Spi_write_data(0xFF);            // write dummy byte
      retry++;
      if (retry > 10)
        return KO;
    }
    return OK;
  }
  else
  {
    // If memory already initialized, send a CRC command (CMD59) (supported only if card is initialized)
    /*
    retry = 0;
    while (retry != 50000)
    {
      r1 = mmc_sd_send_command(MMC_CMD2,0);   // unsupported command in SPI mode
      Spi_write_data(0xFF);            // write dummy byte
      if (r1 != 0)    // memory must answer with an error code (with bit7=0)
      {
        if (r1 < 0x80)
          return OK;
        else
        {
          mmc_sd_init_done = FALSE;
          return KO;
        }
      }
      retry++;
    }
    */
    if ((r1 = mmc_sd_send_command(MMC_CRC_ON_OFF,0)) == 0x00)
      return OK;
    mmc_sd_init_done = FALSE;
    return KO;
  }

  return KO;
}


//!
//! @brief This function performs a memory check on the MMC_SD.
//!
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param none
//!
//! @return bit
//!   The memory is ready     -> OK
//!   The memory check failed -> KO
//!/
bit mmc_sd_mem_check(void)
{
  if (mmc_sd_check_presence() == OK)
  {
    if (mmc_sd_init_done == FALSE)
    {
      mmc_sd_init();
    }
    if (mmc_sd_init_done == TRUE)
      return OK;
    else
      return KO;
  }
  return KO;
}



//!
//! @brief This function checks if the card is password-locked
//!        Old versions of MMC card don't support this feature !
//!        For a MMC, "lock protection" is featured from v2.1 release !
//!          => see CSD[0]<5:2> bits to know the version : 0x0=1.x, 0x1=1.4, 0x2=2.x, 0x3=3.x, 0x4=4.0
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  None
//!
//! @return bit
//!   Password protected         -> OK
//!   NOT password protected     -> KO (or card not initialized)
//!/
bit is_mmc_sd_write_pwd_locked(void)
{
  if (card_type == MMC_CARD)
  {
    if (((csd[0] >> 2) & 0x0F) < 2) // lock feature is not present on the card since the MMC is v1.x released !
      return KO;
  }
  if (KO == mmc_sd_get_status())    // get STATUS response
    return KO;
  if ((r2&0x0001) != 0)             // check "card is locked" flag in R2 response
    return OK;

  return KO;
}


//!
//! @brief This function manages locking operations for the SD/MMC card (password protection)
//!         - Once the card is locked, the only commands allowed are UNLOCK and FORCED_ERASE
//!         - Once the card is unlocked, the commands allowed are all the others
//!         - Before setting a new password (SET_PWD), the current one must be cleared (RESET_PWD)
//!         - If card contains a password (PWDSLEN != 0), the card will automatically be locked at start-up
//!
//!    /!\  Take care that old versions of MMC cards don't support this feature !
//!         For a MMC, "lock protection" is featured only from v2.1 release !
//!           => see CSD[0]<5:2> bits to know the version : 0x0=1.x, 0x1=1.4, 0x2=2.x, 0x3=3.x, 0x4=4.0
//!         Moreover the OP_FORCED_ERASE command can also have no effect on some cards !
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param U8 operation
//!           OP_LOCK           -> to lock the card (the current pasword must be specified)
//!           OP_UNLOCK         -> to unlock the card (the current password must be specified)
//!           OP_RESET_PWD      -> to clear the current password (the current password must be specified)
//!           OP_SET_PWD        -> to set a new password to the card (the old password must have been cleared first)
//!           OP_FORCED_ERASE   -> to erase completely the card and the password (no password needed)
//!        U8 pwd_lg
//!           Password length
//!        U8 * pwd
//!           Pointer on the password (char array) to send
//!
//! @return bit
//!   Operation succeeded       -> OK
//!   Operation failed          -> KO
//!/
bit mmc_sd_lock_operation(U8 operation, U8 pwd_lg, U8 * pwd)
{
  bit status = OK;
  U8 retry;

  // check parameters validity
  if ((operation != OP_FORCED_ERASE) && (pwd_lg == 0))  // password length must be > 0
    return KO;

  // wait card not busy
  if (mmc_sd_wait_not_busy() == KO)
    return KO;

  // set block length
  if (operation == OP_FORCED_ERASE)
    r1 = mmc_sd_send_command(MMC_SET_BLOCKLEN, 1);   // CMD
  else
    r1 = mmc_sd_send_command(MMC_SET_BLOCKLEN, pwd_lg+2);   // CMD + PWDSLEN + PWD
  Spi_write_data(0xFF);            // write dummy byte
  Spi_write_data(0xFF);            // write dummy byte
  Spi_write_data(0xFF);            // write dummy byte
  if (r1 != 0x00)
    return KO;

  // send the lock command to the card
  Mmc_sd_select();                // select MMC_SD

  // issue command
  r1 = mmc_sd_command(MMC_LOCK_UNLOCK, 0);

  // check for valid response
  if(r1 != 0x00)
  {
    status = KO;
  }
  // send dummy
  Spi_write_data(0xFF);   // give clock again to end transaction

  // send data start token
  Spi_write_data(MMC_STARTBLOCK_WRITE);
  // write data
  Spi_write_data(operation);
  if (operation != OP_FORCED_ERASE)
  {
    Spi_write_data(pwd_lg);
    for(retry=0 ; retry<pwd_lg ; retry++)
    {
      Spi_write_data(*(pwd+retry));
    }
  }
  Spi_write_data(0xFF);    // send CRC (field required but value ignored)
  Spi_write_data(0xFF);

  // check data response token
  retry = 0;
  r1 = mmc_sd_send_and_read(0xFF);
  if ((r1 & MMC_DR_MASK) != MMC_DR_ACCEPT)
    status = KO;

  Spi_write_data(0xFF);    // dummy byte
  Mmc_sd_unselect();

  // wait card not busy
  if (operation == OP_FORCED_ERASE)
    retry = 100;
  else
    retry = 10;
  while (mmc_sd_wait_not_busy() == KO)
  {
    retry--;
    if (retry == 0)
    {
      status = KO;
      break;
    }
  }

  // get and check status of the operation
  if (KO == mmc_sd_get_status())    // get STATUS response
    status = KO;
  if ((r2&0x0002) != 0)   // check "lock/unlock cmd failed" flag in R2 response
    status = KO;

  // set original block length
  r1 = mmc_sd_send_command(MMC_SET_BLOCKLEN, 512);
  Spi_write_data(0xFF);            // write dummy byte
  if (r1 != 0x00)
    status = KO;

  return status;
}



//!
//! @brief This function opens a MMC_SD memory in read mode at a given sector address (not byte address)
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  pos   Sector address
//!
//! @return bit
//!   The open succeeded      -> OK
//!/
bit mmc_sd_read_open (U32 pos)
{
  // Set the global memory ptr at a Byte address.
  gl_ptr_mem = pos << 9;        // gl_ptr_mem = pos * 512

  // wait for MMC not busy
  return mmc_sd_wait_not_busy();
}


//!
//! @brief This function unselects the current MMC_SD memory.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  None
//!
//! @return None
//!/
void mmc_sd_read_close (void)
{

}


//!
//! @brief This function opens a MMC_SD memory in write mode at a given sector
//! address.
//!
//! NOTE: If page buffer > 512 bytes, page content is first loaded in buffer to
//! be partially updated by write_byte or write64 functions.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  pos   Sector address
//!
//! @return bit
//!   The open succeeded      -> OK
//!/
bit mmc_sd_write_open (U32 pos)
{
  // Set the global memory ptr at a Byte address.
  gl_ptr_mem = pos << 9;                    // gl_ptr_mem = pos * 512

  // wait for MMC not busy
  return mmc_sd_wait_not_busy();
}


//!
//! @brief This function fills the end of the logical sector (512B) and launch
//! page programming.
//!
//! @warning Code:?? bytes (function code length)
//!
//! @param  gl_ptr_mem   memory ptr on MMC_SD [global][IN/OUT]
//!
//! @return None
//!/
void mmc_sd_write_close (void)
{

}



#if (MMC_SD_USB == ENABLE)

//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! DataFlash memory to USB controller
//!
//!         DATA FLOW is: MMC_SD => USB
//!
//!
//! NOTE:
//!   - First call must be preceded by a call to the mmc_sd_read_open() function,
//!   - The USB EPIN must have been previously selected,
//!   - USB ping-pong buffers are free,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to read [IN]
//! @param gl_ptr_mem   memory ptr on MMC_SD [global][IN/OUT]
//!
//! @return bit
//!   The read succeeded   -> OK
//!/
bit mmc_sd_read_sector(U16 nb_sector)
{
  Byte i;
  U16  read_time_out;

  do
  {
    Mmc_sd_select();                  // select MMC_SD
    // issue command
    r1 = mmc_sd_command(MMC_READ_SINGLE_BLOCK, gl_ptr_mem);
    // check for valid response
    if(r1 != 0x00)
    {
       Mmc_sd_unselect();                  // unselect MMC_SD
       return KO;
    }

    // wait for token (may be a datablock start token OR a data error token !)
    read_time_out = 30000;
    while((r1 = mmc_sd_send_and_read(0xFF)) == 0xFF)
    {
       read_time_out--;
       if (read_time_out == 0)   // TIME-OUT
       {
         Mmc_sd_unselect();               // unselect MMC_SD
         return KO;
       }
    }

    // check token
    if (r1 != MMC_STARTBLOCK_READ)
    {
      Spi_write_data(0xFF);
      Mmc_sd_unselect();                  // unselect MMC_SD
      return KO;
    }

    //#
    //# Read 8x64b = 512b, put them in the USB FIFO IN.
    //#
    for (i = 8; i != 0; i--)
    {
       Disable_interrupt();    // Global disable.

       // Principle: send any Byte to get a Byte.
       // Spi_write_data(0): send any Byte + 1st step to clear the SPIF bit.
       // Spi_read_data(): get the Byte + final step to clear the SPIF bit.
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Spi_write_data(0xFF); Usb_write_byte(Spi_read_data());
       Enable_interrupt();     // Global re-enable.

       //#
       //# Send the USB FIFO IN content to the USB Host.
       //#
       Usb_send_in();       // Send the FIFO IN content to the USB Host.
       // Wait until the tx is done so that we may write to the FIFO IN again.
       while(Is_usb_write_enabled()==FALSE)
       {
          if(!Is_usb_endpoint_enabled())
             return KO; // USB Reset
       }
    } // for (i = 8; i != 0; i--)

    gl_ptr_mem += 512;     // Update the memory pointer.
    nb_sector--;           // 1 more sector read
    // read 16-bit CRC
    Spi_write_data(0xFF);
    Spi_write_data(0xFF);
    // dummy bytes
    Spi_write_data(0xFF);
    Spi_write_data(0xFF);
    // release chip select
    Mmc_sd_unselect();                  // unselect MMC_SD
  }
  while (nb_sector != 0);

  return OK;   // Read done.
}


//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! USB controller to DataFlash memory
//!
//!         DATA FLOW is: USB => MMC_SD
//!
//!
//! NOTE:
//!   - First call must be preceded by a call to the mmc_sd_write_open() function,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - The USB EPOUT must have been previously selected,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to write [IN]
//! @param gl_ptr_mem   memory ptr on MMC_SD [global][IN/OUT]
//!
//! @return bit
//!   The write succeeded  -> OK
//!/
bit mmc_sd_write_sector (U16 nb_sector)
{
U8 i;

  do
  {
    // wait card not busy
    i=0;
    while (KO == mmc_sd_wait_not_busy())
    {
      i++;
      if (i == 10)
        return KO;
    }

    Mmc_sd_select();                  // select MMC_SD
    // issue command
    r1 = mmc_sd_command(MMC_WRITE_BLOCK, gl_ptr_mem);
    // check for valid response
    if(r1 != 0x00)
    {
      Mmc_sd_unselect();                  // unselect MMC_SD
      return KO;
    }
    // send dummy
    Spi_write_data(0xFF);   // give clock again to end transaction
    // send data start token
    Spi_write_data(MMC_STARTBLOCK_WRITE);
      // write data
  //#
  //# Write 8x64b = 512b from the USB FIFO OUT.
  //#
    for (i = 8; i != 0; i--)
    {
      // Wait end of rx in USB EPOUT.
      while(!Is_usb_read_enabled())
      {
         if(!Is_usb_endpoint_enabled())
           return KO; // USB Reset
      }

      Disable_interrupt();    // Global disable.

      // SPI write principle: send a Byte then clear the SPIF flag.
      // Spi_write_data(Usb_read_byte()): (.) Final step to clear the SPIF bit,
      //                                  (.) send a Byte read from USB,
      //                                  (.) 1st step to clear the SPIF bit.
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_write_data(Usb_read_byte());
      Spi_ack_write();        // Final step to clear the SPIF bit.

      Usb_ack_receive_out();  // USB EPOUT read acknowledgement.

      Enable_interrupt();     // Global re-enable.
    } // for (i = 8; i != 0; i--)

    Spi_write_data(0xFF);    // send dummy CRC
    Spi_write_data(0xFF);

    // read data response token
    Spi_write_data(0xFF);
    r1 = Spi_read_data();
    if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
    {
      Mmc_sd_unselect();                  // unselect MMC_SD
      return r1;
    }

    // send dummy byte
    Spi_write_data(0xFF);

    // release chip select
    Mmc_sd_unselect();                  // unselect MMC_SD
    gl_ptr_mem += 512;        // Update the memory pointer.
    nb_sector--;              // 1 more sector written
  }
  while (nb_sector != 0);

  // wait card not busy after last programming operation
  i=0;
  while (KO == mmc_sd_wait_not_busy())
  {
    i++;
    if (i == 10)
      return KO;
  }

  return OK;                  // Write done
}
#endif      // (MMC_SD_USB == ENABLE)

/*
//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! USB HOST controller to DataFlash memory
//!
//!         DATA FLOW is: USB => MMC_SD
//!
//!
//! NOTE:
//!   - This function should ne used only when using the USB controller in HOST mode
//!   - First call must be preceded by a call to the mmc_sd_write_open() function,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - The USB PIPE OUT must have been previously selected,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to write [IN]
//! @param gl_ptr_mem   memory ptr on MMC_SD [global][IN/OUT]
//!
//! @return bit
//!   The write succeeded  -> OK
//!/
bit mmc_sd_host_write_sector (U16 nb_sector)
{

  return OK;                  // Write done
}
*/

/*
//!
//! @brief This function is optimized and writes nb-sector * 512 Bytes from
//! DataFlash memory to USB HOST controller
//!
//!         DATA FLOW is: MMC_SD => USB
//!
//!
//! NOTE:
//!   - This function should ne used only when using the USB controller in HOST mode
//!   - First call must be preceded by a call to the mmc_sd_read_open() function,
//!   - The USB PIPE IN must have been previously selected,
//!   - USB ping-pong buffers are free,
//!   - As 512 is always a sub-multiple of page size, there is no need to check
//!     page end for each Bytes,
//!   - Interrupts are disabled during transfer to avoid timer interrupt,
//!   - nb_sector always >= 1, cannot be zero.
//!
//! @warning code:?? bytes (function code length)
//!
//! @param nb_sector    number of contiguous sectors to read [IN]
//! @param gl_ptr_mem   memory ptr on MMC_SD [global][IN/OUT]
//!
//! @return bit
//!   The read succeeded   -> OK
//!/
bit mmc_sd_host_read_sector (U16 nb_sector)
{

  return OK;   // Read done.
}
*/



//! @brief  This function erase a group of sectors
//!        NOTE : Erasing operation concerns only groups of sectors and not one sector only
//!               The global variable "erase_group_size" (extracted from CSD) contains the sector group size boundary
//!               User specifies the addresses of the first group and the last group to erase (several contiguous groups can be selected for erase)
//!               An misaligned address will not generate an error since the memory card ignore the LSbs of the address
//!               Some examples (with "erase_group_size" = 0x20 = group boundary) :
//!                 - adr_start=0x100 and adr_end=0x100, all the sectors from 0x100 up to 0x11F will be erased
//!                 - adr_start=0x90 and adr_end=0x100, all the sectors from 0x80 up to 0x11F will be erased (0x90 interpreted as 0x80)
//!                 - adr_start=0x80 and adr_end=0x146, all the sectors from 0x80 up to 0x15F will be erased
//!               This function just initiates a transmission, user may get status register to check that operation has succeeded
//!               After an erase, a MMC card contains bits at 0, and SD can contains bits 0 or 1 (according to field DATA_STAT_AFTER_ERASE in the CSD)
//!
//! @param U32 adr_start         address of 1st group   (sector address, not byte address)
//!        U32 adr_end           address of last group  (sector address, not byte address)
//!
//! @return bit
//!   The erase operation succeeded (has been started)  -> OK
//!   The erase operation failed (not started)  -> KO
//!/
bit mmc_sd_erase_sector_group(U32 adr_start, U32 adr_end)
{
  U8 cmd;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();          // select MMC_SD

  // send address of 1st group
  if (card_type == MMC_CARD)
  { cmd = MMC_TAG_ERASE_GROUP_START; }
  else
  { cmd = SD_TAG_WR_ERASE_GROUP_START; }
  if ((r1 = mmc_sd_command(cmd,(adr_start << 9))) != 0)
  {
    Mmc_sd_unselect();
    return KO;
  }
  Spi_write_data(0xFF);

  // send address of last group
  if (card_type == MMC_CARD)
  { cmd = MMC_TAG_ERASE_GROUP_END; }
  else
  { cmd = SD_TAG_WR_ERASE_GROUP_END; }
  if ((r1 = mmc_sd_command(cmd,(adr_end << 9))) != 0)
  {
    Mmc_sd_unselect();
    return KO;
  }
  Spi_write_data(0xFF);

  // send erase command
  if ((r1 = mmc_sd_command(MMC_ERASE,0)) != 0)
  {
    Mmc_sd_unselect();
    return KO;
  }
  Spi_write_data(0xFF);

  Mmc_sd_unselect();

  return OK;
}



#if (MMC_SD_RAM == ENABLED)

//! @brief This function read one MMC sector and load it into a ram buffer
//!
//!         DATA FLOW is: MMC/SD => RAM
//!
//!
//! NOTE:
//!   - First call (if sequential read) must be preceded by a call to the mmc_sd_read_open() function
//!
//! @param *ram         pointer to ram buffer
//!
//! @return bit
//!   The read succeeded   -> OK
//!   The read failed (bad address, etc.)  -> KO
//!/
bit mmc_sd_read_sector_to_ram(U8 *ram)
{
  U16  i;
  U16  read_time_out;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();          // select MMC_SD
  // issue command
  r1 = mmc_sd_command(MMC_READ_SINGLE_BLOCK, gl_ptr_mem);

  // check for valid response
  if (r1 != 0x00)
  {
    Mmc_sd_unselect();     // unselect MMC_SD
    return KO;
  }

  // wait for token (may be a datablock start token OR a data error token !)
  read_time_out = 30000;
  while((r1 = mmc_sd_send_and_read(0xFF)) == 0xFF)
  {
     read_time_out--;
     if (read_time_out == 0)   // TIME-OUT
     {
       Mmc_sd_unselect();               // unselect MMC_SD
       return KO;
     }
  }

  // check token
  if (r1 != MMC_STARTBLOCK_READ)
  {
    Spi_write_data(0xFF);
    Mmc_sd_unselect();                  // unselect MMC_SD
    return KO;
  }

  // store datablock
  Disable_interrupt();    // Global disable.
  for(i=0;i<MMC_SECTOR_SIZE;i++)
  {
    Spi_write_data(0xFF);
    *ram=Spi_read_data();
    ram++;
  }
  Enable_interrupt();     // Global re-enable.
  gl_ptr_mem += 512;     // Update the memory pointer.

  // load 16-bit CRC (ignored)
  Spi_write_data(0xFF);
  Spi_write_data(0xFF);

  // continue delivering some clock cycles
  Spi_write_data(0xFF);
  Spi_write_data(0xFF);

  // release chip select
  Mmc_sd_unselect();                  // unselect MMC_SD

  return OK;   // Read done.
}



//! @brief This function write one MMC sector from a ram buffer
//!
//!         DATA FLOW is: RAM => MMC/SD
//!
//!
//! NOTE (please read) :
//!   - First call (if sequential write) must be preceded by a call to the mmc_sd_write_open() function
//!   - An address error will not detected here, but with the call of mmc_sd_get_status() function
//!   - The program exits the functions with the memory card busy !
//!
//! @param *ram         pointer to ram buffer
//!
//! @return bit
//!   The write succeeded   -> OK
//!   The write failed      -> KO
//!
bit mmc_sd_write_sector_from_ram(U8 *ram)
{
  U16 i;

  // wait for MMC not busy
  if (KO == mmc_sd_wait_not_busy())
    return KO;

  Mmc_sd_select();                  // select MMC_SD
  // issue command
  r1 = mmc_sd_command(MMC_WRITE_BLOCK, gl_ptr_mem);
  // check for valid response
  if(r1 != 0x00)
  {
    Mmc_sd_unselect();
    return KO;
  }
  // send dummy
  Spi_write_data(0xFF);   // give clock again to end transaction

  // send data start token
  Spi_write_data(MMC_STARTBLOCK_WRITE);
  // write data
  for(i=0;i<MMC_SECTOR_SIZE;i++)
  {
    Spi_write_data(*ram);
    ram++;
  }

  Spi_write_data(0xFF);    // send CRC (field required but value ignored)
  Spi_write_data(0xFF);

  // read data response token
  r1 = mmc_sd_send_and_read(0xFF);
  if( (r1&MMC_DR_MASK) != MMC_DR_ACCEPT)
  {
     Spi_write_data(0xFF);    // send dummy bytes
     Spi_write_data(0xFF);
     Mmc_sd_unselect();
     return KO;
//     return r1;             // return ERROR byte
  }

  Spi_write_data(0xFF);    // send dummy bytes
  Spi_write_data(0xFF);

  // release chip select
  Mmc_sd_unselect();                  // unselect MMC_SD
  gl_ptr_mem += 512;        // Update the memory pointer.

  // wait card not busy after last programming operation
  i=0;
  while (KO == mmc_sd_wait_not_busy())
  {
    i++;
    if (i == 10)
      return KO;
  }

  return OK;                  // Write done
}

#endif      // (MMC_SD_RAM == ENABLE)

