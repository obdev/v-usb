This is the Readme file to Objective Development's firmware-only USB driver
for Atmel AVR microcontrollers. For more information please visit
http://www.obdev.at/vusb/

This directory contains the USB firmware only. Copy it as-is to your own
project and add all .c and .S files to your project (these files are marked
with an asterisk in the list below). Then copy usbconfig-prototype.h as
usbconfig.h to your project and edit it according to your configuration.


TECHNICAL DOCUMENTATION
=======================
The technical documentation (API) for the firmware driver is contained in the
file "usbdrv.h". Please read all of it carefully! Configuration options are
documented in "usbconfig-prototype.h".

The driver consists of the following files:
  Readme.txt ............. The file you are currently reading.
  Changelog.txt .......... Release notes for all versions of the driver.
  usbdrv.h ............... Driver interface definitions and technical docs.
* usbdrv.c ............... High level language part of the driver. Link this
                           module to your code!
* usbdrvasm.S ............ Assembler part of the driver. This module is mostly
                           a stub and includes one of the usbdrvasm*.S files
                           depending on processor clock. Link this module to
                           your code!
  usbdrvasm*.inc ......... Assembler routines for particular clock frequencies.
                           Included by usbdrvasm.S, don't link it directly!
  asmcommon.inc .......... Common assembler routines. Included by
                           usbdrvasm*.inc, don't link it directly!
  usbconfig-prototype.h .. Prototype for your own usbdrv.h file.
* oddebug.c .............. Debug functions. Only used when DEBUG_LEVEL is
                           defined to a value greater than 0. Link this module
                           to your code!
  oddebug.h .............. Interface definitions of the debug module.
  usbportability.h ....... Header with compiler-dependent stuff.
  usbdrvasm.asm .......... Compatibility stub for IAR-C-compiler. Use this
                           module instead of usbdrvasm.S when you assembler
                           with IAR's tools.
  License.txt ............ Open Source license for this driver.
  USB-ID-FAQ.txt ......... General infos about USB Product- and Vendor-IDs.
  USB-IDs-for-free.txt ... List of shared VID/PID pairs and sharing rules.

(*) ... These files should be linked to your project.


CPU CORE CLOCK FREQUENCY
========================
We supply assembler modules for clock frequencies of 12 MHz, 12.8 MHz, 15 MHz,
16 MHz, 16.5 MHz 18 MHz and 20 MHz. Other clock rates are not supported. The
actual clock rate must be configured in usbconfig.h.

12 MHz Clock
This is the traditional clock rate of V-USB because it's the lowest clock
rate where the timing constraints of the USB spec can be met.

15 MHz Clock
Similar to 12 MHz, but some NOPs inserted. On the other hand, the higher clock
rate allows for some loops which make the resulting code size somewhat smaller
than the 12 MHz version.

16 MHz Clock
This clock rate has been added for users of the Arduino board and other
ready-made boards which come with a fixed 16 MHz crystal. It's also an option
if you need the slightly higher clock rate for performance reasons. Since
16 MHz is not divisible by the USB low speed bit clock of 1.5 MHz, the code
is somewhat tricky and has to insert a leap cycle every third byte.

12.8 MHz and 16.5 MHz Clock
The assembler modules for these clock rates differ from the other modules
because they have been built for an RC oscillator with only 1% precision. The
receiver code inserts leap cycles to compensate for clock deviations. 1% is
also the precision which can be achieved by calibrating the internal RC
oscillator of the AVR. Please note that only AVRs with internal 64 MHz PLL
oscillator can reach 16.5 MHz with the RC oscillator. This includes the very
popular ATTiny25, ATTiny45, ATTiny85 series as well as the ATTiny26. Almost
all AVRs can reach 12.8 MHz, although this is outside the specified range.

See the EasyLogger example at http://www.obdev.at/vusb/easylogger.html for
code which calibrates the RC oscillator based on the USB frame clock.

