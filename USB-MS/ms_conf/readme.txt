How to support new device access mode:

1. conf_access.h
#define  LUN_4                ENABLE    // AVR-DAP

2. config.h
#define SBC_PRODUCT_ID_4      {'A','T','9','0','U','S','B','1','2','8','7',' ','D','P',' ',' '}  // 16 Bytes only

3. conf_dap.h (new)

4. scsi_decoder.c
#     if (LUN_4 == ENABLE)
code    U8    g_sbc_product_id4[16] = SBC_PRODUCT_ID_4;
#     endif

#     if (LUN_4 == ENABLE)
         case LUN_ID_4:
         ptr = (code U8 *) &g_sbc_product_id4;
		 break;
#     endif

5. ctrl_access.h
(dohuja changes)

6. dap.h (new)

7. dap.c (new - add to config.mk)

8. dap_ms_board_drv.h
#define   Dap_select()               (PORTC &= ~0x04)    // CS
#define   Dap_unselect()             (PORTC |=  0x04)



How to add files to the project:
EVK525-series6-ms_nf_df_sd.aps

