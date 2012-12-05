#!/bin/sh
# Author: Christian Starkjohann
# Creation Date: 2008-04-17
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)


if [ "$1" = remove ]; then
    make clean
    rm -f opendevice.[ch]
    exit
fi

cp ../../libs-host/opendevice.[ch] .
