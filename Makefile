# Name: Makefile
# Project: v-usb
# Author: Christian Starkjohann
# Creation Date: 2012-12-05
# Tabsize: 4
# Copyright: (c) 2012 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

# This is the main Makefile. The two primary targets are "all", to build
# everything which can be built (except tests), and "clean" to remove all
# dependent files. In a repository clone, derived source files are generated
# and deleted as well.
#
# We distinguish between repository clones and source packages by the existence
# of make-files.sh scripts in various subdirectories.


all:
	if [ ! -f examples/hid-mouse/firmware/Makefile ]; then \
		$(MAKE) files; \
	fi
	if [ -n "$(uname -s | grep -i mingw)" ]; then \
		$(MAKE) windows; \
	else \
		$(MAKE) unix; \
	fi

clean:
	$(MAKE) unixclean
	if cross-make.sh --help >/dev/null 2>&1; then \
		$(MAKE) windowsclean; \
	fi
	$(MAKE) filesremove


unix unixclean:
	target=$$(echo $@ | sed -e 's/unix//g'); \
	find . -mindepth 3 -name Makefile -print | while read i; do \
		dir=$$(dirname $$i); \
		dirname=$$(basename $$dir); \
		pushd $$dir >/dev/null; \
		if [ "$$dirname" = firmware -a -z "$$target" ]; then \
			if ! $(MAKE) hex; then break; fi; \
		else \
			if ! $(MAKE) $$target; then break; fi;\
		fi; \
		popd >/dev/null; \
	done


windows windowsclean:
	target=$$(echo $@ | sed -e 's/windows//g'); \
	find . -mindepth 3 -name Makefile.windows -execdir cross-make.sh $$target \; ; \
	if [ -z "$$target" ]; then target=hex; fi; \
	find . -mindepth 2 -name firmware -exec sh -c "cd '{}'; $(MAKE) $$target" \;

files filesremove:
	target=$$(echo $@ | sed -e 's/files//g'); \
	find . -mindepth 2 -name make-files.sh -execdir ./make-files.sh $$target \;

