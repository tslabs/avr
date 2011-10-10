##############################################################################
# Generic Makefile for following parts AT90USBx ATMegaxUx
###############################################################################

OUTPUT = default

# General Flags
TARGET = $(PROJECT).elf
CC = avr-gcc.exe
MAKECFG   = config.mk

# Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

# Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -fsigned-char -ffunction-sections
CFLAGS += -MD -MP -MT $(OUTPUT)/$(*F).o -MF $(OUTPUT)/dep/$(@F).d 

# Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

# Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(PROJECT).map,--cref,--gc-sections,--relax
LDFLAGS += -Wl,-section-start=.nf_add=0x803200
LDFLAGS += -Wl,-section-start=.nf_cmd=0x803100
LDFLAGS += -Wl,-section-start=.nf_dat=0x803000
LDFLAGS += -Wl,-section-start=.dap_dat=0x803000

# Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom
HEX_FLASH_FLAGS += --set-section-flags=.nf_cmd="noload"
HEX_FLASH_FLAGS += --set-section-flags=.nf_add="noload"
HEX_FLASH_FLAGS += --set-section-flags=.nf_dat="noload"
HEX_FLASH_FLAGS += --set-section-flags=.dap_dat="noload"

# Eeprom file production flags
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0

# Include Directories
INCLUDES = -I"./.." -I"../ms_conf" -I"../lib_board" -I"../lib_mcu" -I"../lib_misc" -I"../lib_mem" -I"../lib_fat" -I"../lib_usb"

# Include Source files list and part informations
include $(MAKECFG)


## Build
.PHONY: build
build: $(TARGET) ../$(PROJECT).hex size

## Clean target
.PHONY: clean
clean:
	@echo "Clean project"
	@-rm -rf $(OUTPUT)/dep/* $(OUTPUT)/* $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).map

## Rebuild the project.
.PHONY: rebuild
rebuild: clean all

## Rebuild the project.
.PHONY: all
all: clean build


## Compile

# Create objects files list with sources files
OBJECTS  = $(CSRCS:.c=.o) $(ASSRCS:.s=.o)

.PHONY: objfiles
objfiles: $(OBJECTS)

# create object files from C source files.
%.o: %.c
	@echo 'Building file: $<'
	@$(shell mkdir $(OUTPUT) 2>/dev/null)
	@$(shell mkdir $(OUTPUT)/dep 2>/dev/null)
	@$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $(OUTPUT)/$(@F)
	
# Preprocess & assemble: create object files from assembler source files.
%.o: %.s
	@echo 'Building file: $<'
	@$(shell mkdir $(OUTPUT) 2>/dev/null)
	@$(shell mkdir $(OUTPUT)/dep 2>/dev/null)
	@$(CC) $(INCLUDES) $(ASMFLAGS) -c $< -o $(OUTPUT)/$(@F)


## Link
$(TARGET): $(OBJECTS)
	@echo "Linking"
	@$(CC) $(LDFLAGS) $(addprefix $(OUTPUT)/,$(notdir $(OBJECTS))) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	@echo "Create hex file"
	@avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	@echo "Create eep file"
	@avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@  || exit 0

%.lss: $(TARGET)
	@echo "Create lss file"
	@avr-objdump -h -S $< > $@

size: ${TARGET}
	@avr-size -C --mcu=${MCU} ${TARGET}

