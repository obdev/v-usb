This is the Readme file for the libs-host directory. This directory contains
code snippets which may be useful for host side USB software.


WHAT IS INCLUDED IN THIS DIRECTORY?
===================================

opendevice.c and opendevice.h
  This module contains a function to find and open a device given its
  numeric IDs (VID, PID), names (vendor name and product name) and serial
  number. It is based on libusb/libusb-win32 and returns a libusb device
  handle. See opendevice.h for an API documentation.

hiddata.c and hiddata.h
  This module contains functions for data transfer over HID feature reports.
  It is based on libusb on Unix and native Windows functions on Windows. No
  driver DLL is needed on Windows. See hiddata.h for an API documentation.

hidsdi.h
  This DDK header file is missing in the free MinGW version of the Windows
  DDK. Use this version if you get an "include file not found" error.


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
