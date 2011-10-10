
# Project name
PROJECT = avr_usb_ms

# CPU architecture : {avr0|...|avr6}
# Parts : {at90usb646|at90usb647|at90usb1286|at90usb1287|at90usb162|at90usb82}
MCU = at90usb1287

# Source files
CSRCS = \
  ../lib_mcu/power_drv.c            \
  ../lib_mcu/uart_lib.c             \
  ../lib_mcu/usb_drv.c              \
  ../lib_mcu/wdt_drv.c              \
  ../lib_mem/dap.c                  \
  ../lib_mem/df.c                   \
  ../lib_mem/df_mem.c               \
  ../lib_mem/mmc_sd.c               \
  ../lib_mem/mmc_sd_mem.c           \
  ../lib_mem/nf_drv.c               \
  ../lib_mem/nf_mngt.c              \
  ../lib_mem/nf_raw.c               \
  ../lib_mem/nf_unusual.c           \
  ../lib_misc/ctrl_access.c         \
  ../lib_misc/debug.c               \
  ../lib_misc/main.c                \
  ../lib_misc/scheduler.c           \
  ../lib_misc/scsi_decoder.c        \
  ../lib_misc/storage_task.c        \
  ../lib_usb/usb_descriptors.c     \
  ../lib_usb/usb_device_task.c     \
  ../lib_usb/usb_specific_request.c\
  ../lib_usb/usb_standard_request.c\
  ../lib_usb/usb_task.c            \


# Assembler source files
ASSRCS = \

