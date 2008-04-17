This is the Readme file for the hid-custom-rq example. This is basically the
same as the custom-class example, except that the device conforms to the USB
HID class.


WHAT IS DEMONSTRATED?
=====================
This example demonstrates how custom requests can be sent to devices which
are otherwise HID compliant. This mechanism can be used to prevent the
"driver CD" dialog on Windows and still control the device with libusb-win32.
It can also be used to extend the functionality of the USB class, e.g. by
setting parameters.

Please note that you should install the filter version of libusb-win32 to
take full advantage or this mode. The device driver version only has access
to devices which have been registered for it with a *.inf file. The filter
version has access to all devices.


MORE INFORMATION
================
For information about how to build this example and how to use the command
line tool see the Readme file in the custom-class example.


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
