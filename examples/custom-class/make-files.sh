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
    rm -f commandline/Makefile.windows
    rm -f commandline/Makefile
    rm -f commandline/opendevice.[ch]
    exit
fi

cat << \EOF | sed -n -f /dev/stdin ../../usbdrv/usbconfig-prototype.h >firmware/usbconfig.h
/^\( [*] \)\{0,1\}[+].*$/ d
s/^#define USB_CFG_DMINUS_BIT .*$/#define USB_CFG_DMINUS_BIT      4/g
s|^.*#define USB_CFG_CLOCK_KHZ.*$|#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)|g
s/^#define USB_CFG_DEVICE_NAME .*$/#define USB_CFG_DEVICE_NAME     'L', 'E', 'D', 'C', 'o', 'n', 't', 'r', 'o', 'l'/g
s/^#define USB_CFG_DEVICE_NAME_LEN .*$/#define USB_CFG_DEVICE_NAME_LEN 10/g

s/^#define USB_CFG_MAX_BUS_POWER .*$/#define USB_CFG_MAX_BUS_POWER           40/g
p
EOF

cat << \EOF | sed -n -f /dev/stdin ../usbtool/Makefile.windows >commandline/Makefile.windows
/^\( [*] \)\{0,1\}[+].*$/ d
s/^# Project: .*$/# Project: custom-class example/g
p
EOF

cat << \EOF | sed -n -f /dev/stdin ../usbtool/Makefile >commandline/Makefile
/^\( [*] \)\{0,1\}[+].*$/ d
s/^# Project: .*$/# Project: custom-class example/g
s/^NAME = .*$/NAME = set-led/g
p
EOF

cp ../../libs-host/opendevice.[ch] commandline/
