# What is V-USB?

V-USB is a firmware-only USB driver for Atmel's AVR microcontrollers.
For more information please visit <http://www.obdev.at/vusb/>.

# What is in this Repository?

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

# How do I Add the Driver to My Project?

## In a Makefile

Simply copy the entire usbdrv subdirectory into your project's firmware
source code directory. Then edit the firmware's Makefile and add the following
object files to your binary:

    usbdrv/usbdrv.o
    usbdrv/usbdrvasm.o
    usbdrv/oddebug.o

Then make sure that your Makefile contains rules to convert *.S and *.c to
object files. See the Makefiles in the examples subdirectory for an
inspiration.

## With PlatformIO

The easiest way to use V-USB in PlatformIO is to just copy the `usbdrv` folder
to the `src` directory of the project:
```
project/
├── platformio.ini
└── src/
    ├── main.c
    ├── usbconfig.h
    └── usbdrv/
        ├── usbdrv.c
        ├── usbdrv.h
        ├── usbdrvasm.S
        ├── usbdrvasm*.inc
        ├── oddebug.c
        └── oddebug.h
```
The `usbconfig.h` is part of your project. Derive it from the template as
usual.

In `platformio.ini` you must make sure that `.S` files are preprocessed by
the "C" preprocessor and then passed to the assembler. `.inc` and `.asm`files
should *not* be touched, they are included as needed.

Here is a minimal example `platformio.ini` for a bare metal ATTiny85:
```
[env:attiny85]
platform = atmelavr
board = attiny85

board_build.f_cpu = 16500000L      ; also reaches usbdrvasm.S as -DF_CPU=...

build_flags =
    -Isrc                          ; explicit, so usbdrv/*.c find usbconfig.h
    -Wall

; keep the IAR-syntax assembly file out of the build
build_src_filter =
    +<*>
    -<usbdrv/usbdrvasm.asm>
```
