This is the Readme file for usbtool, a general purpose command line utility
which can send USB requests to arbitrary devices. Usbtool is based on libusb.


WHAT IS USBTOOL GOOD FOR?
=========================
When you implement a communication protocol like USB, you must usually write
two programs: one on each end of the communication. For USB, this means that
you must write a firmware for the device and driver software for the host.

Usbtool can save you the work of writing the host software, at least during
firmware development and testing. Usbtool can send control-in and -out
requests to arbitrary devices and send and receive data on interrupt- and
bulk-endpoints.

Usbtool is not only a useful developer tool, it's also an example for using
libusb for communication with the device.


SYNOPSIS
========
  usbtool [options] <command>


COMMANDS
========
  list
    This command prints a list of devices found on all available USB busses.
    Options -v, -V, -p and -P can be used to filter the list.

  control in|out <type> <recipient> <request> <value> <index>
    Sends a control-in or control-out request to the device. The request
    parameters are:
      type ........ Type of request, can be "standard", "class", "vendor" or
                    "reserved". The type determines which software module in
                    the device is responsible for answering the request:
                    Standard requests are answered by the driver, class
                    requests by the class implementation (e.g. HID, CDC) and
                    vendor requests by custom code.
      recipient ... Recipient of the request in the device. Can be "device",
                    "interface", "endpoint" or "other". For standard and
                    class requests, the specification defines a recipient for
                    each request. For vendor requests, choose whatever your
                    code expects.
      request ..... 8 bit numeric value identifying the request.
      value ....... 16 bit numeric value passed to the device.
      index ....... another 16 bit numeric value passed to the device.
    Use options -v, -V, -p and -P to single out a particular device. Use
    options -d or -D to to send data in an OUT request. Use options -n, -O
    and -b to determine what to do with data received in an IN request.

  interrupt in|out
    Sends or receives data on an interrupt-out resp. -in endpoint.
    Use options -v, -V, -p and -P to single out a particular device. Use
    options -d or -D to to send data to an OUT endpoint. Use options -n, -O
    and -b to determine what to do with data received from an IN endpoint.
    Use option -e to set the endpoint number, -c to choose a configuration
    -i to claim a particular interface.

  bulk in|out
    Same as "interrupt in" and "interrupt out", but for bulk endpoints.


OPTIONS
=======
Most options have already been mentioned at the commands which use them.
here is a complete list:

  -h or -?
    Prints a short help.

  -v <vendor-id>
    Numeric vendor ID, can be "*" to allow any VID. Take only devices with
    matching vendor ID into account.

  -p <product-id>
    Numeric product ID, can be "*" to allow any PID. Take only devices with
    matching product ID into account.

  -V <vendor-name-pattern>
    Shell style matching pattern for vendor name. Take only devices into
    account which have a vendor name that matches this pattern.

  -P <product-name-pattern>
    Shell style matching pattern for product name. Take only devices into
    account which have a product name that matches this pattern.

  -S <serial-pattern>
    Shell style matching pattern for serial number. Take only devices into
    account which have a serial number that matches this pattern.

  -d <databytes>
    Data bytes to send to the device, comma separated list of numeric values.
    E.g.: "1,2,3,4,5".

  -D <file>
    Binary data sent to the device should be taken from this file.

  -O <file>
    Write received data bytes to the given file. Format is either hex or
    binary, depending on the -b flag. By default, received data is printed
    to standard output.

  -b
    Request binary output format for files and standard output. Default is
    a hexadecimal listing.

  -n <count>
    Numeric value: Maximum number of bytes to receive. This value is passed
    directly to the libusb API functions.

  -e <endpoint>
    Numeric value: Endpoint number for interrupt and bulk commands.

  -t <timeout>
    Numeric value: Timeout in milliseconds for the request. This value is
    passed directly to the libusb API functions.

  -c <configuration>
    Numeric value: Interrupt and bulk endpoints can usually only be used if
    a configuration and an interface has been chosen. Use -c and -i to
    specify configuration and interface values.

  -i <interface>
    Numeric value: Interrupt and bulk endpoints can usually only be used if
    a configuration and an interface has been chosen. Use -c and -i to
    specify configuration and interface values.

  -w
    Usbtool may be too verbose with warnings for some applications. Use this
    option to suppress USB warnings.


NUMERIC VALUES
==============
All numeric values can be given in hexadecimal, decimal or octal. Hex values
are identified by their 0x or 0X prefix, octal values by a leading "0" (the
digit zero) and decimal values because they start with a non-zero digit. An
optional sign character is allowed. The special value "*" is translated to
zero and stands for "any value" in some contexts.


SHELL STYLE MATCHING PATTERNS
=============================
Some options take shell style matching patterns as an argument. This refers
to Unix shells and their file wildcard operations:
  + "*" (asterisk character) matches any number (0 to infinite) of any
    characters.
  + "?" matches exactly one arbitrary character.
  + A list of characters in square brackets (e.g. "[abc]") matches any of the
    characters in the list. If a dash ("-") is in the list, it must be the
    first or the last character. If a caret ("^") is in the list, it must
    not be the first character. A closing square bracket ("]") must be the
    first character in the list. A range of characters can be specified in
    the way "[a-z]". This matches all characters with numeric representation
    (usually ASCII) starting with "a" and ending with "z". The entire
    construct matches only one character.
  + A list of characters in square brackets starting with a caret ("^"), e.g.
    ("[^abc]") matches any character NOT in the list. The other rules are as
    above.
  + "\" (backslash) followed by any character matches that following
    character. This can be used to escape "*", "?", "[" and "\".


BUILDING USBTOOL
================
Usbtool uses libusb on Unix and libusb-win32 on Windows. These libraries can
be obtained from http://libusb.sourceforge.net/ and
http://libusb-win32.sourceforge.net/ respectively. On Unix, a simple "make"
should compile the sources (although you may have to edit Makefile to
include or remove additional libraries). On Windows, we recommend that you
use MinGW and MSYS. See the top level Readme file for details. Edit
Makefile.windows according to your library installation paths and build with
"make -f Makefile.windows".


EXAMPLES
========
To list all devices connected to your computer, do

    usbtool -w list

To check whether our selection options single out the desired device, use eg.

    usbtool -w -P LEDControl list

This command shows all LEDControl devices connected or prints nothing if
none is found. LEDControl is the device from the "custom-class" example.

You can also send commands to the LEDControl device using usbtool. From
the file requests.h in custom-class/firmware, we know that the set-status
request has numeric value 1 and the get-status request is 2. See this file
for details of the protocol used. We can therefore query the status with

    usbtool -w -P LEDControl control in vendor device 2 0 0

This command prints 0x00 if the LED is off or 0x01 if it is on. To turn the
LED on, use

    usbtool -w -P LEDControl control out vendor device 1 1 0

and to turn it off, use

    usbtool -w -P LEDControl control out vendor device 1 0 0


----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
