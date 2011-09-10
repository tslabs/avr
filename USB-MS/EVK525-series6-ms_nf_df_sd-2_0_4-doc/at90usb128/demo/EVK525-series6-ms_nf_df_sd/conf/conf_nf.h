/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file contains the possible external configuration of the Nand Flash interface
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

#ifndef _CONF_NF_H_
#define _CONF_NF_H_

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N ________________________________________________

// The list of the supported Nand Flash is available in "support_NF_Driver_AVR.pdf" file.


// !!!!!! WARNING !!!!!!!!
// !! The auto-detect mode (#define NF_AUTO_...) may be less speed that static compilation (#define NF_TYPE_...)


//! ************ Auto-detect mode
//!
//! Values: TRUE : Firmware will autodetect
//!         FALSE: Firmware will not autodetect
//! Note: the 2KB and 512B detect can't be enable in same time
//!
#define NF_AUTO_DETECT_2KB      FALSE
#define NF_AUTO_DETECT_512B     FALSE



//! ************ Static mode
//!  To enable a reference, you shall define a NF_TYPE_X (X = reference)
#define NF_TYPE_K9F1G08U0A
// #define NF_TYPE_HY27UF082G2M


//! ************ For all mode
//! Define the number of NandFlash connected (= number of Chip Select)
#define NF_N_DEVICES            1


// This enables RAW access to NAND bulk
// (instead of messy 'chinese-flash-stick-controller' type)
// It does not allow to write data, only to read (at least by now)
// TSL'2011
#define NF_RAW TRUE


// ******** Exemples **********
//
// You have connected one MT29F2G08AACWP, you shall set
// #define NF_TYPE_MT29F2G08AACWP
// #define NF_N_DEVICES    1
//
// You have connected two MT29F2G08AACWP, you shall set
// #define NF_TYPE_MT29F2G08AACWP
// #define NF_N_DEVICES    2
//
// You have connected one MT29F16G08FAAWC, you shall set
// #define NF_TYPE_MT29F16G08FAAWC
// #define NF_N_DEVICES    2  // 2 because this reference use 2 Chip Select
//
// You have connected two MT29F16G08FAAWC, you shall set
// #define NF_TYPE_MT29F16G08FAAWC
// #define NF_N_DEVICES    4  // 4 because this reference use 2 Chip Select AND you have 2 NandFlash
//
// *** END OF Exemples ***


//!!!! Don't change following parameters
#define ERASING_ALL            DISABLE // erase the whole flash then hangs
#define NF_CACHE_LUT_LOG_SZ         64 // number of logical blocks cached from the LUT
#define NF_CACHE_FBB_LOG_SZ         32 // number of logical blocks cached from the Free-blocks Block

//! Function linker for NF access indications
//!
//! Values :   Nf_access_signal_on()      The linked function is called when a read/write operation to NF starts
//!            Nf_access_signal_off()     The linked function is called when the read/write operation to NF ends
//!
#define  Nf_access_signal_on()		Led0_on();
#define  Nf_access_signal_off()		Led0_off();


//! External Memory Interface settings
//! This driver uses the XMCR interface to accede to the NF, but the port could be shared with another peripheral (like a LCD module)
//!
//! Values: ENABLED :   the port driven by XMCR is also used for another peripheral, so XMCR will be disabled (port free of use)
//!                     when the memory is not acceeded
//!                     This is done automatically while user only directly calls the following functions :
//!                        - nfc_check_type(), nfc_detect(), nf_init()                                (that must be called at start up)
//!                        - nf_test_unit_ready(), nf_read_capacity(), nf_read_10(), nf_write_10()    (that are called by ctrl_access.c)
//!                        - nf_usb_stop()                                                            (called after suspend/disconnect USB event)
//!         DISABLED :  the port driven by XMCR is not shared, so it is dedicated to the NF and is not accessible for direct user access
//!
#define  NF_XMCR_MODULE_SHARED         DISABLED
#define  NF_CLE_ALE_MANUAL             DISABLED



#endif // _CONF_NF_H_
