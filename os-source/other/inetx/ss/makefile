# ------------------------------------------------------------------
# Makefile for: s.library   (shared socket library)
#
# $Locker$
#
# $Id$
#
# $Revision$
#
# $Header$
#
#-------------------------------------------------------------------
#	MANX 5.0E   MANX 5.0E   MANX 5.0E   MANX 5.0E -ONLY-
#-------------------------------------------------------------------
# makefile
#-------------------------------------------------------------------

# - NOTE! C functions for a shared library that are compiled
# -       with Manx C 5.0x need the .begin statement held off.
# -       Use the -m0b flag.

# The networking include file directory
NETINC = NETINC:

# the server's BIN directory for this stuff
NETBIN = NETBIN:lib

# AOBJS are the functions that will be used in the library.
# These functions need to be reentrant! They also get compiled
# with the ".begin" statement removed!

# these functions are the glue code. Link with this (ss.lib) if not 
# using pragmas
AOBJS = f0.m5 f1.m5 f2.m5 f3.m5

# the library source code itself
LOBJS = sslib.m5

# functions in C for the library. Must be reentrant!
COBJS = func2.m5 testfunc.m5

.asm.m5:
	as $*.asm -o $*.m5

# the only C files that are being created here at this time
# are functions that go into the library. You need to be sure
# to omit the ".begin" symbol using the "-m0b" compiler flag

.c.m5:
	cc -so -m0b -me -iNETINC: -DAMIGA $*.c -o $*.m5


all: ss.library ss.lib

# the actual shared library
ss.library: $(LOBJS) $(COBJS) 
	ln +q -o ss.library $(LOBJS) $(COBJS) -lc
	
# the glue code  "ss.lib"
ss.lib: $(AOBJS)
	lb $@ $(AOBJS)
	
_func2.m5:
	cc -o_func2.m5 -m0b _func2.c

bump:
	bumprev 1 sslib_rev