18 MHz Clock
This module is closer to the USB specification because it performs an on the
fly CRC check for incoming packets. Packets with invalid checksum are
discarded as required by the spec. If you also implement checks for data
PID toggling on application level (see option USB_CFG_CHECK_DATA_TOGGLING
in usbconfig.h for more info), this ensures data integrity. Due to the CRC
tables and alignment requirements, this code is bigger than modules for other
clock rates. To activate this module, you must define USB_CFG_CHECK_CRC to 1
and USB_CFG_CLOCK_KHZ to 18000 in usbconfig.h.

20 MHz Clock
This module is for people who won't do it with less than the maximum. Since
20 MHz is not divisible by the USB low speed bit clock of 1.5 MHz, the code
uses similar tricks as the 16 MHz module to insert leap cycles.


USB IDENTIFIERS
===============
Every USB device needs a vendor- and a product-identifier (VID and PID). The
USB Implementers Forum (USB-IF, www.usb.org) is the authority which assigns
Vendor IDs. The fees for a Vendor ID are out of reach for most hobbyists.

V-USB is designed for hobby use. For hobby projects, one option is a random
PID from 0x4400 to 0x7fff under VID 0x20a0. As of November 2026, the public
USB ID database used by Linux (http://www.linux-usb.org/usb.ids) lists no
products in this range, so a conflict with a known device is unlikely, but
not impossible. This is technical information on how to avoid conflicts, not
legal advice. See USB-ID-FAQ.txt for details.

Alternatively, the examples in this package use a few VID/PID pairs which are
shared by many V-USB based devices. These pairs are NOT assigned to you and
Objective Development is not authorized by the USB-IF to assign USB IDs.

Objective Development disclaims all liability for problems caused by your
choice of USB IDs. See the files USB-ID-FAQ.txt and USB-IDs-for-free.txt for
details.


DEVELOPMENT SYSTEM
==================
This driver has been developed and optimized for the GNU compiler version 3
and 4. We recommend that you use the GNU compiler suite because it is freely
available. V-USB has also been ported to the IAR compiler and assembler. It
has been tested with IAR 4.10B/W32 and 4.12A/W32 on an ATmega8 with the
"small" and "tiny" memory model. Not every release is tested with IAR CC and
the driver may therefore fail to compile with IAR. Please note that gcc is
more efficient for usbdrv.c because this module has been deliberately
optimized for gcc.

Gcc version 3 produces smaller code than version 4 due to new optimizing
capabilities which don't always improve things on 8 bit CPUs. The code size
generated by gcc 4 can be reduced with the compiler options
-fno-move-loop-invariants, -fno-tree-scev-cprop and
-fno-inline-small-functions in addition to -Os. On devices with more than
8k of flash memory, we also recommend the linker option --relax (written as
-Wl,--relax for gcc) to convert absolute calls into relative where possible.

For more information about optimizing options see:

    http://www.tty1.net/blog/2008-04-29-avr-gcc-optimisations_en.html

These optimizations are good for gcc 4.x. Version 3.x of gcc does not support
most of these options and produces good code anyway.


USING V-USB FOR FREE
====================
The AVR firmware driver is published under the GNU General Public License
Version 2 (GPL2) and the GNU General Public License Version 3 (GPL3). It is
your choice whether you apply the terms of version 2 or version 3.

If you decide for the free GPL2 or GPL3, we STRONGLY ENCOURAGE you to do the
following things IN ADDITION to the obligations from the GPL:

(1) Publish your entire project on a web site and drop us a note with the URL.
Use the form at http://www.obdev.at/vusb/feedback.html for your submission.
If you don't have a web site, you can publish the project in obdev's
documentation wiki at
http://www.obdev.at/goto.php?t=vusb-wiki&p=hosted-projects.

(2) Adhere to minimum publication standards. Please include AT LEAST:
    - a circuit diagram in PDF, PNG or GIF format
    - full source code for the host software
    - a Readme.txt file in ASCII format which describes the purpose of the
      project and what can be found in which directories and which files
    - a reference to http://www.obdev.at/vusb/

