What is V-USB?
==============
V-USB is a firmware-only USB driver for Atmel's AVR microcontrollers.
For more information please visit <http://www.obdev.at/vusb/>.

What is in this Repository?
===========================
This repository contains the source code of the driver (in the usbdrv
subdirectory), examples (in the examples) subdirectory and other things
you might need when you design a device using V-USB.

When you check out this repository, the resulting directory is *not* equivalent
to the source code package which can be downloaded at
<http://www.obdev.at/vusb/>. Some files in the source code package are generated
by scripts when the package is created. On the other hand, the scripts which
generate source files and the package are not contained in the package itself.

If you want to know more about the files and directories, see the file
Readme.txt in the top level directory.

How do I Add the Driver to My Project?
======================================
Simply copy the entire usbdrv subdirectory into your project's firmware
source code directory. Then edit the firmware's Makefile and add the following
object files to your binary:

    usbdrv/usbdrv.o
    usbdrv/usbdrvasm.o
    usbdrv/oddebug.o

Then make sure that your Makefile contains rules to convert *.S and *.c to
object files. See the Makefiles in the examples subdirectory for an
inspiration.
