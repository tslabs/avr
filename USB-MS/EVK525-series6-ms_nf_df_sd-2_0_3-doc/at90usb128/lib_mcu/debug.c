//! @file $RCSfile: debug.c,v $
//!
//! Copyright (c) 2004 Atmel.
//!
//! Please read file license.txt for copyright notice.
//!
//! @brief This file contains the routines of debug trace and assert
//!
//! @version $Revision: 1.18 $ $Name: snd3-last $ $Id: debug.c,v 1.18 2006/12/06 17:46:41 coger Exp $
//!
//! @todo
//! @bug

#define _TRACE_ (DISABLE)
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
   printf("0x%02X",val);
}
#endif

#if (_TRACE_==ENABLE)
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
   printf("0x%04X",val);
}

void trace_hex32(U32 val)
{
   printf("0x%08lX",val);
}

void trace_nl()
{
   trace("\n\r");
}

#endif

