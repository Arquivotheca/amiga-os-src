#
#  FILE
#
#	bru.mk    main makefile for bru utility
#
#  SCCS
#
#	@(#)bru.mk	5.7	3/20/89
#
#  SYNOPSIS
#
#	cd bru
#	make -f bru.mk
#
#  DESCRIPTION
#
#	Main makefile for the bru utility.  Uses make recursively on
#	various subdirectories to make the executable utility, documentation,
#	tests, etc.
#
#	This file normally lives in the root node of the bru directory
#	hierarchy, which is typically someplace like /usr/src/cmd/bru.
#
#	The makefile in ./src/unix expects an explicit target.  The default
#	set here is to use the autoconfiguration script, make.sh, which
#	then invokes the makefile with the appropriate defines for make.
#

MAKE =	make
SHELL = /bin/sh

#
#	The default things to make.
#

all :		executable documentation


executable :
		cd src/unix; $(SHELL) make.sh

documentation :
		cd doc; $(MAKE)

#
#	Other things to make.
#

install :
		cd src/unix; $(SHELL) make.sh install

clean :
		cd src/unix; $(SHELL) make.sh clean
		cd doc; $(MAKE) clean
		rm -f *.BAK *.old nohup.out
