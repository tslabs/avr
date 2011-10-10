//
// Module for RAW working with NAND bulk
// Useful if you need to read out the data from dead flash stick
// Only 1 CE supported
// (c) TS-Labs '2011
//
// Particularities (and differences with 'canonical' version):
//	- readiness of NAND is checked with R/~B signal, not by Status Read command
// ATTENTION!
// The NAND has a delay before R/~B becomes valid, so if you poll it right after
// a command sent and received 'ready' state, it is INVALID until next poll!!!
//
// NF_CLE_ALE_MANUAL must be DISABLED


//_____  I N C L U D E S ___________________________________________________

#define  _nfc_raw_c_

#include "config.h"
#include "conf_nf.h"
#include "nf.h"
#include "nf_drv.h"
#include "nf_mngt.h"
#include "usb_drv.h"            /* usb driver definition */
#include "debug.h"


//_____ D E C L A R A T I O N ______________________________________________
//

#ifndef __GNUC__
  extern __no_init volatile xdata Byte nf_send_cmd At(NF_CMD_LATCH_ENABLE_ADD);  // Command
  extern __no_init volatile xdata Byte nf_send_add At(NF_ADD_LATCH_ENABLE_ADD);  // Address
  extern __no_init volatile xdata Byte nf_data At(NF_ADDRESS_CMD_DATA);          // Data
#else
  extern volatile unsigned char nf_send_cmd __attribute__ ((section (".nf_cmd")));
  extern volatile unsigned char nf_send_add __attribute__ ((section (".nf_add")));
  extern volatile unsigned char nf_data     __attribute__ ((section (".nf_dat")));
#endif

const U32 nf_sectors_number = 2L*1024L*2048L;		//size of bulk

//_____ D E F I N I T I O N ________________________________________________



//_____ M A C R O S ________________________________________________________



//_____ P R I V A T E    FUNCTION   D E C L A R A T I O N _____________________________

U8  				nf_raw_xfer_update_vars(void);
void				nf_raw_write_sector_from_usb(U8);
void				nf_raw_read_sector_to_usb(U8);
void				nf_raw_open_page_read(U32 sect);


//_____ P R I V A T E    D E C L A R A T I O N _____________________________

// Static definition, which can be optimized by the compiler
//
// static _MEM_TYPE_FAST_		U16		s_n_sectors                  ; // Holds the number of sectors to read/write
// static _MEM_TYPE_FAST_		U32		s_curr_log_sector            ; // Holds the logical sector number
// static _MEM_TYPE_SLOW_		U32		s_save_log_addr              ; // Used for Stand-by / Restart operations
// static _MEM_TYPE_FAST_    U16       s_curr_n_byte                ; // Holds the position in the page
       // _MEM_TYPE_FAST_    U8        g_curr_dev_id                ; // Holds the current device number that is used
       // _MEM_TYPE_SLOW_    U8        g_save_curr_dev_id                ; // Holds the previous device number that is used
       // _MEM_TYPE_FAST_    U32       g_phys_page_addr[NF_N_DEVICES]    ; // Holds the current phys page number for each device
       // _MEM_TYPE_SLOW_    U32       g_save_phys_page_addr             ; // Holds the previous phys page number
       // _MEM_TYPE_FAST_    U32       g_next_phys_page_addr             ; // Holds the previous phys page number	   

// U8                 nf_raw_xfer_update_vars(void);
// static _MEM_TYPE_FAST_    U8        s_nb_sectors_step            ; // Holds the number of sectors read after each page
// static _MEM_TYPE_FAST_    Nf_state  s_state                      ; // Holds the current state of the driver

							// Bool	g_fatal       ; // Used in LUT/FBB building and ECC management...
// static 						Bool 	s_mem         ;
// static 						Bool 	s_start       ;

typedef enum
{
   STATE_READ_INIT=0       // The very first open_read must be done
,  STATE_READ_RESUME_PAGE  // A page has been read
,  STATE_WRITE_INIT        // The very first open_write must be done
,  STATE_WRITE_RESUME_PAGE // A page has been written
,  STATE_COMPLETE          // The read or write session is over.
} Nf_state;


//_____ SCSI FUNCTIONS __________________________________________________
//
//

