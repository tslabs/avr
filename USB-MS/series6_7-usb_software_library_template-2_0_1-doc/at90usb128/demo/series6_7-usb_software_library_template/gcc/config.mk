
# Project name
PROJECT = series6_7-usb_software_library_template

# CPU architecture : {avr0|...|avr6}
# Parts : {at90usb646|at90usb647|at90usb1286|at90usb1287|at90usb162|at90usb82}
MCU = at90usb1287

# Source files
CSRCS = \
  ../../../../common/lib_mcu/wdt/wdt_drv.c\
  ../../../lib_mcu/usb/usb_drv.c\
  ../main.c\
  ../../../lib_mcu/power/power_drv.c\
  ../../../../common/modules/scheduler/scheduler.c\
  ../../../modules/usb/device_chap9/usb_device_task.c\
  ../usb_specific_request.c\
  ../../../modules/usb/device_chap9/usb_standard_request.c\
  ../../../modules/usb/host_chap9/usb_host_enum.c\
  ../../../modules/usb/host_chap9/usb_host_task.c\
  ../../../modules/usb/usb_task.c\
  ../../../../common/lib_mcu/uart/uart_lib.c\
  ../device_template_task.c\
  ../host_template_task.c\
  ../usb_descriptors.c\

# Assembler source files
ASSRCS = \

