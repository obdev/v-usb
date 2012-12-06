#!/bin/sh
# Name: mkdist.sh
# Project: v-usb
# Author: Christian Starkjohann
# Creation Date: 2008-04-18
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

# This script was created for Mac OS X with EAGLE and ImageMagick installed
# The "open" command is specific to Mac OS X and is used to start GUI
# applications or open files and directories.

name=vusb

#PATH="$PATH"
eagle=~/Applications/EAGLE/EAGLE.app/Contents/MacOS/EAGLE

#-------------------------------------------------------------------
# initial user dialog:
#-------------------------------------------------------------------

changes=$(git status --porcelain --untracked-files=no)
if [ -n "$changes" ]; then
    echo "There are uncommitted changes. Please commit them before making a release!"
    exit 1
fi

branch="$(git symbolic-ref HEAD)"
branch="${branch##refs/heads/}"
if [ "$branch" != master ]; then
    echo "Warning: On branch $branch, not master! Type enter to continue anyway."
    read dummy
fi

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
    if cmp --silent usbdrv/usbdrv.h usbdrv/usbdrv.h.new; then
        rm usbdrv/usbdrv.h.new  #files are equal
    else
        rm usbdrv/usbdrv.h
        mv usbdrv/usbdrv.h.new usbdrv/usbdrv.h
        git add usbdrv/usbdrv.h
        git commit -m "RELEASE: Updated version number to $today"
    fi
else
	echo "For a public release (tagged in subversion) add parameter \"public\""
	isPublic=no
fi

#-------------------------------------------------------------------
# determine version and tag in GIT
#-------------------------------------------------------------------

version=`grep USBDRV_VERSION usbdrv/usbdrv.h | awk '{print $NF}'`
if [ "$isPublic" != yes ]; then
	version="$version"-priv
else
    (
        currentGcc=`avr-gcc-select | awk '{print $NF}'`
        cd tests
        for i in 3 4; do
            avr-gcc-select $i >/dev/null 2>&1
            gccvers=`avr-gcc --version | awk '{print $NF; exit}'`
            file=sizes-$version-gcc$gccvers.txt
            make sizes
            mv sizes.txt sizes-reference/$file
            git add sizes-reference/$file
        done
        git commit -m "RELEASE: Added sizes files for this version"
        avr-gcc-select $currentGcc
    )
	echo "Tagging as version $version"
    git tag "releases/$version"
fi

#-------------------------------------------------------------------
# checkout source from repository
#-------------------------------------------------------------------

echo "Creating distribution for $name version $version"
pkgname="$name-$version"

rm -rf "/tmp/$pkgname"
rm -f "/tmp/$pkgname".*
mkdir "/tmp/$pkgname"
git archive --format tar "$branch" | tar -x -C "/tmp/$pkgname"
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

make files

#-------------------------------------------------------------------
# Remove unnecessary files from distribution and create archive
#-------------------------------------------------------------------

rm -rf examples/drivertest v-usb.xcodeproj
find . -name 'make-files.sh' -exec rm '{}' \;   # remove helper scripts
rm -f mkdist.sh README.md .gitignore
(
    cd usbdrv
    cp Changelog.txt License.txt CommercialLicense.txt USB-IDs-for-free.txt USB-ID-FAQ.txt ..
)
cd ..
echo "Creating /tmp/$pkgname.zip and /tmp/$pkgname.tar.gz"
zip -rq9 "$pkgname.zip" "$pkgname"
tar cfz "$pkgname.tar.gz" "$pkgname"
open /tmp

echo
echo "***********************************************************************"
echo "Don't forget to push GIT repo (including tags!) to origin!"
echo "***********************************************************************"
