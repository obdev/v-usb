#!/bin/sh
# Author: Christian Starkjohann
# Creation Date: 2008-04-17
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
# This Revision: $Id$

if [ "$2" = windows ]; then
    find . -mindepth 2 -name Makefile.windows -exec sh -c "cd \`dirname {}\`; cross-make.sh $1" \;
else
    find . -mindepth 2 -name Makefile -print | while read i; do
        dir=`dirname $i`
        dirname=`basename $dir`
        (
            cd $dir
            if [ "$dirname" = "firmware" -a -z "$1" ]; then
                make hex
            else
                make $1
            fi
        )
    done
fi
