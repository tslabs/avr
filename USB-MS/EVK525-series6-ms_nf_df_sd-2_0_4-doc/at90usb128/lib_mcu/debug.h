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

#ifndef _DEBUG_H_
#define _DEBUG_H_

//_____ I N C L U D E S ____________________________________________________
#include <stdio.h>

//_____ M A C R O S ________________________________________________________
//! This is Compilation switches definition
//#define SOFT_OCD          // when OCD dongle is not used, only VT100 or hyperterminal

#ifndef _ASSERT_
// Disable this switch to remove the Assert code from the compilation
// Enable it to add the Assert code from the compilation
#define _ASSERT_ (DISABLE) // default value
#endif

//! This macro is used to test fatal errors which may be caused by
//! software or hardware bugs.
//!
//! The macro tests if the expression is TRUE. If it is not, a fatal error
//! is detected and the application hangs.
//!
//! @param expr   expression which is supposed to be TRUE.
#if (_ASSERT_==ENABLE)
#  define Assert( expr )      \
   {                          \
      if( !(expr) )           \
      {                       \
         trace("\n\r");       \
         trace(__FILE__);     \
         trace(":");          \
         trace_u16(__LINE__); \
         while(1) {           \
         }                    \
      }                       \
   }
#else
#  define Assert( expr )
#endif


extern U8 _MEM_TYPE_SLOW_ g_trace_en;

#if (_TRACE_==ENABLE)
void    trace        ( const char* str );
void    trace_nl     ( void    );
void    trace_hex    ( U8  val );
void    trace_hex16  ( U16 val );
void    trace_hex32  ( U32 val );
void    trace_u8     ( U8  val );
void    trace_u16    ( U16 val );
void    trace_u32    ( U32 val );
void    trace_ptwo   ( U8  hex );

#else

#  define trace(        str )
#  define trace_nl(         )
#  define trace_hex(    val )
#  define trace_hex16(  val )
#  define trace_hex32(  val )
#  define trace_u8(     val )
#  define trace_u16(    val )
#  define trace_u32(    val )
#  define trace_ptwo(   hex )
#endif

#if (_TRACE_!=ENABLE) && (_TRACE_!=DISABLE)
#  error _TRACE_ can only be defined to ENABLE or DISABLE
#endif
#if (_ASSERT_!=ENABLE) && (_ASSERT_!=DISABLE)
#  error _ASSERT_ can only be defined to ENABLE or DISABLE
#endif

#endif  // _DEBUG_H_
