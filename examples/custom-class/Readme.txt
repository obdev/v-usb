This is the Readme file for the custom-class example. In this example, we
show how an LED can be controlled via USB.


WHAT IS DEMONSTRATED?
=====================
This example shows how small amounts of data (several bytes) can be
transferred between the device and the host. In addition to a very basic
USB device, it demonstrates how to build a host side driver application
using libusb or libusb-win32. It does NOT show how usbFunctionWrite() and
usbFunctionRead() are used. See the hid-data example if you want to learn
about these functions.


PREREQUISITES
=============
Target hardware: You need an AVR based circuit based on one of the examples
(see the "circuits" directory at the top level of this package), e.g. the
metaboard (http://www.obdev.at/goto.php?t=metaboard).

AVR development environment: You need the gcc tool chain for the AVR, see
the Prerequisites section in the top level Readme file for how to obtain it.

Host development environment: A C compiler and libusb. See the top level
Readme file, section Prerequisites for more information.


BUILDING THE FIRMWARE
=====================
Change to the "firmware" directory and modify Makefile according to your
architecture (CPU clock, target device, fuse values) and ISP programmer. Then
edit usbconfig.h according to your pin assignments for D+ and D-. The default
settings are for the metaboard hardware. You should have wired an LED with a
current limiting resistor of ca. 270 Ohm to a free I/O pin. Change the
defines in main.c to match the port and bit number.

Type "make hex" to build main.hex, then "make flash" to upload the firmware
to the device. Don't forget to run "make fuse" once to program the fuses. If
you use a prototyping board with boot loader, follow the instructions of the
boot loader instead.

Please note that the first "make hex" copies the driver from the top level
into the firmware directory. If you use a different build system than our
Makefile, you must copy the driver by hand.


BUILDING THE HOST SOFTWARE
==========================
Since the host software is based on libusb or libusb-win32, make sure that
this library is installed. On Unix, ensure that libusb-config is in your
search PATH. On Windows, edit Makefile.windows and set the library path
appropriately. Then type "make" on Unix or "make -f Makefile.windows" on
Windows to build the command line tool.


USING THE COMMAND LINE TOOL
===========================
The command line tool has three valid arguments: "status" to query the
current LED status, "on" to turn on the LED and "off" to turn it off.


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
