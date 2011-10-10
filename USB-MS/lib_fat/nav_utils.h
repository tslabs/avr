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

#ifndef _NAV_UTILS_H_
#define _NAV_UTILS_H_

//_____ I N C L U D E S ____________________________________________________


#include "config.h"
#include "fat.h"
#include "fs_com.h"
#include "navigation.h"

//_____ M A C R O S ________________________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

#ifndef MAX_FILE_LENGHT
   #warning MAX_FILE_LENGHT not defined in config file, using default
   #define MAX_FILE_LENGHT 30
#endif


#ifdef __GNUC__
U8 goto_code_name(U8 *str, Bool b_case_sensitive, Bool b_create);
void str_code_to_str_ram(U8 string_code[],U8 str_ram[MAX_FILE_LENGHT]);
Bool copy_dir( U8 string_src[], U8 string_dst[], Bool b_print  );
#else
U8 goto_code_name(U8 code *str, Bool b_case_sensitive, Bool b_create);
void str_code_to_str_ram(U8 code string_code[],U8 str_ram[MAX_FILE_LENGHT]);
Bool copy_dir( U8 code string_src[], U8 code string_dst[], Bool b_print  );
#endif

#endif /* _NAV_UTILS_H_ */
