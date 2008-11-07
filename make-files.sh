#!/bin/sh
# Author: Christian Starkjohann
# Creation Date: 2008-04-17
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
# This Revision: $Id$


find . -mindepth 2 -name 'make-files.sh' -exec sh -c "cd \`dirname {}\`; ./make-files.sh \"$1\"" \;
