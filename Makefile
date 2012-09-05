# Copyright (c) 2012, Jan Vaughan
# All rights reserved.
#
# Makefile for firmware.
#
# OBJECTS ...... Object files to build.
# PROGRAMMER ... Options defining the programmer hardware and the interface it
#                is connected to. Alternatively, leave this undefined and define
#                defaults in ~/.avrduderc, e.g:
#                    default_programmer = "stk500v2"
#                    default_serial = "avrdoper"
# DEVICE ....... AVR hardware device to compile for.
# CLOCK ........ Target AVR clock rate in Hertz.
# FUSES ........ Fuse settings for the device. Calculator available at:
#                    http://www.engbedded.com/fusecalc/

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o oscillator.o main.o
INCLUDES = -Iusbdrv -I.
PROGRAMMER =

DEVICE = attiny85
CLOCK = 16500000
FUSES = -U lfuse:w:0xf1:m -U hfuse:w:0xd7:m -U efuse:w:0xff:m

# Fuse extended byte:
# 0xff = 1 1 1 1   1 1 1 1
#                        ^
#                        +---- SELFPRGEN (disallow self programming)

# Fuse high byte:
# 0xd7 = 1 1 0 1   0 1 1 1
#        ^ ^ ^ ^   ^ \-+-/
#        | | | |   |   +------ BODLEVEL 2..0 (brownout detection disabled)
#        | | | |   +---------- EESAVE (preserve EEPROM on flash)
#        | | | +-------------- WDTON (no watchdog)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ DWEN (no debug wire)
#        +-------------------- RSTDISBL (keep reset pin)
# Fuse low byte:
# 0xf1 = 1 1 1 1   0 0 0 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (16MHz internal HF PLL clock)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ CKOUT (don't share clock)
#        +-------------------- CKDIV8 (no clock division)


# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os $(INCLUDES) -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -DDEBUG_LEVEL=0

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

readcal:
	$(AVRDUDE) -U calibration:r:/dev/stdout:i | head -1

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS) usbdrv/oddebug.s usbdrv/usbdrv.s .DS_Store usbdrv/.DS_Store

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
