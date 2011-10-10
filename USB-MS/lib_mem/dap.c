//
// Module for AVR-DAP
//
//_____  I N C L U D E S ___________________________________________________

#define  _dap_c_

#include "config.h"
#include "conf_dap.h"
#include "dap.h"
#include "usb_drv.h"            /* usb driver definition */
#include "debug.h"


//_____ D E C L A R A T I O N ______________________________________________

#ifndef __GNUC__
  __no_init volatile xdata Byte dap_data At(DAP_DATA);
#else
  volatile unsigned char dap_data __attribute__ ((section (".dap_dat")));
#endif

const U32 dap_sectors_number = 16L*2048L;		//size of bulk = 16MB


//_____ PRIVATE FUNCTION DECLARATION _____________________________

void				dap_write_sector_from_usb(U8);
void				dap_read_sector_to_usb(U8);
void				dap_set_sect(U32);
void				dap_set_addr(U32);
void				dap_XMCR_enable();
void				dap_XMCR_disable();


//_____ SCSI FUNCTIONS __________________________________________________
//
//

Ctrl_status    dap_test_unit_ready         ( void )
{
	return CTRL_GOOD;
}


Ctrl_status    dap_read_capacity           (U32  *u32_nb_sector )
{
	*u32_nb_sector = dap_sectors_number-1;
	return CTRL_GOOD;
}


Ctrl_status dap_read_10( U32 log_sector , U16 n_sectors)
{
	// U8  status;
	
	// Test that the logical sector address is valid
	if ( 0==n_sectors )	{ return CTRL_GOOD; }
	if ( (log_sector+n_sectors) > dap_sectors_number ) { return CTRL_FAIL; }
	
	// trace("read, curr_sector:"); trace_hex32(log_sector); trace(", n_sectors: "); trace_hex16(n_sectors); trace_nl();
	
	Dap_access_signal_on();		// LED - currently undefined
	dap_XMCR_enable();
	Dap_select();
	
	dap_set_sect(log_sector);
	
	while (n_sectors > 0)
	{
	dap_read_sector_to_usb(1);
	n_sectors -= 1;
	}
	
	Dap_access_signal_off();
	Dap_unselect();
	dap_XMCR_disable();
	
	return CTRL_GOOD;
}


Ctrl_status    dap_write_10( U32 log_sector , U16 n_sectors)
{
	return CTRL_GOOD;
}


Ctrl_status    dap_ram_2_dap                (U32 addr, U8 *ram)
{
	return CTRL_GOOD;
}


Ctrl_status    dap_dap_2_ram                (U32 addr, U8 *ram)
{
	return CTRL_GOOD;
}

Bool           dap_wr_protect              ( void )
{
	return TRUE;
}


Bool           dap_removal                 ( void )
{
	return TRUE;
}


//_____ LOW LEVEL FUNCTIONS __________________________________________________
//
//

void dap_read_sector_to_usb(U8 nb_sectors)
{
 U8 j,i;

   for (j = 8*nb_sectors; j != 0; j--)                      // 8 * 64 bytes = 512 bytes
   {
      Disable_interrupt();
      for (i = 0; i<64; i++)
	  {
		Usb_write_byte(dap_data);                         // read 64 bytes from card
	  }
      Enable_interrupt();

      Usb_send_in();                            // validate transfer
      while(Is_usb_write_enabled()==FALSE)
      {
         if(!Is_usb_endpoint_enabled())
            return; // USB Reset
      }
   }
}


void dap_set_sect(U32 sect)
{
	dap_set_addr(sect<<8);
}


void dap_set_addr(U32 addr)
{
	dap_data = (addr & 0xFF);
	dap_data = (addr>>8 & 0xFF);
	dap_data = (addr>>16 & 0xFF);

}


void dap_XMCR_enable(void)
{
  XMCRB |= ((1<<XMM2) | (1<<XMM1));                // limit XRAM interface to A9 (release PC2..7)

// enable the external memory 
// SRWn[1:0] delay: 00 - 1 tact / 01 - 2 tacts / 10 - 3 tacts
//RE_n, WE_n = 62.5ns (1 clk 16MHz)
  XMCRA |= (1<<SRE);                  		

//RE_n, WE_n = 187.5ns (3 clks 16MHz)
  // XMCRA |= ((1<<SRE) | (1<<SRW01) | (0<<SRW00) | (1<<SRW11) | (0<<SRW10));
}


void dap_XMCR_disable(void)
{
  XMCRA &= ~(1<<SRE);  // disable the external memory
}