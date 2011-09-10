
# Project name
PROJECT = EVK525-series6-ms_nf_df_sd

# CPU architecture : {avr0|...|avr6}
# Parts : {at90usb646|at90usb647|at90usb1286|at90usb1287|at90usb162|at90usb82}
MCU = at90usb1287

# Source files
CSRCS = \
  ../main.c\
  ../storage_task.c\
  ../usb_descriptors.c\
  ../usb_specific_request.c\
  ../../../modules/usb/usb_task.c\
  ../../../modules/usb/device_chap9/usb_device_task.c\
  ../../../modules/usb/device_chap9/usb_standard_request.c\
  ../../../modules/control_access/ctrl_access.c\
  ../../../modules/scsi_decoder/scsi_decoder.c\
  ../../../lib_mem/df/df.c\
  ../../../lib_mem/df/df_mem.c\
  ../../../lib_mem/mmc_sd/mmc_sd.c\
  ../../../lib_mem/mmc_sd/mmc_sd_mem.c\
  ../../../lib_mem/nf/nf_drv.c\
  ../../../lib_mem/nf/nf_mngt.c\
  ../../../lib_mem/nf/nf_unusual.c\
  ../../../lib_mem/nf/nf_raw.c\
  ../../../lib_mcu/power/power_drv.c\
  ../../../lib_mcu/debug.c\
  ../../../lib_mcu/usb/usb_drv.c\
  ../../../../common/modules/scheduler/scheduler.c\
  ../../../../common/lib_mcu/uart/uart_lib.c\
  ../../../../common/lib_mcu/wdt/wdt_drv.c\


# Assembler source files
ASSRCS = \

