This is the Readme file for V-USB and related code. V-USB is Objective
Development's firmware-only USB driver for Atmel's(r) AVR(r) microcontrollers.
For more information please visit http://www.obdev.at/vusb/.

To avoid name confusion: This project was formerly known as AVR-USB. Due to
a trademark issue, it was renamed to V-USB in April 2009.


WHAT IS INCLUDED IN THIS PACKAGE?
=================================
This package consists of the device side USB driver firmware, library code
for device and host and fully working examples for device and host:

  Readme.txt .............. The file you are currently reading.
  usbdrv .................. V-USB firmware, to be included in your project.
  examples ................ Example code for device and host side.
  libs-device ............. Useful code snippets for the device firmware.
  libs-host ............... Useful code snippets for host-side drivers.
  circuits ................ Example circuits using this driver.
  Changelog.txt ........... Documentation of changes between versions.
  License.txt ............. Free Open Source license for this package (GPL).
  CommercialLicense.txt ... Alternative commercial license for this package.
  USB-ID-FAQ.txt .......... General infos about USB Product- and Vendor-IDs.
  USB-IDs-for-free.txt .... List and terms of use for free shared PIDs.

Each subdirectory contains a separate Readme file which explains its
contents. We recommend that you also read the Readme.txt file in the
usbdrv subdirectory.


PREREQUISITES
=============
The AVR code of V-USB is written in C and assembler. You need either
avr-gcc or IAR CC to compile the project. We recommend avr-gcc because it
is free and easily available. Gcc version 3 generates slightly more
efficient code than version 4 for V-USB. Not every release is tested with
the IAR compiler. Previous versions have been tested with IAR 4.10B/W32 and
4.12A/W32 on an ATmega8 with the "small" and "tiny" memory model.

Ready made avr-gcc tool chains are available for most operating systems:
  * Windows: WinAVR http://winavr.sourceforge.net/
  * Mac: CrossPack for AVR Development http://www.obdev.at/crosspack/
  * Linux and other Unixes: Most free Unixes have optional packages for AVR
    development. If not, follow the instructions at
    http://www.nongnu.org/avr-libc/user-manual/install_tools.html

Our host side examples are compiled with gcc on all platforms. Gcc is the
default C compiler on Mac, Linux and many other Unixes. On windows, we
recommend MinGW (http://www.mingw.org/). Use the automated MinGW installer
for least troubles. You also need MSYS from the same site to work with
standard Makefiles.

Most examples also depend on libusb. Libusb is available from
http://libusb.sourceforge.net/ for Unix and
http://libusb-win32.sourceforge.net/ for Windows.


TECHNICAL DOCUMENTATION
=======================
The API reference of the driver firmware can be found in usbdrv/usbdrv.h.
Documentation for host and device library files are in the respective header
files. For more information, see our documentation wiki at
http://www.obdev.at/goto.php?t=vusb-wiki.

See the file usbdrv/Readme.txt for more info about the driver itself.


LICENSE
=======
V-USB and related code is distributed under the terms of the GNU General
Public License (GPL) version 2 (see License.txt for details) and the GNU
General Public License (GPL) version 3. It is your choice whether you apply
the terms of version 2 or version 3. In addition to the terms of the GPL, we
strongly encourage you to publish your entire project and mail OBJECTIVE
DEVELOPMENT a link to your publication.

Alternatively, we offer a commercial license without the restrictions of the
GPL. See CommercialLicense.txt for details.


----------------------------------------------------------------------------
(c) 2010 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
