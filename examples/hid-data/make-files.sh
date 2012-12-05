#!/bin/sh
# Author: Christian Starkjohann
# Creation Date: 2008-04-17
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)


if [ "$1" = remove ]; then
    (cd firmware; make clean)
    rm -f firmware/usbconfig.h
    rm -rf firmware/usbdrv
    rm -f firmware/Makefile
    rm -f commandline/hiddata.[ch]
    rm -f commandline/hidsdi.h
    exit
fi

cat << \EOF | sed -n -f /dev/stdin ../../usbdrv/usbconfig-prototype.h >firmware/usbconfig.h
/^\( [*] \)\{0,1\}[+].*$/ d
s/^#define USB_CFG_DMINUS_BIT .*$/#define USB_CFG_DMINUS_BIT      4/g
s|^.*#define USB_CFG_CLOCK_KHZ.*$|#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)|g
s/^#define USB_CFG_HAVE_INTRIN_ENDPOINT .*$/#define USB_CFG_HAVE_INTRIN_ENDPOINT    1/g
s|^#define  USB_CFG_DEVICE_ID .*$|#define  USB_CFG_DEVICE_ID       0xdf, 0x05 /* obdev's shared PID for HIDs */|g
s/^#define USB_CFG_DEVICE_NAME .*$/#define USB_CFG_DEVICE_NAME     'D', 'a', 't', 'a', 'S', 't', 'o', 'r', 'e'/g
s/^#define USB_CFG_DEVICE_NAME_LEN .*$/#define USB_CFG_DEVICE_NAME_LEN 9/g

s/^#define USB_CFG_INTR_POLL_INTERVAL .*$/#define USB_CFG_INTR_POLL_INTERVAL      100/g
s/^#define USB_CFG_MAX_BUS_POWER .*$/#define USB_CFG_MAX_BUS_POWER           20/g
s/^#define USB_CFG_IMPLEMENT_FN_WRITE .*$/#define USB_CFG_IMPLEMENT_FN_WRITE      1/g
s/^#define USB_CFG_IMPLEMENT_FN_READ .*$/#define USB_CFG_IMPLEMENT_FN_READ       1/g
s/^#define USB_CFG_DEVICE_CLASS .*$/#define USB_CFG_DEVICE_CLASS        0/g
s/^#define USB_CFG_INTERFACE_CLASS .*$/#define USB_CFG_INTERFACE_CLASS     3/g
s/^.*#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH.*$/#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    22/g
p
EOF

cat << \EOF | sed -n -f /dev/stdin ../custom-class/firmware/Makefile >firmware/Makefile
/^\( [*] \)\{0,1\}[+].*$/ d
s/^# Project: .*$/# Project: hid-data example/g
p
EOF

cp ../../libs-host/hiddata.[ch] ../../libs-host/hidsdi.h commandline
