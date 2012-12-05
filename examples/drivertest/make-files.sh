#!/bin/sh
# Author: Christian Starkjohann
# Creation Date: 2008-04-17
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)


if [ "$1" = remove ]; then
    (cd firmware; make clean)
    rm -rf firmware/usbdrv
    rm -f firmware/osccal.[ch]
    rm -f commandline/Makefile.windows
    rm -f commandline/Makefile
    rm -f commandline/opendevice.[ch]
    exit
fi

cat << \EOF | sed -n -f /dev/stdin ../usbtool/Makefile.windows >commandline/Makefile.windows
/^\( [*] \)\{0,1\}[+].*$/ d
s/^# Project: .*$/# Project: hid-custom-rq example/g
p
EOF

cat << \EOF | sed -n -f /dev/stdin ../usbtool/Makefile >commandline/Makefile
/^\( [*] \)\{0,1\}[+].*$/ d
s/^# Project: .*$/# Project: hid-custom-rq example/g
s/^NAME = .*$/NAME = runtest/g
p
EOF

cp ../../libs-host/opendevice.[ch] commandline/
cp ../../libs-device/osccal.[ch] firmware/
