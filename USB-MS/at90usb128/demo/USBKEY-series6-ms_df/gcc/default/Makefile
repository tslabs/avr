###############################################################################
# Makefile for the project USBKEY-series6-ms_df
###############################################################################

## General Flags
PROJECT = USBKEY-series6-ms_df
MCU = at90usb1286
TARGET = USBKEY-series6-ms_df.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=USBKEY-series6-ms_df.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"c:\@\4\USBKEY-series6-ms_df-2_0_3-doc\at90usb128\demo\USBKEY-series6-ms_df\gcc\..\conf" -I"c:\@\4\USBKEY-series6-ms_df-2_0_3-doc\at90usb128\demo\USBKEY-series6-ms_df\gcc\..\..\.." -I"c:\@\4\USBKEY-series6-ms_df-2_0_3-doc\at90usb128\demo\USBKEY-series6-ms_df\gcc\..\..\..\..\common" -I"c:\@\4\USBKEY-series6-ms_df-2_0_3-doc\at90usb128\demo\USBKEY-series6-ms_df\gcc\.." 

## Objects that must be built in order to link
OBJECTS = main.o ctrl_access.o usb_descriptors.o usb_device_task.o usb_drv.o usb_specific_request.o usb_standard_request.o usb_task.o df_mem.o df.o power_drv.o scheduler.o scsi_decoder.o storage_task.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) USBKEY-series6-ms_df.hex USBKEY-series6-ms_df.eep USBKEY-series6-ms_df.lss size

## Compile
main.o: ../../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ctrl_access.o: ../../../../modules/control_access/ctrl_access.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_descriptors.o: ../../usb_descriptors.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_device_task.o: ../../../../modules/usb/device_chap9/usb_device_task.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_drv.o: ../../../../lib_mcu/usb/usb_drv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_specific_request.o: ../../usb_specific_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_standard_request.o: ../../../../modules/usb/device_chap9/usb_standard_request.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb_task.o: ../../../../modules/usb/usb_task.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

df_mem.o: ../../../../lib_mem/df/df_mem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

df.o: ../../../../lib_mem/df/df.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

power_drv.o: ../../../../lib_mcu/power/power_drv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

scheduler.o: ../../../../../common/modules/scheduler/scheduler.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

scsi_decoder.o: ../../../../modules/scsi_decoder/scsi_decoder.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

storage_task.o: ../../storage_task.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) USBKEY-series6-ms_df.elf dep/* USBKEY-series6-ms_df.hex USBKEY-series6-ms_df.eep USBKEY-series6-ms_df.lss USBKEY-series6-ms_df.map


## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

