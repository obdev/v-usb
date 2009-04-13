This is the Readme file for the directory "examples" of V-USB, a firmware-
only USB driver for AVR microcontrollers.

WHAT IS IN THIS DIRECTORY?
==========================
This directory contains examples which are mostly for educational purposes.
Examples can be device firmware only, host software only or both. Here is
a summary:

custom-class
  A custom class device with host software based on libusb. It demonstrates
  the straight forward way of sending small amounts of data to a device and
  receiving data from the device. It does NOT demonstrate how to send large
  amounts of data to the device or how to receive data generated on the fly
  by the device (how to use usbFunctionWrite() and usbFunctionRead()). See
  the hid-data example for how usbFunctionWrite() and usbFunctionRead() are
  used.

hid-custom-rq
  This example implements the same functionality as the custom-class example
  above, but declares the device as HID. This prevents the "give me a driver
  CD" dialog on Windows. The device can still be controlled with libusb as in
  the previous example (on Windows, the filter version of libusb-win32 must
  be installed). In addition to the features presented in custom-class, this
  example demonstrates how a HID class device is defined.

hid-mouse
  This example implements a mouse device. No host driver is required since
  today's operating systems have drivers for USB mice built-in. It
  demonstrates how a real-world HID class device is implemented and how
  interrupt-in endpoints are used.

hid-data
  This example demonstrates how the HID class can be misused to transfer
  arbitrary data over HID feature reports. This technique is of great value
  on Windows because no driver DLLs are needed (the hid-custom-rq example
  still requires the libusb-win32 DLL, although it may be in the program's
  directory). The host side application requires no installation, it can
  even be started directly from a CD. This example also demonstrates how
  to transfer data using usbFunctionWrite() and usbFunctionRead().

usbtool
  This is a general purpose development and debugging tool for USB devices.
  You can use it during development of your device to test various requests
  without special test programs. But it is also an example how all the
  libusb API functions are used.

More information about each example can be found in the Readme file in the
respective directory.

Hardware dependencies of AVR code has been kept at a minimum. All examples
should work on any AVR chip which has enough resources to run the driver.
Makefile and usbconfig.h have been configured for the metaboard hardware (see
http://www.obdev.at/goto.php?t=metaboard for details). Edit the target
device, fuse values, clock rate and programmer in Makefile and the I/O pins
dedicated to USB in usbconfig.h.


WHAT IS NOT DEMONSTRATED IN THESE EXAMPLES?
===========================================
These examples show only the most basic functionality. More elaborate
examples and real world applications showing more features of the driver are
available at http://www.obdev.at/vusb/projects.html. Most of these
features are described in our documentation wiki at
http://www.obdev.at/goto.php?t=vusb-wiki.

To mention just a few:

Using RC oscillator for system clock
  The 12.8 MHz and 16.5 MHz modules of V-USB have been designed to cope
  with clock rate deviations up to 1%. This allows an RC oscillator to be
  used. Since the AVR's RC oscillator has a factory precision of only 10%,
  it must be calibrated to an external reference. The EasyLogger example
  shows how this can be done.

Dynamically generated descriptors
  Sometimes you want to implement different typtes of USB device depending
  on a jumper or other condition. V-USB has a very flexible interface for
  providing USB descriptors. See AVR-Doper for how to provide descriptors
  at runtime.

Virtual COM port
  Some people prefer a virtual serial interface to communicate with their
  device. We strongly discourage this method because it does things
  forbidden by the USB specification. If you still want to go this route,
  see AVR-CDC.

Implementing suspend mode
  V-USB does not implement suspend mode. This means that the device does
  not reduce power consumption when the host goes into sleep mode. Device
  firmware is free to implement suspend mode, though. See USB2LPT for an
  example.

The projects mentioned above can best be found on

    http://www.obdev.at/vusb/prjall.html

where all projects are listed.

----------------------------------------------------------------------------
(c) 2009 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
