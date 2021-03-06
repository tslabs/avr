//! \file *********************************************************************
//!
//! \brief This file is the header file for the high level management of the
//!
//!  AVR DAP.
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB1287, AT90USB1286, AT90USB647, AT90USB646
//!
//! ***************************************************************************


#ifndef _DAP_H_
#define _DAP_H_


//_____ INCLUDES ____________________________________________________

#include "config.h"
#include "conf_dap.h"
#include "ctrl_status.h"

//________ DEFINES _______________________

#define  DAP_DATA           0x3000

//_____ FUNCTIONS __________________________________________________

Ctrl_status    dap_test_unit_ready      ( void ) ;
Ctrl_status    dap_read_capacity        ( U32* ) ;
Bool           dap_wr_protect           ( void ) ;
Bool           dap_removal              ( void ) ;
Ctrl_status    dap_read_10			    ( U32 log_sector , U16 n_sectors );
Ctrl_status    dap_write_10		  	    ( U32 log_sector , U16 n_sectors );
Ctrl_status    dap_ram_2_dap            ( U32 addr, U8 *ram );
Ctrl_status    dap_dap_2_ram            ( U32 addr, U8 *ram );

#endif  // _DAP_H_
