#!/bin/sh
# Name: mkdist.sh
# Project: avr-usb
# Author: Christian Starkjohann
# Creation Date: 2008-04-18
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
# This Revision: $Id$

# This script was created for Mac OS X with EAGLE and ImageMagick installed
# The "open" command is specific to Mac OS X and is used to start GUI
# applications or open files and directories.

name=avrusb

PATH="$PATH:/usr/local/EAGLE/bin"
eagle=eagle

#-------------------------------------------------------------------
# initial user dialog:
#-------------------------------------------------------------------

if [ "$1" = public ]; then
	echo "Generating a public (tagged) release"
	isPublic=yes
    today=`date +%Y%m%d`
    releasedate=`grep '^[*] Release ' usbdrv/Changelog.txt | awk '{date=$NF} END{gsub("-", "", date); print date}'`
    if [ "$releasedate" != "$today" ]; then
        echo "Release is not documented in usbdrv/Changelog.txt, please do that!"
        exit 1
    fi
    cat << EOF | sed -n -f /dev/stdin usbdrv/usbdrv.h >usbdrv/usbdrv.h.new
/^\( [*] \)\{0,1\}[+].*\$/ d
s/^#define USBDRV_VERSION .*\$/#define USBDRV_VERSION  $today/g
p
EOF
    rm usbdrv/usbdrv.h
    mv usbdrv/usbdrv.h.new usbdrv/usbdrv.h
else
	echo "For a public release (tagged in subversion) add parameter \"public\""
	isPublic=no
fi

#-------------------------------------------------------------------
# determine version, commit and tag in SVN
#-------------------------------------------------------------------
open -a X11	# we need X11 for EAGLE

version=`grep USBDRV_VERSION usbdrv/usbdrv.h | awk '{print $NF}'`
if [ "$isPublic" != yes ]; then
	version="$version"-priv
fi

if [ "$isPublic" = yes ]; then
(
    currentGcc=`avr-gcc-select | awk '{print $NF}'`
    cd tests
    for i in 3 4; do
        avr-gcc-select $i >/dev/null 2>&1
        gccvers=`avr-gcc --version | awk '{print $NF; exit}'`
        file=sizes-$version-gcc$gccvers.txt
        make sizes
        mv sizes.txt sizes-reference/$file
        svn add sizes-reference/$file
        svn commit -m "Added sizes file for this version" sizes-reference/$file
    done
    avr-gcc-select $currentGcc
)
fi

if svn commit; then
	:
else
	echo "svn commit failed, aborting"
	exit 1
fi

repository=`svn info | sed -n -e '/^URL:/ s|^URL: \(.*\)/trunk|\1| p'`
if [ "$isPublic" = yes ]; then
	echo "Tagging $repository as $version"
	svn copy "$repository/trunk" "$repository/tags/$version" -m "tagging as $version"
fi

#-------------------------------------------------------------------
# SVN checkout
#-------------------------------------------------------------------

echo "Creating distribution for $name version $version"
pkgname="$name-$version"

if svn checkout "$repository/trunk" "/tmp/$pkgname"; then
	:
else
	echo "svn checkout failed, aborting"
	exit 1
fi
cd "/tmp/$pkgname"

#-------------------------------------------------------------------
# Automatically create PNG files from EAGLE design
#-------------------------------------------------------------------

# Script for exporting circuit diagram:
tname="mkdist-$$"
cat >/tmp/$tname.scr <<EOF
EXPORT IMAGE 'circuits/image.png' monochrome 300;
QUIT
EOF

# Copy the schematics file and run the script on it:
for i in circuits/*.sch; do
    rm -f "circuits/image.png"
    cp "$i" /tmp/$tname.sch
    $eagle -S/tmp/$tname.scr /tmp/$tname.sch
    file=`basename -s .sch $i`
    mv circuits/image.png circuits/$file.png
done
rm /tmp/$tname.scr /tmp/$tname.sch

#-------------------------------------------------------------------
# Generate all derived files
#-------------------------------------------------------------------

find . -mindepth 2 -name 'make-files.sh' -execdir ./make-files.sh \;

#-------------------------------------------------------------------
# Remove unnecessary files from distribution and create archive
#-------------------------------------------------------------------

rm -rf examples/drivertest
find . -name '.svn' -prune -exec rm -rf '{}' \; # remove SVN files
find . -name 'make-files.sh' -exec rm '{}' \;   # remove helper scripts
rm -f mkdist.sh make-files.sh
(
    cd usbdrv
    cp Changelog.txt License.txt CommercialLicense.txt USBID-License.txt ..
)
cd ..
echo "Creating /tmp/$pkgname.zip and /tmp/$pkgname.tar.gz"
zip -rq9 "$pkgname.zip" "$pkgname"
tar cfz "$pkgname.tar.gz" "$pkgname"
open /tmp
