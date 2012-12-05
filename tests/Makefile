# Name: Makefile
# Project: custom-class example
# Author: Christian Starkjohann
# Creation Date: 2008-04-07
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

DEVICE  = attiny2313
F_CPU   = 16000000	# in Hz
DEFINES =

CFLAGS  = $(DEFINES) -Iusbdrv -I. -DDEBUG_LEVEL=0
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o

COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

SIZES_TMP = /tmp/sizetmp.txt

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make sizes ..... compute code and RAM sizes for various options"
	@echo "make test ...... test with all features whether everything compiles"

sizes sizes.txt:
	rm -f $(SIZES_TMP) sizes.txt
	$(MAKE) null.elf
	avr-size null.elf | tail -1 | awk '{print "null", $$1+$$2, $$3+$$2}' >$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_16_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=12000000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_12_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=12800000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_12_8_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=15000000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_15_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=16500000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_16_5_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=18000000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_18_MHz+CRC", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf F_CPU=20000000
	avr-size main.elf | tail -1 | awk '{print "Minimum_with_20_MHz", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf DEFINES=-DUSB_CFG_IMPLEMENT_FN_WRITE=1
	avr-size main.elf | tail -1 | awk '{print "With_usbFunctionWrite", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf DEFINES=-DUSB_CFG_IMPLEMENT_FN_READ=1
	avr-size main.elf | tail -1 | awk '{print "With_usbFunctionRead", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_IMPLEMENT_FN_READ=1 -DUSB_CFG_IMPLEMENT_FN_WRITE=1"
	avr-size main.elf | tail -1 | awk '{print "With_usbFunctionRead_and_Write", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_IMPLEMENT_FN_WRITEOUT=1"
	avr-size main.elf | tail -1 | awk '{print "With_usbFunctionWriteOut", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_HAVE_INTRIN_ENDPOINT=1"
	avr-size main.elf | tail -1 | awk '{print "With_Interrupt_In_Endpoint_1", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_IMPLEMENT_HALT=1 -DUSB_CFG_HAVE_INTRIN_ENDPOINT=1"
	avr-size main.elf | tail -1 | awk '{print "With_Interrupt_In_Endpoint_1_and_Halt", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_HAVE_INTRIN_ENDPOINT3=1"
	avr-size main.elf | tail -1 | awk '{print "With_Interrupt_In_Endpoint_1_and_3", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSE_DYNAMIC_DESCRIPTOR=1"
	avr-size main.elf | tail -1 | awk '{print "With_Dynamic_Descriptor", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	$(MAKE) clean; $(MAKE) main.elf "DEFINES=-DUSB_CFG_LONG_TRANSFERS=1"
	avr-size main.elf | tail -1 | awk '{print "With_Long_Transfers", $$1+$$2, $$3+$$2}' >>$(SIZES_TMP)
	cat $(SIZES_TMP) | awk 'BEGIN{printf("%39s %5s %5s %5s %5s\n"), "Variation", "Flash", "RAM", "+F", "+RAM"}\
		/^null/{nullRom=$$2; nullRam=$$3; next} \
		{rom=$$2-nullRom; ram=$$3-nullRam; if(!refRom){refRom=rom; refRam=ram} \
		printf("%39s %5d %5d %+5d %+5d\n", $$1, rom, ram, rom-refRom, ram-refRam)}' | tee sizes.txt
	rm $(SIZES_TMP)

test:
	for freq in 12000000 12800000 15000000 16000000 16500000 18000000 20000000; do \
		for opt in USB_COUNT_SOF USB_CFG_HAVE_INTRIN_ENDPOINT USB_CFG_HAVE_INTRIN_ENDPOINT3 USB_CFG_HAVE_MEASURE_FRAME_LENGTH USB_CFG_LONG_TRANSFERS; do \
			$(MAKE) clean; $(MAKE) main.elf F_CPU=$$freq "DEFINES=-D$$opt=1" || exit 1; \
			$(MAKE) clean; $(MAKE) main.elf F_CPU=$$freq "DEFINES=-D$$opt=1 -DDUSB_CFG_IMPLEMENT_FN_WRITEOUT=1" || exit 1; \
		done \
	done

# The following rule is used to check the compiler
devices: #exclude devices without RAM for stack and atmega603 for gcc 3
	excludes="at90s1200 attiny11 attiny12 attiny15 attiny28"; \
	for gccVersion in 3 4; do \
		avr-gcc-select $$gccVersion; \
		for device in `echo | avr-gcc -xc -mmcu=x - 2>&1 | egrep '^ *at[a-zA-Z0-9_-]+$$'`; do \
			if echo "$$excludes" | grep "$$device" >/dev/null; then continue; fi; \
			if [ "$$gccVersion" = 3 -a "$$device" = atmega603 ]; then continue; fi; \
			$(MAKE) clean; $(MAKE) null.elf DEVICE=$$device || exit 1; \
		done \
	done
	$(MAKE) clean
	avr-gcc-select 3
	@echo "+++ Device test succeeded!"

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f *.hex *.lst *.map  *.elf *.o
	rm -rf usbdrv

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

# Since we don't want to ship the driver multipe times, we copy it into this project:
usbdrv:
	cp -r ../usbdrv .

main.elf: usbdrv $(OBJECTS)	# usbdrv dependency only needed because we copy it
	$(COMPILE) -o main.elf $(OBJECTS)

main_i.elf: usbdrv main.o usbdrv/usbdrvasm.o	# usbdrv dependency only needed because we copy it
	$(COMPILE) -o main_i.elf main.o usbdrv/usbdrvasm.o

null.elf: null.o
	$(COMPILE) -o null.elf null.o