Ctrl_status    nf_raw_test_unit_ready         ( void )
{
	return CTRL_GOOD;
}


Ctrl_status    nf_raw_read_capacity           (U32  *u32_nb_sector )
{
	*u32_nb_sector = nf_sectors_number-1;
	return CTRL_GOOD;
}


Ctrl_status nf_raw_read_10( U32 log_sector , U16 n_sectors)
{
	// U8  status;
	
	// Test that the logical sector address is valid
	if ( 0==n_sectors )	{ return CTRL_GOOD; }
	if ( (log_sector+n_sectors) > nf_sectors_number ) { return CTRL_FAIL; }
	
	// trace("read, curr_sector:"); trace_hex32(log_sector); trace(", n_sectors: "); trace_hex16(n_sectors); trace_nl();
	
	Nf_access_signal_on();	// LED - currently undefined
	nf_XMCR_enable();
	nfc_select_dev(0);
	
	while (n_sectors > 0)
	{
	nf_raw_open_page_read(log_sector);
	nf_raw_read_sector_to_usb(1);
	n_sectors -= 1; log_sector += 1;
	}
	
	Nf_access_signal_off();
	Nandflash_unselect();
	nf_XMCR_disable();
	
	return CTRL_GOOD;
}


Ctrl_status    nf_raw_write_10( U32 log_sector , U16 n_sectors)
{
	return CTRL_GOOD;
}


Ctrl_status    nf_raw_ram_2_nf                (U32 addr, U8 *ram)
{
	return CTRL_GOOD;
}


Ctrl_status    nf_raw_nf_2_ram                (U32 addr, U8 *ram)
{
	return CTRL_GOOD;
}

Bool           nf_raw_wr_protect              ( void )
{
	return TRUE;
}


Bool           nf_raw_removal                 ( void )
{
	return TRUE;
}


//_____ PRIVATE FUNCTIONS _________________________________
//




//_____ LOW LEVEL FUNCTIONS __________________________________________________
//
//

U32         nf_raw_read_id(void)
{
U32		nf_id;

	nf_XMCR_enable();
	nfc_select_dev(0);
	
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	nf_send_cmd = NF_RESET_CMD;
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	nf_send_cmd = NF_READ_ID_CMD;
	nf_send_add = 0;
	MSB0(nf_id) = (nf_data); // Maker Code
	MSB1(nf_id) = (nf_data); // Device Id
	MSB2(nf_id) = (nf_data); // extra
	MSB3(nf_id) = (nf_data); // extra (Multi Plane Support)

	Nandflash_unselect();
	nf_XMCR_disable();

	return (nf_id);
}


void nf_raw_read_sector_to_usb(U8 nb_sectors)
{
 U8 j,i;

   for (j = 8*nb_sectors; j != 0; j--)                      // 8 * 64 bytes = 512 bytes
   {
      Disable_interrupt();
      for (i = 0; i<64; i++)
	  {
		Usb_write_byte(nf_data);                         // read 64 bytes from card
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

void nf_raw_open_page_read(U32 sect)
{
  U8 tmp1,tmp2,tmp3,tmp4;

	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());

	// 4KB per page pattern
	// tmp1 = (sect & 0x07) << 1;
	// tmp2 =  sect >> 3;
	// tmp3 =  sect >> 11;
	// tmp4 =  sect >> 19;
   
	// 2KB per page pattern
	tmp1 = (sect & 0x03) << 1;
	tmp2 =  sect >> 2;
	tmp3 =  sect >> 10;
	tmp4 =  sect >> 18;
   
	nf_send_cmd = NF_READ_CMD;
	nf_send_add = 0;
 	nf_send_add = tmp1;
 	nf_send_add = tmp2;
 	nf_send_add = tmp3;
 	nf_send_add = tmp4;
	nf_send_cmd = NF_READ_CMD2;
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());
	while(!Is_nandflash0_ready());

	// trace_hex(tmp4);	trace(" ");
	// trace_hex(tmp3);	trace(" ");
	// trace_hex(tmp2);	trace(" ");
	// trace_hex(tmp1);	trace(" ");
	// trace_hex(0);
	// trace_nl();
		

}
