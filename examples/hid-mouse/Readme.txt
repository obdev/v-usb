This is the Readme file for hid-mouse, an example of a USB mouse device. In
order to have as little dependencies on hardware and architecture as
possible, mouse movements are computed internally so that the mouse pointer
moves in a circle.


WHAT IS DEMONSTRATED?
=====================
This example demonstrates how HID class devices are implemented. The example
is kept as simple as possible, except the report descriptor which is taken
from a real-world mouse.

It does NOT include a host side driver because all modern operating systems
include one. It does NOT implement USBRQ_HID_SET_REPORT and report-IDs. See
the "hid-data" example for this topic. It does NOT implement any special
features such as suspend mode etc.


PREREQUISITES
=============
Target hardware: You need an AVR based circuit based on one of the examples
(see the "circuits" directory at the top level of this package), e.g. the
metaboard (http://www.obdev.at/goto.php?t=metaboard).

AVR development environment: You need the gcc tool chain for the AVR, see
the Prerequisites section in the top level Readme file for how to obtain it.


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


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
