This is the Readme file for the hid-data example. In this example, we show
how blocks of data can be exchanged with the device using only functionality
compliant to the HID class. Since class drivers for HID are included with
Windows, you don't need to install drivers on Windows.


WHAT IS DEMONSTRATED?
=====================
This example demonstrates how the HID class can be misused to transfer fixed
size blocks of data (up to the driver's transfer size limit) over HID feature
reports. This technique is of great value on Windows because no driver DLLs
are needed (the hid-custom-rq example still requires the libusb-win32 DLL,
although it may be in the program's directory). The host side application
requires no installation, it can even be started directly from a CD. This
example also demonstrates how to transfer data using usbFunctionWrite() and
usbFunctionRead().


PREREQUISITES
=============
Target hardware: You need an AVR based circuit based on one of the examples
(see the "circuits" directory at the top level of this package), e.g. the
metaboard (http://www.obdev.at/goto.php?t=metaboard).

AVR development environment: You need the gcc tool chain for the AVR, see
the Prerequisites section in the top level Readme file for how to obtain it.

Host development environment: A C compiler and libusb on Unix. On Windows
you need the Driver Development Kit (DDK) Instead of libusb. MinGW ships
with a free version of the DDK.


BUILDING THE FIRMWARE
=====================
Change to the "firmware" directory and modify Makefile according to your
architecture (CPU clock, target device, fuse values) and ISP programmer. Then
edit usbconfig.h according to your pin assignments for D+ and D-. The default
settings are for the metaboard hardware.

Type "make hex" to build main.hex, then "make flash" to upload the firmware
to the device. Don't forget to run "make fuse" once to program the fuses. If
you use a prototyping board with boot loader, follow the instructions of the
boot loader instead.

Please note that the first "make hex" copies the driver from the top level
into the firmware directory. If you use a different build system than our
Makefile, you must copy the driver by hand.


BUILDING THE HOST SOFTWARE
==========================
Make sure that you have libusb (on Unix) or the DDK (on Windows) installed.
We recommend MinGW on Windows since it includes a free version of the DDK.
Then change to directory "commandline" and run "make" on Unix or
"make -f Makefile.windows" on Windows.


USING THE COMMAND LINE TOOL
===========================
The device implements a data store of 128 bytes in EEPROM. You can send a
block of 128 bytes to the device or read the block using the command line
tool.

To send a block to the device, use e.g.

    hidtool write 0x01,0x02,0x03,0x04,...

and to receive the block, use

    hidtool read


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
