/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file holds definitions to help software designers in debugging
//!
//!  their applications.
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
#include "debug.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________
// Board Config


//_____ D E C L A R A T I O N S ____________________________________________



#if (_TRACE_==ENABLE)
U8 _MEM_TYPE_SLOW_ g_trace_en=TRUE;



//! Fonction used to display a byte value in the decimal form (16 bits) on OCD/Serial Debug Interface
//!
//! @param val: value of byte
//!
void trace_u32( U32 val )
{
   if( FALSE==g_trace_en ) return;
   printf("%lu",val);
}

//! Fonction used to display a 16-bit value in the decimal form on OCD/Serial Debug Interface
//!
//! @param val: value of byte
//!
void trace_u16( U16 val )
{
   if( FALSE==g_trace_en ) return;
   printf("%u",val);
}

//! Fonction used to display a byte value in the decimal form on OCD/Serial Debug Interface
//!
//! @param val: value of byte
//!
void trace_u8( U8 val )
{
   if( FALSE==g_trace_en ) return;
   printf("%u",val);
}

//! Fonction used to display a byte value in the hex form on OCD/Serial Debug Interface
//!
//! @param val: value of byte
//!
void trace_hex( U8 val )
{
   if( FALSE==g_trace_en ) return;
   printf("%02X",val);
}


//! Fonction used for send a texte on OCD/Serial Debug Interface
//!
//! @param str: texte to send (max. size = 256)
//!
void trace( const U8* str )
{
   if( FALSE==g_trace_en ) return;
   printf((const char*)str);
}


void trace_hex16(U16 val)
{
   printf("%04X",val);
}

void trace_hex32(U32 val)
{
   printf("%08lX",val);
}

void trace_nl()
{
   trace("\n\r");
}

#endif

