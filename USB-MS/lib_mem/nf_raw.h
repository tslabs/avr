#ifndef _NFC_RAW_H_
#define _NFC_RAW_H_

// low level functions
void		nf_raw_init(void);
U32			nf_raw_read_id(void);

// SCSI functions
Ctrl_status    nf_raw_test_unit_ready         ( void ) ;
Ctrl_status    nf_raw_read_capacity           ( U32* ) ;
Bool           nf_raw_wr_protect              ( void ) ;
Bool           nf_raw_removal                 ( void ) ;
Ctrl_status    nf_raw_read_10( U32 log_sector , U16 n_sectors);
Ctrl_status    nf_raw_write_10( U32 log_sector , U16 n_sectors);
Ctrl_status    nf_raw_ram_2_nf                (U32 addr, U8 *ram);
Ctrl_status    nf_raw_nf_2_ram                (U32 addr, U8 *ram);




#endif  // _NFC_RAW_H_
